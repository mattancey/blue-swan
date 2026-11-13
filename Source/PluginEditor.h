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
    
    juce::Slider macroSlider, pitchSlider, feedbackSlider, inputCutoffSlider, loopCutoffSlider, dryWetSlider, jitterSlider, crossFbSlider, selfModSlider, wetGainSlider, freezeTiltSlider;
    juce::Label macroLabel, pitchLabel, feedbackLabel, inputCutoffLabel, loopCutoffLabel, dryWetLabel, jitterLabel, crossFbLabel, selfModLabel, wetGainLabel, freezeTiltLabel;
    
    juce::ComboBox polarityBox;
    juce::Label polarityLabel;
    juce::ToggleButton freezeButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> macroAtt, pitchAtt, feedbackAtt, inputCutoffAtt, loopCutoffAtt, dryWetAtt, jitterAtt, crossFbAtt, selfModAtt, wetGainAtt, freezeTiltAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> polarityAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
