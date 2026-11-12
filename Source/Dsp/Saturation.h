#pragma once
#include <juce_dsp/juce_dsp.h>

class Saturation
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)blockSize, (juce::uint32)numChannels };
        
        // Oversampling 2x
        oversampler = std::make_unique<juce::dsp::Oversampling<float>>(numChannels, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
        oversampler->initProcessing(blockSize);
        
        // HPF et LPF pré-saturation (Omega 2N)
        hpf.prepare(spec); lpf.prepare(spec);
        hpf.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        lpf.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        hpf.setCutoffFrequency(80.0f);
        lpf.setCutoffFrequency(12000.0f);
        
        // Filtre Peaking (Tuck)
        *tuckFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 150.0f, 1.0f, 0.8f);
        tuckFilter.prepare(spec);
    }

    void reset() { if (oversampler) oversampler->reset(); hpf.reset(); lpf.reset(); tuckFilter.reset(); }

    void process(juce::AudioBuffer<float>& buffer, int driveType, float driveAmt, float omegaIntensity, bool useOversampling)
    {
        if (driveAmt <= 1.01f && omegaIntensity <= 0.01f) return; // Bypass

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
    juce::dsp::IIR::Filter<float> tuckFilter;

    void processBlock(juce::dsp::AudioBlock<float>& block, int driveType, float driveAmt, float omegaIntensity)
    {
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
            float* data = block.getChannelPointer(ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i) {
                float x = data[i];
                
                // Omega 2N Pre-Filter
                if (omegaIntensity > 0.01f) {
                    x = hpf.processSample((int)ch, x);
                    x = lpf.processSample((int)ch, x);
                }

                // Drive Principal
                float y = x * driveAmt;
                if (driveType == 0) y = std::tanh(y); // Soft
                else if (driveType == 1) y = std::tanh(y + 0.2f) - 0.1f; // Asym
                else if (driveType == 2) { y = std::sin(y); y = juce::jlimit(-1.0f, 1.0f, y); } // Fold

                // Omega 2N Post-Saturation & Tuck
                if (omegaIntensity > 0.01f) {
                    // Mélange dry/sat en interne basé sur l'intensité
                    x = x * (1.0f - omegaIntensity * 0.5f) + y * (omegaIntensity * 0.5f);
                    // Tuck filter sur le bas
                    x = tuckFilter.processSample((int)ch, x);
                } else {
                    x = y;
                }
                
                data[i] = x;
            }
        }
    }
};
