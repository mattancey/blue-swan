#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace Params
{
    inline constexpr auto pitch = "pitch";
    inline constexpr auto feedback = "feedback";
    inline constexpr auto inputCutoff = "inputCutoff";
    inline constexpr auto loopCutoff = "loopCutoff";
    inline constexpr auto dryWet = "dryWet";

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(std::make_unique<juce::AudioParameterFloat>(pitch, "Pitch", juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(feedback, "Feedback", juce::NormalisableRange<float>(0.0f, 0.98f, 0.001f), 0.85f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(inputCutoff, "Input Cutoff", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.3f), 8000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(loopCutoff, "Loop Cutoff", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.3f), 4000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(dryWet, "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));
        return layout;
    }
}
