#pragma once
#include <juce_core/juce_core.h>
#include <cmath>
#include <random>

class LFO
{
public:
    LFO() { gen.seed(std::random_device{}()); }
    void prepare(double sr) { sampleRate = sr; phase = 0.0; shValue = 0.0f; smoothValue = 0.0f; logX = 0.5f; }
    void setShape(int shape) { currentShape = shape; }
    void setPulseWidth(float w) { pulseWidth = w; }

    // Nouvelle méthode pour traiter un bloc entier sans dérive de phase
    float processBlock(float frequencyHz, int numSamples, float phaseOffsetDegrees)
    {
        phase += (frequencyHz * (float)numSamples) / (float)sampleRate;
        if (phase >= 1.0f) {
            phase -= std::floor(phase); // Garde la phase entre 0 et 1
            shValue = dist(gen);
            logX = 0.1f + (dist(gen) * 0.5f + 0.5f) * 0.8f; 
        }

        float p = phase + (phaseOffsetDegrees / 360.0f);
        if (p >= 1.0f) p -= 1.0f;
        if (p < 0.0f) p += 1.0f;

        float val = 0.0f;
        switch (currentShape) {
            case 0: val = std::sin(p * 6.2831853f); break;
            case 1: val = 4.0f * std::abs(p - 0.5f) - 1.0f; break;
            case 2: val = 2.0f * p - 1.0f; break;
            case 3: val = p < 0.5f ? 1.0f : -1.0f; break;
            case 4: val = p < pulseWidth ? 1.0f : -1.0f; break;
            case 5: val = p < 0.9f ? (p / 0.9f) * 2.0f - 1.0f : -1.0f; break;
            case 6: val = std::exp(-5.0f * p) * 2.0f - 1.0f; break;
            case 7: val = (1.0f - std::exp(-5.0f * p)) * 2.0f - 1.0f; break;
            case 8: val = shValue; break;
            case 9: smoothValue += 0.01f * (shValue - smoothValue); val = smoothValue; break;
            case 10: logX = 3.6f * logX * (1.0f - logX); val = logX * 2.0f - 1.0f; break;
            case 11: val = std::floor(p * 4.0f) / 3.0f * 2.0f - 1.0f; break;
            case 12: val = std::sin(p * 4.0f * 3.1415926f); break;
            case 13: val = std::sin(p * 3.1415926f) * 1.5f; if (val > 1.0f) val = 2.0f - val; if (val < -1.0f) val = -2.0f - val; break;
            default: val = 0.0f; break;
        }
        return val;
    }

    // Ancienne méthode (utilisée si besoin)
    float process(float frequencyHz, float phaseOffsetDegrees) { return processBlock(frequencyHz, 1, phaseOffsetDegrees); }
    
    // Récupérer la phase actuelle
    float getPhase() const { return phase; }
    void setPhase(float p) { phase = p; }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;
    int currentShape = 0;
    float pulseWidth = 0.5f;
    float shValue = 0.0f;
    float smoothValue = 0.0f;
    float logX = 0.5f;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };
};
