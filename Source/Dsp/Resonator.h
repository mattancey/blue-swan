#pragma once
#include <juce_dsp/juce_dsp.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h> // <-- Nouvel include
#include <vector>
#include <random>
#include <cmath>
#include "LFO.h"

// ... (struct ModParams inchangé, sauf l'ajout des smooth)
struct ModParams {
    // ...
    float lfo1Smooth, lfo2Smooth;
    // ...
};

class Resonator
{
public:
    Resonator();
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, const ModParams& p, double bpm, bool isNegative);
    
    // ... (le reste est inchangé)

private:
    // ...
    std::unique_ptr<chowdsp::VariableOversampling<float>> oversampling; // <-- Nouveau

    // ... (le reste est inchangé)
};
