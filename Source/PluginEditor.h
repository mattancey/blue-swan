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
    
    juce::Slider macroSlider, pitchSlider, decaySlider, pluckSlider, inharmSlider, inputCutoffSlider, loopCutoffSlider, dryWetSlider, jitterSlider, crossFbSlider, selfModSlider, wetGainSlider;
    juce::Label macroLabel, pitchLabel, decayLabel, pluckLabel, inharmLabel, inputCutoffLabel, loopCutoffLabel, dryWetLabel, jitterLabel, crossFbLabel, selfModLabel, wetGainLabel;
    
    juce::ComboBox polarityBox;
    juce::Label polarityLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> macroAtt, pitchAtt, decayAtt, pluckAtt, inharmAtt, inputCutoffAtt, loopCutoffAtt, dryWetAtt, jitterAtt, crossFbAtt, selfModAtt, wetGainAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> polarityAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
