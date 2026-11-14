#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace Params
{
    // Paramètres de base
    inline constexpr auto pitch = "pitch";
    inline constexpr auto feedback = "feedback"; 
    inline constexpr auto pluck = "pluck";
    inline constexpr auto inharm = "inharm";
    inline constexpr auto inputCutoff = "inputCutoff";
    inline constexpr auto loopCutoff = "loopCutoff";
    inline constexpr auto dryWet = "dryWet";
    inline constexpr auto macro = "macro";
    inline constexpr auto jitter = "jitter";
    inline constexpr auto crossFb = "crossFb";
    inline constexpr auto polarity = "polarity";
    inline constexpr auto selfMod = "selfMod";
    inline constexpr auto wetGain = "wetGain";

    // LFO 1
    inline constexpr auto lfo1Sync = "lfo1Sync";
    inline constexpr auto lfo1Rate = "lfo1Rate";
    inline constexpr auto lfo1Shape = "lfo1Shape";
    inline constexpr auto lfo1Chaos = "lfo1Chaos";
    inline constexpr auto lfo1Pw = "lfo1Pw";

    // LFO 2
    inline constexpr auto lfo2Sync = "lfo2Sync";
    inline constexpr auto lfo2Rate = "lfo2Rate";
    inline constexpr auto lfo2Shape = "lfo2Shape";
    inline constexpr auto lfo2Chaos = "lfo2Chaos";
    inline constexpr auto lfo2Pw = "lfo2Pw";

    // Matrice (12 destinations x 2 LFOs)
    inline constexpr auto m1_pitch = "m1_pitch"; inline constexpr auto m2_pitch = "m2_pitch";
    inline constexpr auto m1_feed = "m1_feed";   inline constexpr auto m2_feed = "m2_feed";
    inline constexpr auto m1_incut = "m1_incut"; inline constexpr auto m2_incut = "m2_incut";
    inline constexpr auto m1_loopcut = "m1_loopcut"; inline constexpr auto m2_loopcut = "m2_loopcut";
    inline constexpr auto m1_hbeta = "m1_hbeta"; inline constexpr auto m2_hbeta = "m2_hbeta";
    inline constexpr auto m1_inharm = "m1_inharm"; inline constexpr auto m2_inharm = "m2_inharm";
    inline constexpr auto m1_cross = "m1_cross"; inline constexpr auto m2_cross = "m2_cross";
    inline constexpr auto m1_self = "m1_self";   inline constexpr auto m2_self = "m2_self";
    inline constexpr auto m1_drift = "m1_drift"; inline constexpr auto m2_drift = "m2_drift";
    inline constexpr auto m1_drywet = "m1_drywet"; inline constexpr auto m2_drywet = "m2_drywet";
    inline constexpr auto m1_wetgain = "m1_wetgain"; inline constexpr auto m2_wetgain = "m2_wetgain";
    inline constexpr auto m1_macro = "m1_macro"; inline constexpr auto m2_macro = "m2_macro";

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        
        // --- Paramètres de base ---
        layout.add(std::make_unique<juce::AudioParameterFloat>(pitch, "Pitch", juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(feedback, "Feedback", juce::NormalisableRange<float>(0.0f, 0.99f, 0.001f), 0.85f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(pluck, "H\u03B2", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(inharm, "Inharm", juce::NormalisableRange<float>(0.0f, 0.99f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(inputCutoff, "In Cutoff", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.3f), 8000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(loopCutoff, "Loop Cutoff", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.3f), 4000.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(dryWet, "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(macro, "Macro", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(jitter, "Drift", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(crossFb, "Cross FB", juce::NormalisableRange<float>(0.0f, 0.98f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(selfMod, "Self-Mod", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(polarity, "Polarity", juce::StringArray{"Positive", "Negative"}, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(wetGain, "Wet Gain", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));

        // --- LFO 1 ---
        juce::StringArray syncOpts = {"Free", "1/1", "1/1 T", "1/1 D", "1/2", "1/2 T", "1/2 D", "1/4", "1/4 T", "1/4 D", "1/8", "1/8 T", "1/8 D", "1/16", "1/16 T", "1/16 D"};
        juce::StringArray shapeOpts = {"Sine", "Tri", "Saw Up", "Saw Dn", "Sqr", "Pulse", "Ramp", "Exp Dec", "Exp Rse", "S&H", "Smth Rnd", "Logistic", "Stairs", "Dbl Sine", "SOMA"};
        
        layout.add(std::make_unique<juce::AudioParameterChoice>(lfo1Sync, "LFO1 Sync", syncOpts, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo1Rate, "LFO1 Rate", juce::NormalisableRange<float>(0.01f, 100.0f, 0.01f, 0.3f), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(lfo1Shape, "LFO1 Shape", shapeOpts, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo1Chaos, "LFO1 Chaos", juce::NormalisableRange<float>(0.1f, 4.0f, 0.01f), 3.6f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo1Pw, "LFO1 PW", juce::NormalisableRange<float>(0.05f, 0.95f, 0.01f), 0.5f));

        // --- LFO 2 ---
        layout.add(std::make_unique<juce::AudioParameterChoice>(lfo2Sync, "LFO2 Sync", syncOpts, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo2Rate, "LFO2 Rate", juce::NormalisableRange<float>(0.01f, 100.0f, 0.01f, 0.3f), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(lfo2Shape, "LFO2 Shape", shapeOpts, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo2Chaos, "LFO2 Chaos", juce::NormalisableRange<float>(0.1f, 4.0f, 0.01f), 3.6f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(lfo2Pw, "LFO2 PW", juce::NormalisableRange<float>(0.05f, 0.95f, 0.01f), 0.5f));

        // --- Matrice (Depth -100% à +100%) ---
        auto addDepth = [&](const juce::String& id, const juce::String& name) {
            layout.add(std::make_unique<juce::AudioParameterFloat>(id, name, juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
        };
        
        addDepth(m1_pitch, "M1 Pitch"); addDepth(m2_pitch, "M2 Pitch");
        addDepth(m1_feed, "M1 Feed"); addDepth(m2_feed, "M2 Feed");
        addDepth(m1_incut, "M1 InCut"); addDepth(m2_incut, "M2 InCut");
        addDepth(m1_loopcut, "M1 LoopCut"); addDepth(m2_loopcut, "M2 LoopCut");
        addDepth(m1_hbeta, "M1 Hβ"); addDepth(m2_hbeta, "M2 Hβ");
        addDepth(m1_inharm, "M1 Inharm"); addDepth(m2_inharm, "M2 Inharm");
        addDepth(m1_cross, "M1 Cross"); addDepth(m2_cross, "M2 Cross");
        addDepth(m1_self, "M1 Self"); addDepth(m2_self, "M2 Self");
        addDepth(m1_drift, "M1 Drift"); addDepth(m2_drift, "M2 Drift");
        addDepth(m1_drywet, "M1 DryWet"); addDepth(m2_drywet, "M2 DryWet");
        addDepth(m1_wetgain, "M1 WetGain"); addDepth(m2_wetgain, "M2 WetGain");
        addDepth(m1_macro, "M1 Macro"); addDepth(m2_macro, "M2 Macro");

        return layout;
    }
}
