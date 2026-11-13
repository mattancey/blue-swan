#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <random>

// --- Modèle de Vactrol (inspiré de Parker & D'Angelo) ---
// Réponse asymétrique : attaque rapide, relâchement lent et non-linéaire.
struct Vactrol
{
    float state = 0.0f;          // État interne (0 = fermé, 1 = ouvert)
    float attackCoeff = 0.0f;    // Coefficient d'attaque (rapide)
    float releaseCoeff = 0.0f;   // Coefficient de relâchement (lent)
    
    void prepare(double sampleRate, float attackMs = 5.0f, float releaseMs = 250.0f)
    {
        reset();
        attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * (float)sampleRate));
        releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * (float)sampleRate));
    }
    
    float process(float target)
    {
        // Attaque rapide si la cible est plus grande, relâchement lent sinon
        if (target > state)
            state = target + (state - target) * attackCoeff;
        else
            state = target + (state - target) * releaseCoeff;
        
        // Courbe non-linéaire (le vactrol n'est pas linéaire en lumière)
        // On applique une courbe en puissance pour adoucir le relâchement
        return state * state; // Courbe quadratique simple
    }
    
    void reset() { state = 0.0f; }
};

// --- Module Buchla 292 (LPG) ---
// Combine un filtre passe-bas 6dB et un VCA, tous deux contrôlés par le vactrol.
struct BuchlaLPG
{
    // Filtre passe-bas 6dB (un pôle) pour la partie VCF
    float filterState = 0.0f;
    float filterCoeff = 0.0f;
    
    void prepare(double sampleRate)
    {
        reset();
        // Le coefficient sera recalculé à chaque échantillon
    }
    
    float processSample(float input, float vactrolOut, int mode, bool deep)
    {
        // vactrolOut est dans [0, 1] (0 = fermé, 1 = ouvert)
        
        // --- Partie VCA ---
        float vcaGain = vactrolOut;
        
        // --- Partie VCF (filtre passe-bas 6dB) ---
        // La fréquence de coupure dépend du vactrol et du mode Deep
        // Plage : 20 Hz (fermé) à 20 kHz (ouvert)
        float cutoff = 20.0f * std::pow(1000.0f, vactrolOut); // 20 * 1000^vactrolOut
        if (deep) cutoff *= 0.25f; // Divise par 4 en mode Deep
        
        // Coefficient du filtre à un pôle : a = exp(-2*pi*fc/fs)
        float a = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / 44100.0f); // fs fixé pour simplifier
        // Note : dans le vrai code, fs est stocké dans la classe Resonator.
        
        // Traitement du filtre
        filterState = input * (1.0f - a) + filterState * a;
        
        // --- Sortie selon le mode ---
        if (mode == 0) // VCF only
            return filterState;
        else if (mode == 1) // VCA only
            return input * vcaGain;
        else // Both (LPG)
            return filterState * vcaGain;
    }
    
    void reset() { filterState = 0.0f; }
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
    void setBuchla292(float amount);   // Nouveau
    void setLpgMode(int mode);         // Nouveau
    void setLpgDeep(bool deep);        // Nouveau
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
    int currentLpgMode = 2; // Both par défaut
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
        Vactrol vactrol;        // Nouveau
        BuchlaLPG lpg;          // Nouveau
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
    juce::SmoothedValue<float> buchla292Smoothed; // Pour lisser le contrôle
    
    float polarityMultiplier = 1.0f;

    std::mt19937 jitterGen;
    std::uniform_real_distribution<float> jitterDist { -1.0f, 1.0f };
    float jitterState[2] = {0.0f, 0.0f};

    float semitonesToDelaySamples(float semitones) const noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Resonator)
};
