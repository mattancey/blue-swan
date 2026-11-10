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
    juce::Slider pitchSlider, feedbackSlider, inputCutoffSlider, loopCutoffSlider, dryWetSlider;
    juce::Label pitchLabel, feedbackLabel, inputCutoffLabel, loopCutoffLabel, dryWetLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment, feedbackAttachment, inputCutoffAttachment, loopCutoffAttachment, dryWetAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
