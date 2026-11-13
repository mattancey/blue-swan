#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <random>

// --- Structure du Filtre SEM (SVF ZDF avec saturation tanh) ---
struct SEMFilter
{
    float ic1eq = 0.0f;
    float ic2eq = 0.0f;
    float g = 0.0f;
    float k = 2.0f;

    void prepare(double sampleRate, float cutoffHz, float resonance)
    {
        reset();
        setParams(sampleRate, cutoffHz, resonance);
    }

    void setParams(double sampleRate, float cutoffHz, float resonance)
    {
        // Pré-warping de la fréquence de coupure
        float wd = juce::MathConstants<float>::twoPi * cutoffHz;
        float T = 1.0f / (float)sampleRate;
        float wa = (2.0f / T) * std::tan(wd * T * 0.5f);
        g = wa * T * 0.5f;
        
        // Mapping de la résonance (0 à 1) vers l'amortissement k
        // k = 2 - 2*resonance donne une résonance douce qui n'explose pas
        k = 2.0f - 2.0f * juce::jlimit(0.0f, 0.98f, resonance);
    }

    float processSample(float input)
    {
        // Topologie SVF ZDF (Zavalishin / Simper)
        float t0 = input - ic2eq;
        float t1 = g * t0 + ic1eq;
        float t2 = g * t1 + ic2eq;
        
        // Saturation non-linéaire sur les états (le secret du SEM)
        ic1eq = 2.0f * std::tanh(t1) - ic1eq;
        ic2eq = 2.0f * std::tanh(t2) - ic2eq;
        
        // Sortie Lowpass
        return ic2eq;
    }

    void reset()
    {
        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }
};
// -------------------------------------------------------------

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
    void setLoopResonance(float q); 
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
    float currentLoopRes = 0.0f;

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
        juce::dsp::StateVariableTPTFilter<float> inputFilter; // Garde le filtre d'entrée JUCE (pas dans la boucle)
        SEMFilter loopFilter; // Notre filtre SEM custom
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
    
    float polarityMultiplier = 1.0f;

    std::mt19937 jitterGen;
    std::uniform_real_distribution<float> jitterDist { -1.0f, 1.0f };
    float jitterState[2] = {0.0f, 0.0f};

    float semitonesToDelaySamples(float semitones) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Resonator)
};
