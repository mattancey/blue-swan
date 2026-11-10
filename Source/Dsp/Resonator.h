#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>

class Resonator
{
public:
    Resonator();
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setPitchSemitones(float semitones);
    void setFeedback(float feedback);
    void setInputCutoff(float cutoffHz);
    void setLoopCutoff(float cutoffHz);
    void setDryWet(float dryWet);

    void process(juce::AudioBuffer<float>& buffer);

private:
    static constexpr float baseFreqHz = 261.63f;
    static constexpr float maxDelaySamples = 16384.0f;
    double currentSampleRate = 44100.0;
    float currentInputCutoff = 8000.0f;
    float currentLoopCutoff = 4000.0f;

    struct DCBlocker
    {
        float x1 = 0.0f, y1 = 0.0f;
        static constexpr float R = 0.995f;
        float process(float x) noexcept {
            const float y = x - x1 + R * y1;
            x1 = x; y1 = y;
            return y;
        }
        void reset() { x1 = 0.0f; y1 = 0.0f; }
    };

    struct OnePoleLP
    {
        float a = 0.0f, z = 0.0f;
        void setCutoff(float cutoff, float sampleRate) noexcept {
            float x = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / sampleRate);
            a = juce::jlimit(0.0f, 0.99f, x);
        }
        float process(float input) noexcept {
            z = input * (1.0f - a) + z * a;
            return z;
        }
        void reset() { z = 0.0f; }
    };

    struct ChannelState
    {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { (int) maxDelaySamples };
        juce::dsp::StateVariableTPTFilter<float> inputFilter;
        OnePoleLP loopFilter;
        DCBlocker dcBlocker;
    };

    std::vector<ChannelState> channels;
    juce::SmoothedValue<float> pitchSemitonesSmoothed;
    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> dryWetSmoothed;

    float semitonesToDelaySamples(float semitones) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Resonator)
};
