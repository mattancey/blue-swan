#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <random>
#include "LFO.h"
#include "Saturation.h"

class Resonator
{
public:
    Resonator();
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setParameter(int index, float value); // Pour la matrice
    void setFreeze(bool shouldFreeze);
    
    void process(juce::AudioBuffer<float>& buffer, 
                 float pitch, float fb, float inCut, float loopCut, float res, float mix, 
                 float drift, float crossFB, float selfMod, bool isNeg, 
                 float driveAmt, int driveType, float omegaInt, bool oversample,
                 float wetGain, float tilt,
                 int lfoShape, float lfoRate, float lfoPhase, float lfoChaos, float lfoPulse, bool lfoSync, float bpm,
                 int m1D, float m1Dep, int m2D, float m2Dep, int m3D, float m3Dep, int m4D, float m4Dep);

private:
    static constexpr float baseFreqHz = 261.63f;
    static constexpr float maxDelaySamples = 16384.0f;
    double currentSampleRate = 44100.0;

    struct DCBlocker {
        float x1 = 0.0f, y1 = 0.0f;
        static constexpr float R = 0.995f;
        float process(float x) noexcept { const float y = x - x1 + R * y1; x1 = x; y1 = y; return y; }
        void reset() { x1 = 0.0f; y1 = 0.0f; }
    };

    struct OnePoleLP {
        float a = 0.0f, z = 0.0f;
        void setCutoff(float cutoff, float sampleRate) noexcept {
            float x = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / sampleRate);
            a = juce::jlimit(0.0f, 0.99f, x);
        }
        float process(float input) noexcept { z = input * (1.0f - a) + z * a; return z; }
        void reset() { z = 0.0f; }
    };

    struct ChannelState {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { (int) maxDelaySamples };
        juce::dsp::StateVariableTPTFilter<float> inputFilter;
        juce::dsp::StateVariableTPTFilter<float> loopFilter;
        DCBlocker dcBlocker;
        OnePoleLP modFilter;
    };

    std::vector<ChannelState> channels;
    juce::SmoothedValue<float> envFollower;
    juce::SmoothedValue<float> wetGainSmoothed;
    
    LFO lfo;
    Saturation saturation;
    
    bool isFrozen = false;
    float lfoPhaseCont = 0.0f;
    float freezeTiltState = 0.0f;

    std::mt19937 jitterGen;
    std::uniform_real_distribution<float> jitterDist { -1.0f, 1.0f };
    float jitterState[2] = {0.0f, 0.0f};

    float semitonesToDelaySamples(float semitones) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Resonator)
};
