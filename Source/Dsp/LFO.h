#pragma once
#include <juce_core/juce_core.h>
#include <cmath>
#include <random>

class LFO
{
public:
    LFO() { gen.seed(std::random_device{}()); }
    void prepare(double sr) { 
        sampleRate = sr; 
        phase = 0.0; 
        shValue = 0.0f; 
        smoothValue = 0.0f; 
        logX = 0.5f; 
        smoothedOutput.reset(sr, 0.01); 
    }
    void setShape(int shape) { currentShape = shape; }
    void setPulseWidth(float w) { pulseWidth = w; }
    void setSmooth(float smoothAmount) { 
        // Mappe 0..1 vers un temps de lissage de 1ms à 100ms
        smoothedOutput.reset(sampleRate, 0.001 + smoothAmount * 0.099); 
    }

    float processBlock(float frequencyHz, int numSamples, float phaseOffsetDegrees, float sr)
    {
        phase += (frequencyHz * (float)numSamples) / sr;
        if (phase >= 1.0f) {
            phase -= std::floor(phase);
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
            case 6: val = (1.0f - std::exp(-5.0f * p)) * 2.0f - 1.0f; break;
            case 7: val = shValue; break;
            case 8: smoothValue += 0.01f * (shValue - smoothValue); val = smoothValue; break;
            case 9: logX = 3.6f * logX * (1.0f - logX); val = logX * 2.0f - 1.0f; break;
            case 10: val = std::floor(p * 4.0f) / 3.0f * 2.0f - 1.0f; break;
            case 11: val = std::sin(p * 4.0f * 3.1415926f); break;
            case 12: val = std::sin(p * 3.1415926f) * 1.5f; if (val > 1.0f) val = 2.0f - val; if (val < -1.0f) val = -2.0f - val; break;
            default: val = 0.0f; break;
        }
        
        // On applique le lissage ici
        smoothedOutput.setTargetValue(val);
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
    
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedOutput;
};
