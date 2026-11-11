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
    
    juce::Slider macroSlider;
    juce::Label macroLabel;
    juce::ComboBox polarityBox;
    juce::Label polarityLabel;

    juce::Slider pitchSlider, feedbackSlider, inputCutoffSlider, loopCutoffSlider, dryWetSlider, jitterSlider, crossFbSlider, selfModSlider;
    juce::Label pitchLabel, feedbackLabel, inputCutoffLabel, loopCutoffLabel, dryWetLabel, jitterLabel, crossFbLabel, selfModLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> macroAttachment, pitchAttachment, feedbackAttachment, inputCutoffAttachment, loopCutoffAttachment, dryWetAttachment, jitterAttachment, crossFbAttachment, selfModAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> polarityAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
