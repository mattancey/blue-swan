#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace Params
{
    // Core
    inline constexpr auto pitch = "pitch";
    inline constexpr auto feedback = "feedback";
    inline constexpr auto inputCutoff = "inputCutoff";
    inline constexpr auto loopCutoff = "loopCutoff";
    inline constexpr auto resonance = "resonance";
    inline constexpr auto dryWet = "dryWet";
    inline constexpr auto macro = "macro";
    // Modulations & Dynamique
    inline constexpr auto jitter = "jitter";
    inline constexpr auto crossFb = "crossFb";
    inline constexpr auto selfMod = "selfMod";
    inline constexpr auto polarity = "polarity";
    // Saturation
    inline constexpr auto driveType = "driveType";
    inline constexpr auto driveAmount = "driveAmount";
    inline constexpr auto omegaIntensity = "omegaIntensity";
    inline constexpr auto omegaOversample = "omegaOversample";
    // LFO
    inline constexpr auto lfoSync = "lfoSync";
    inline constexpr auto lfoRate = "lfoRate";
    inline constexpr auto lfoPhase = "lfoPhase";
    inline constexpr auto lfoShape = "lfoShape";
    inline constexpr auto lfoChaos = "lfoChaos";
    inline constexpr auto lfoPulseWidth = "lfoPulseWidth";
    // Matrix (4 Slots)
    inline constexpr auto m1Dest = "m1Dest"; inline constexpr auto m1Depth = "m1Depth";
    inline constexpr auto m2Dest = "m2Dest"; inline constexpr auto m2Depth = "m2Depth";
    inline constexpr auto m3Dest = "m3Dest"; inline constexpr auto m3Depth = "m3Depth";
    inline constexpr auto m4Dest = "m4Dest"; inline constexpr auto m4Depth = "m4Depth";
    // Output & Freeze
    inline constexpr auto wetGain = "wetGain";
    inline constexpr auto freeze = "freeze";
    inline constexpr auto freezeTilt = "freezeTilt";

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        
        // Core
        layout.add(std::make_unique<juce::AudioParameterFloat>(pitch, "Pitch", juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(feedback, "Feedback", juce::NormalisableRange<float>(0.0f, 0.99f, 0.001f), 0.85f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(inputCutoff, "In Cutoff", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.3f), 8000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(loopCutoff, "Loop Cutoff", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.3f), 4000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(resonance, "Resonance", juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 0.707f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(dryWet, "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(macro, "Macro", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        
        // Modulations & Dynamique
        layout.add(std::make_unique<juce::AudioParameterFloat>(jitter, "Drift", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(crossFb, "Cross FB", juce::NormalisableRange<float>(0.0f, 0.98f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(selfMod, "Self-Mod", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(polarity, "Polarity", juce::StringArray{"Positive", "Negative"}, 0));
        
        // Saturation
        layout.add(std::make_unique<juce::AudioParameterChoice>(driveType, "Drive Type", juce::StringArray{"Soft", "Asymmetric", "Fold"}, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(driveAmount, "Drive", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(omegaIntensity, "Omega 2N", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterBool>(omegaOversample, "Oversample", true));
        
        // LFO
        layout.add(std::make_unique<juce::AudioParameterChoice>(lfoSync, "LFO Sync", juce::StringArray{"Free", "1/1", "1/1 T", "1/1 D", "1/2", "1/2 T", "1/2 D", "1/4", "1/4 T", "1/4 D", "1/8", "1/8 T", "1/8 D", "1/16", "1/16 T", "1/16 D"}, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfoRate, "LFO Rate/Phase", juce::NormalisableRange<float>(0.05f, 30.0f, 0.01f, 0.3f), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfoPhase, "LFO Phase", juce::NormalisableRange<float>(0.0f, 360.0f, 1.0f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(lfoShape, "LFO Shape", juce::StringArray{"Sine", "Triangle", "Saw Up", "Saw Down", "Square", "Pulse", "Ramp", "Exp Decay", "Exp Rise", "S&H", "Smooth Rnd", "Logistic", "Staircase", "Double Sine", "SOMA"}, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfoChaos, "Chaos", juce::NormalisableRange<float>(0.1f, 4.0f, 0.01f), 3.6f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfoPulseWidth, "Pulse Width", juce::NormalisableRange<float>(0.05f, 0.95f, 0.01f), 0.5f));
        
        // Matrix
        juce::StringArray dests = {"None", "Pitch", "Feedback", "In Cutoff", "Loop Cutoff", "Resonance", "Dry/Wet", "Drift", "Cross FB", "Self-Mod", "Drive", "Omega 2N", "Wet Gain", "Freeze Tilt"};
        for (int i = 1; i <= 4; ++i) {
            layout.add(std::make_unique<juce::AudioParameterChoice>("m" + juce::String(i) + "Dest", "M" + juce::String(i) + " Dest", dests, 0));
            layout.add(std::make_unique<juce::AudioParameterFloat>("m" + juce::String(i) + "Depth", "M" + juce::String(i) + " Depth", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
        }
        
        // Output & Freeze
        layout.add(std::make_unique<juce::AudioParameterFloat>(wetGain, "Wet Gain", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterBool>(freeze, "Freeze", false));
        layout.add(std::make_unique<juce::AudioParameterFloat>(freezeTilt, "Freeze Tilt", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));

        return layout;
    }
}
