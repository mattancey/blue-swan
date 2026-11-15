#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/Widgets.h"

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
    
    float currentHeight = 480.0f;
    float targetHeight = 480.0f;

    juce::Slider macroSlider, pitchSlider, decaySlider, pluckSlider, inharmSlider, inputCutoffSlider, loopCutoffSlider, dryWetSlider, jitterSlider, crossFbSlider, selfModSlider, wetGainSlider;
    juce::Label macroLabel, pitchLabel, decayLabel, pluckLabel, inharmLabel, inputCutoffLabel, loopCutoffLabel, dryWetLabel, jitterLabel, crossFbLabel, selfModLabel, wetGainLabel;
    juce::ComboBox polarityBox;
    juce::Label polarityLabel;
    
    juce::TextButton drawerButton { "MODULATION" };

    juce::Slider m1Sliders[14], m2Sliders[14];
    juce::Label matrixLabels[14];
    
    // Bypass LFO (remplace les Labels LFO)
    juce::TextButton lfo1BypassButton, lfo2BypassButton;
    
    CustomComboBox lfo1SyncBox, lfo1ShapeBox, lfo2SyncBox, lfo2ShapeBox;
    juce::Slider lfo1RateSlider, lfo1PwSlider;
    juce::Slider lfo2RateSlider, lfo2PwSlider;
    
    juce::Label lfo1RateNameLabel, lfo1PwNameLabel;
    juce::Label lfo2RateNameLabel, lfo2PwNameLabel;
    
    juce::Label lfo1RateValueLabel, lfo1PwValueLabel;
    juce::Label lfo2RateValueLabel, lfo2PwValueLabel;
    
    juce::Label lfo1RowLabel, lfo2RowLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    std::unique_ptr<SliderAttachment> mainAtts[12], m1Atts[14], m2Atts[14];
    std::unique_ptr<ComboAttachment> polarityAtt;
    std::unique_ptr<ComboAttachment> lfo1SyncAtt, lfo1ShapeAtt, lfo2SyncAtt, lfo2ShapeAtt;
    std::unique_ptr<SliderAttachment> lfo1RateAtt, lfo1PwAtt, lfo2RateAtt, lfo2PwAtt;
    std::unique_ptr<ButtonAttachment> lfo1BypassAtt, lfo2BypassAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
