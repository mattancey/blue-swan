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
    
    // UI Components
    juce::Slider macroSlider, pitchSlider, feedbackSlider, inputCutoffSlider, loopCutoffSlider, resonanceSlider, dryWetSlider;
    juce::Slider jitterSlider, crossFbSlider, selfModSlider, driveAmountSlider, omegaIntensitySlider, wetGainSlider, freezeTiltSlider;
    juce::Slider lfoRateSlider, lfoPhaseSlider, lfoChaosSlider, lfoPulseSlider;
    juce::Slider m1DepthSlider, m2DepthSlider, m3DepthSlider, m4DepthSlider;
    
    juce::Label macroLabel, pitchLabel, feedbackLabel, inputCutoffLabel, loopCutoffLabel, resonanceLabel, dryWetLabel;
    juce::Label jitterLabel, crossFbLabel, selfModLabel, driveAmountLabel, omegaLabel, wetGainLabel, freezeTiltLabel;
    juce::Label lfoRateLabel, lfoPhaseLabel, lfoChaosLabel, lfoPulseLabel;
    juce::Label m1DepthLabel, m2DepthLabel, m3DepthLabel, m4DepthLabel;

    juce::ComboBox polarityBox, driveTypeBox, lfoSyncBox, lfoShapeBox;
    juce::ComboBox m1DestBox, m2DestBox, m3DestBox, m4DestBox;
    juce::Label polarityLabel, driveTypeLabel, lfoSyncLabel, lfoShapeLabel;
    juce::Label m1DestLabel, m2DestLabel, m3DestLabel, m4DestLabel;

    juce::ToggleButton freezeButton, oversampleButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> macroAtt, pitchAtt, feedbackAtt, inputCutoffAtt, loopCutoffAtt, resonanceAtt, dryWetAtt;
    std::unique_ptr<SliderAttachment> jitterAtt, crossFbAtt, selfModAtt, driveAmountAtt, omegaIntensityAtt, wetGainAtt, freezeTiltAtt;
    std::unique_ptr<SliderAttachment> lfoRateAtt, lfoPhaseAtt, lfoChaosAtt, lfoPulseAtt;
    std::unique_ptr<SliderAttachment> m1DepthAtt, m2DepthAtt, m3DepthAtt, m4DepthAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> polarityAtt, driveTypeAtt, lfoSyncAtt, lfoShapeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m1DestAtt, m2DestAtt, m3DestAtt, m4DestAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAtt, oversampleAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlueSwanAudioProcessorEditor)
};
