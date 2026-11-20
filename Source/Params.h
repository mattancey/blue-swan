#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace Params
{
    // ... (tous les paramètres existants restent inchangés)
    // Je les omet pour la concision, mais ils doivent être présents.

    // ... (m1_..., m2_..., etc.)

    // --- Nouveaux paramètres Smooth ---
    inline constexpr auto lfo1Smooth = "lfo1Smooth";
    inline constexpr auto lfo2Smooth = "lfo2Smooth";

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        
        // ... (tous les layout.add existants)

        // --- Ajout des paramètres Smooth ---
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo1Smooth, "LFO1 Smooth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo2Smooth, "LFO2 Smooth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

        return layout;
    }
}
