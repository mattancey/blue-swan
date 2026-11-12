#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>

class Saturation
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)blockSize, 1 };
        
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(numChannels, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
        oversampler->initProcessing((size_t)blockSize); // Correction warning
        
        hpf.prepare(spec); lpf.prepare(spec);
        hpf.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        lpf.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        hpf.setCutoffFrequency(80.0f);
        lpf.setCutoffFrequency(12000.0f);
        
        tuckFilters.resize((size_t)numChannels); // Correction warning
        for (auto& f : tuckFilters) {
            f.prepare(spec);
            f.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 150.0f, 1.0f, 1.2f);
        }
    }

    void reset() 
    { 
        if (oversampler) oversampler->reset(); 
        hpf.reset(); lpf.reset(); 
        for(auto& f : tuckFilters) f.reset();
    }

    void process(juce::AudioBuffer<float>& buffer, int driveType, float driveAmt, float omegaIntensity, bool useOversampling)
    {
        if (driveAmt <= 1.01f && omegaIntensity <= 0.01f) return;

        if (useOversampling && oversampler) {
            juce::dsp::AudioBlock<float> block(buffer);
            auto upBlock = oversampler->processSamplesUp(block);
            processBlock(upBlock, driveType, driveAmt, omegaIntensity);
            oversampler->processSamplesDown(block);
        } else {
            juce::dsp::AudioBlock<float> block(buffer);
            processBlock(block, driveType, driveAmt, omegaIntensity);
        }
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::dsp::StateVariableTPTFilter<float> hpf, lpf;
    std::vector<juce::dsp::IIR::Filter<float>> tuckFilters;

    void processBlock(juce::dsp::AudioBlock<float>& block, int driveType, float driveAmt, float omegaIntensity)
    {
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
            float* data = block.getChannelPointer(ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i) {
                float x = data[i];
                
                if (omegaIntensity > 0.01f) {
                    x = hpf.processSample((int)ch, x);
                    x = lpf.processSample((int)ch, x);
                }

                float y = x * driveAmt;
                if (driveType == 0) y = std::tanh(y);
                else if (driveType == 1) y = std::tanh(y + 0.2f) - 0.1f;
                else if (driveType == 2) { y = std::sin(y); y = juce::jlimit(-1.0f, 1.0f, y); }

                if (omegaIntensity > 0.01f) {
                    x = x * (1.0f - omegaIntensity * 0.5f) + y * (omegaIntensity * 0.5f);
                    x = tuckFilters[ch].processSample(x);
                } else {
                    x = y;
                }
                
                data[i] = x;
            }
        }
    }
};
