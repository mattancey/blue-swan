#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <random>
#include <cmath> // Ajouté pour std::exp et std::pow

struct Vactrol
{
    float state = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    
    void prepare(double sampleRate, float attackMs = 5.0f, float releaseMs = 250.0f)
    {
        reset();
        attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * (float)sampleRate));
        releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * (float)sampleRate));
    }
    
    float process(float target)
    {
        if (target > state)
            state = target + (state - target) * attackCoeff;
        else
            state = target + (state - target) * releaseCoeff;
        
        return state * state;
    }
    
    void reset() { state = 0.0f; }
};

struct BuchlaLPG
{
    float filterState = 0.0f;
    
    void prepare(double sampleRate) { reset(); }
    
    float processSample(float input, float vactrolOut, int mode, bool deep, double sampleRate)
    {
        float vcaGain = vactrolOut;
        
        float cutoff = 20.0f * std::pow(1000.0f, vactrolOut);
        if (deep) cutoff *= 0.25f;
        
        float a = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / (float)sampleRate);
        filterState = input * (1.0f - a) + filterState * a;
        
        if (mode == 0) return filterState;
        else if (mode == 1) return input * vcaGain;
        else return filterState * vcaGain;
    }
    
    void reset() { filterState = 0.0f; }
};

class Resonator
{
public:
    Resonator();
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setPitchSemitones(float semitones);
    void setFeedback(float feedback);
    void setInputCutoff(float cutoffHz);
    void setBuchla292(float amount);
    void setLpgMode(int mode);
    void setLpgDeep(bool deep);
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
    float currentBuchla292 = 0.5f;
    int currentLpgMode = 2;
    bool currentLpgDeep = false;

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
        Vactrol vactrol;
        BuchlaLPG lpg;
        DCBlocker dcBlocker;
        OnePoleLP modFilter;
    };

    std::vector<ChannelState> channels;
    juce::SmoothedValue<float> pitchSemitonesSmoothed;
    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> dryWetSmoothed;
    juce::SmoothedValue<float> jitterSmoothed;
    juce::SmoothedValue<float> crossFbSmoothed;
    juce::SmoothedValue<float> selfModSmoothed;
    juce::SmoothedValue<float> wetGainSmoothed;
    juce::SmoothedValue<float> gateSmoothed;
    juce::SmoothedValue<float> buchla292Smoothed;
    
    float polarityMultiplier = 1.0f;

    std::mt19937 jitterGen;
    std::uniform_real_distribution<float> jitterDist { -1.0f, 1.0f };
    float jitterState[2] = {0.0f, 0.0f};

    float semitonesToDelaySamples(float semitones) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Resonator)
};
