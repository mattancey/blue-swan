#include "PluginEditor.h"
#include "Params.h"
#include "UI/Widgets.h"

BlueSwanAudioProcessorEditor::BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    setupRotarySlider(feedbackSlider, feedbackLabel, "Feedback", *this);
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "Input Cutoff", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cutoff", *this);
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);

    pitchAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    feedbackAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, feedbackSlider);
    inputCutoffAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    loopCutoffAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    dryWetAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);

    setSize(600, 280);
}

BlueSwanAudioProcessorEditor::~BlueSwanAudioProcessorEditor() = default;

void BlueSwanAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("Blue Swan", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
}

void BlueSwanAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(30);
    const int w = bounds.getWidth() / 5;

    auto place = [&](juce::Slider& s, juce::Label& l)
    {
        auto area = bounds.removeFromLeft(w);
        l.setBounds(area.removeFromTop(20));
        s.setBounds(area.reduced(5));
    };

    place(pitchSlider, pitchLabel);
    place(feedbackSlider, feedbackLabel);
    place(inputCutoffSlider, inputCutoffLabel);
    place(loopCutoffSlider, loopCutoffLabel);
    place(dryWetSlider, dryWetLabel);
}
