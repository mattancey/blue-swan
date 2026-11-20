#include "Resonator.h"

Resonator::Resonator()
{
    // ... (initialisation inchangée)
}

void Resonator::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    // ... (préparation des filtres, etc. inchangée)

    // --- Initialisation de ChowDSP VariableOversampling ---
    oversampling = std::make_unique<chowdsp::VariableOversampling<float>>(numChannels, sampleRate, samplesPerBlock);
}

void Resonator::process(juce::AudioBuffer<float>& buffer, const ModParams& p, double bpm, bool isNegative)
{
    // ... (lecture des paramètres et application de la matrice)

    // --- Gestion de l'oversampling ---
    auto osFactor = static_cast<chowdsp::VariableOversampling<float>::OSFactor>(p.oversampling); // 0, 1, 2 pour 1x, 2x, 4x
    oversampling->setOSFactor(osFactor);

    juce::dsp::AudioBlock<float> block(buffer);
    auto upBlock = oversampling->processSamplesUp(block);
    
    // Traitement interne sur le bloc suréchantillonné
    processInternal(upBlock, p, bpm, isNegative, currentSampleRate * (double)oversampling->getOSFactor());

    oversampling->processSamplesDown(block);
}

// ... (processInternal inchangé)
