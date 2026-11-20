#pragma once
#include <juce_core/juce_core.h>
#include <cmath>
#include <random>

class LFO
{
public:
    LFO() { gen.seed(std::random_device{}()); }
    void prepare(double sr) { sampleRate = sr; phase = 0.0; shValue = 0.0f; smoothValue = 0.0f; logX = 0.5f; smoothedOutput.reset(sr, 0.01); } // 10ms par défaut
    void setShape(int shape) { currentShape = shape; }
    void setPulseWidth(float w) { pulseWidth = w; }
    void setSmooth(float smoothAmount) { smoothedOutput.reset(sampleRate, 0.001 + smoothAmount * 0.099); } // 1ms à 100ms

    float processBlock(float frequencyHz, int numSamples, float phaseOffsetDegrees, float sr)
    {
        // ... (calcul de la forme d'onde brute, inchangé)
        float rawValue = ...; // le switch case existant

        // Application du lissage
        smoothedOutput.setTargetValue(rawValue);
        return smoothedOutput.getNextValue();
    }

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
    
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedOutput; // <-- Nouveau
};
