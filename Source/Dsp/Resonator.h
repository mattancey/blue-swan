#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <random>
#include <cmath>
#include "LFO.h"

struct ModParams {
    float pitch, feedback, pluck, inharm, inputCutoff, loopCutoff, dryWet, jitter, crossFb, selfMod, wetGain, macro;
    int lfo1Shape, lfo1Sync, lfo2Shape, lfo2Sync;
    float lfo1Rate, lfo1Chaos, lfo1Pw, lfo2Rate, lfo2Chaos, lfo2Pw;
    float m1_pitch, m1_feed, m1_incut, m1_loopcut, m1_hbeta, m1_inharm, m1_cross, m1_self, m1_drift, m1_drywet, m1_wetgain, m1_macro;
    float m2_pitch, m2_feed, m2_incut, m2_loopcut, m2_hbeta, m2_inharm, m2_cross, m2_self, m2_drift, m2_drywet, m2_wetgain, m2_macro;
    float m1_lfo2, m2_lfo1; // Croisements uniquement
};

class Resonator
{
public:
    Resonator();
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, const ModParams& p, double bpm, bool isNegative);

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

    struct Allpass {
        float a = 0.0f;
        float x1 = 0.0f, y1 = 0.0f;
        void setCoeff(float coeff) { a = juce::jlimit(-0.99f, 0.99f, coeff); }
        float process(float x) noexcept { float y = -a * x + x1 + a * y1; x1 = x; y1 = y; return y; }
        void reset() { x1 = 0.0f; y1 = 0.0f; }
    };

    struct ChannelState {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { (int) maxDelaySamples };
        juce::dsp::DelayLine<float> pluckDelay { 4000 }; 
        juce::dsp::StateVariableTPTFilter<float> inputFilter;
        OnePoleLP loopFilter;
        DCBlocker dcBlocker;
        OnePoleLP modFilter;
        Allpass inharmAllpass[24];
    };

    std::vector<ChannelState> channels;
    juce::SmoothedValue<float> gateSmoothed;
    LFO lfo1, lfo2;
    
    float lfo1Value = 0.0f;
    float lfo2Value = 0.0f;
    
    std::mt19937 jitterGen;
    std::uniform_real_distribution<float> jitterDist { -1.0f, 1.0f };
    float jitterState[2] = {0.0f, 0.0f};
};
