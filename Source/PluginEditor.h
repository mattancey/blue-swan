#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class BlueSwanAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor&);
    ~BlueSwanAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    BlueSwanAudioProcessor& processorRef;
    bool isDrawerOpen = false;

    // UI Principale
    juce::Slider macroSlider, pitchSlider, decaySlider, pluckSlider, inharmSlider, inputCutoffSlider, loopCutoffSlider, dryWetSlider, jitterSlider, crossFbSlider, selfModSlider, wetGainSlider;
    juce::Label macroLabel, pitchLabel, decayLabel, pluckLabel, inharmLabel, inputCutoffLabel, loopCutoffLabel, dryWetLabel, jitterLabel, crossFbLabel, selfModLabel, wetGainLabel;
    juce::ComboBox polarityBox;
    juce::Label polarityLabel;
    
    // Bouton Tiroir
    juce::TextButton drawerButton { "MODULATION" };

    // UI Tiroir (Matrice)
    juce::Slider m1Sliders[12], m2Sliders[12];
    juce::Label matrixLabels[12];
    
    // UI Tiroir (LFOs)
    juce::ComboBox lfo1SyncBox, lfo1ShapeBox, lfo2SyncBox, lfo2ShapeBox;
    juce::Slider lfo1RateSlider, lfo1ChaosSlider, lfo1PwSlider;
    juce::Slider lfo2RateSlider, lfo2ChaosSlider, lfo2PwSlider;
    juce::Label lfo1RateLabel, lfo1ChaosLabel, lfo1PwLabel;
    juce::Label lfo2RateLabel, lfo2ChaosLabel, lfo2PwLabel;

    // Attachements
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    std::unique_ptr<SliderAttachment> mainAtts[12], m1Atts[12], m2Atts[12];
    std::unique_ptr<ComboAttachment> polarityAtt;
    
    // Attachements LFO (Maintenant stockés en tant que membres)
    std::unique_ptr<ComboAttachment> lfo1SyncAtt, lfo1ShapeAtt, lfo2SyncAtt, lfo2ShapeAtt;
    std::unique_ptr<SliderAttachment> lfo1RateAtt, lfo1ChaosAtt, lfo1PwAtt, lfo2RateAtt, lfo2ChaosAtt, lfo2PwAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
