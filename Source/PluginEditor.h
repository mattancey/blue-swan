#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class BlueSwanAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor&);
    ~BlueSwanAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BlueSwanAudioProcessor& processorRef;
    
    juce::Slider macroSlider, pitchSlider, feedbackSlider, inputCutoffSlider, buchla292Slider, dryWetSlider, jitterSlider, crossFbSlider, selfModSlider, wetGainSlider;
    juce::Label macroLabel, pitchLabel, feedbackLabel, inputCutoffLabel, buchla292Label, dryWetLabel, jitterLabel, crossFbLabel, selfModLabel, wetGainLabel;
    
    juce::ComboBox polarityBox, lpgModeBox;
    juce::Label polarityLabel, lpgModeLabel;
    juce::ToggleButton lpgDeepButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> macroAtt, pitchAtt, feedbackAtt, inputCutoffAtt, buchla292Att, dryWetAtt, jitterAtt, crossFbAtt, selfModAtt, wetGainAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> polarityAtt, lpgModeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lpgDeepAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
