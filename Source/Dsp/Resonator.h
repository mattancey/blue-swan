#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <random>
#include <cmath>

class Resonator
{
public:
    Resonator();
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setPitchSemitones(float semitones);
    void setDecay(float decay);
    void setEksEnable(bool enable);
    void setPluck(float pluck);
    void setInharm(float inharm);
    void setInharmJitter(float jitter);
    void setInputCutoff(float cutoffHz);
    void setLoopCutoff(float cutoffHz);
    void setDryWet(float dryWet);
    void setJitter(float jitterAmount);
    void setCrossFeedback(float crossAmt);
    void setPolarity(bool isNegative);
    void setSelfMod(float selfModAmount);
    void setWetGain(float gain);

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

    struct Allpass
    {
        float a = 0.0f;
        float x1 = 0.0f, y1 = 0.0f;
        void setCoeff(float coeff) { a = juce::jlimit(-0.99f, 0.99f, coeff); }
        float process(float x) noexcept {
            float y = -a * x + x1 + a * y1;
            x1 = x; y1 = y;
            return y;
        }
        void reset() { x1 = 0.0f; y1 = 0.0f; }
    };

    struct ChannelState
    {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { (int) maxDelaySamples };
        juce::dsp::DelayLine<float> pluckDelay { 4000 }; 
        juce::dsp::StateVariableTPTFilter<float> inputFilter;
        OnePoleLP loopFilter;
        DCBlocker dcBlocker;
        OnePoleLP modFilter;
        Allpass inharmAllpass[24]; // Changé de 16 à 24
    };

    std::vector<ChannelState> channels;
    
    juce::SmoothedValue<float> pitchSemitonesSmoothed;
    juce::SmoothedValue<float> decaySmoothed;
    juce::SmoothedValue<float> pluckSmoothed;
    juce::SmoothedValue<float> inharmSmoothed;
    juce::SmoothedValue<float> inharmJitSmoothed; 
    bool isEksEnabled = true;
    
    juce::SmoothedValue<float> dryWetSmoothed;
    juce::SmoothedValue<float> jitterSmoothed;
    juce::SmoothedValue<float> crossFbSmoothed;
    juce::SmoothedValue<float> selfModSmoothed;
    juce::SmoothedValue<float> wetGainSmoothed;
    juce::SmoothedValue<float> gateSmoothed;
    
    float polarityMultiplier = 1.0f;
    float inharmLfoPhase = 0.0f; // LFO pour le Jit

    std::mt19937 jitterGen;
    std::uniform_real_distribution<float> jitterDist { -1.0f, 1.0f };
    float jitterState[2] = {0.0f, 0.0f};

    float semitonesToDelaySamples(float semitones) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Resonator)
};
