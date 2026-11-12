#pragma once
#include <cmath>
#include <random>

class LFO
{
public:
    LFO() { gen.seed(std::random_device{}()); }
    
    void prepare(double sr) { sampleRate = sr; phase = 0.0; shValue = 0.0f; smoothValue = 0.0f; logX = 0.5f; }
    
    void setShape(int shape) { currentShape = shape; }
    void setChaos(float c) { chaos = c; }
    void setPulseWidth(float w) { pulseWidth = w; }

    // Advance phase and return value between -1 and 1
    float process(float frequencyHz, float phaseOffsetDegrees)
    {
        phase += frequencyHz / (float)sampleRate;
        if (phase >= 1.0f) {
            phase -= 1.0f;
            // Actions au changement de cycle
            shValue = dist(gen);
            logX = 0.1f + (dist(gen) * 0.5f + 0.5f) * 0.8f; // initialisation chaos
        }

        float p = phase + (phaseOffsetDegrees / 360.0f);
        if (p >= 1.0f) p -= 1.0f;
        if (p < 0.0f) p += 1.0f;

        float val = 0.0f;
        switch (currentShape) {
            case 0: val = std::sin(p * 6.2831853f); break; // Sine
            case 1: val = 4.0f * std::abs(p - 0.5f) - 1.0f; break; // Triangle
            case 2: val = 2.0f * p - 1.0f; break; // Saw Up
            case 3: val = 1.0f - 2.0f * p; break; // Saw Down
            case 4: val = p < 0.5f ? 1.0f : -1.0f; break; // Square
            case 5: val = p < pulseWidth ? 1.0f : -1.0f; break; // Pulse
            case 6: val = p < 0.9f ? (p / 0.9f) * 2.0f - 1.0f : -1.0f; break; // Ramp
            case 7: val = std::exp(-5.0f * p) * 2.0f - 1.0f; break; // Exp Decay
            case 8: val = (1.0f - std::exp(-5.0f * p)) * 2.0f - 1.0f; break; // Exp Rise
            case 9: val = shValue; break; // S&H
            case 10: // Smooth Random
                smoothValue += 0.01f * (shValue - smoothValue);
                val = smoothValue;
                break;
            case 11: // Logistic Map
                logX = chaos * logX * (1.0f - logX);
                val = logX * 2.0f - 1.0f;
                break;
            case 12: val = std::floor(p * 4.0f) / 3.0f * 2.0f - 1.0f; break; // Staircase
            case 13: val = std::sin(p * 4.0f * 3.1415926f); break; // Double Sine
            case 14: val = std::sin(p * 3.1415926f) * 1.5f; if (val > 1.0f) val = 2.0f - val; if (val < -1.0f) val = -2.0f - val; break; // SOMA (Fold)
            default: val = 0.0f; break;
        }
        return val;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;
    int currentShape = 0;
    float chaos = 3.6f;
    float pulseWidth = 0.5f;
    float shValue = 0.0f;
    float smoothValue = 0.0f;
    float logX = 0.5f;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };
};
