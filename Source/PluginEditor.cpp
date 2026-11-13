#include "PluginEditor.h"
#include "Params.h"
#include "UI/Widgets.h"

BlueSwanAudioProcessorEditor::BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    macroSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(macroSlider);
    macroLabel.setText("MACRO", juce::dontSendNotification);
    macroLabel.setJustificationType(juce::Justification::centred);
    macroLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
    addAndMakeVisible(macroLabel);

    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    setupRotarySlider(feedbackSlider, feedbackLabel, "Feedback", *this);
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "In Cutoff", *this);
    setupRotarySlider(buchla292Slider, buchla292Label, "Buchla 292", *this);
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(wetGainSlider, wetGainLabel, "Wet Gain", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "Cross FB", *this);
    setupRotarySlider(selfModSlider, selfModLabel, "Self-Mod", *this);

    polarityBox.addItemList({"Positive", "Negative"}, 1);
    addAndMakeVisible(polarityBox);
    polarityLabel.setText("Polarity", juce::dontSendNotification);
    polarityLabel.setJustificationType(juce::Justification::centred);
    polarityLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(polarityLabel);

    lpgModeBox.addItemList({"VCF", "VCA", "Both"}, 1);
    addAndMakeVisible(lpgModeBox);
    lpgModeLabel.setText("LPG Mode", juce::dontSendNotification);
    lpgModeLabel.setJustificationType(juce::Justification::centred);
    lpgModeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(lpgModeLabel);

    lpgDeepButton.setButtonText("Deep");
    lpgDeepButton.setColour(juce::ToggleButton::textColourId, juce::Colours::orange);
    addAndMakeVisible(lpgDeepButton);

    macroAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    pitchAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    feedbackAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, feedbackSlider);
    inputCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    buchla292Att = std::make_unique<SliderAttachment>(processorRef.apvts, Params::buchla292, buchla292Slider);
    dryWetAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    wetGainAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::wetGain, wetGainSlider);
    jitterAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);
    crossFbAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);
    selfModAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::selfMod, selfModSlider);
    polarityAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::polarity, polarityBox);
    lpgModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::lpgMode, lpgModeBox);
    lpgDeepAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processorRef.apvts, Params::lpgDeep, lpgDeepButton);

    setSize(800, 400);
}

BlueSwanAudioProcessorEditor::~BlueSwanAudioProcessorEditor() = default;

void BlueSwanAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("Blue Swan", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
}

void BlueSwanAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40);

    auto leftArea = area.removeFromLeft(180).reduced(10);
    macroLabel.setBounds(leftArea.removeFromTop(30));
    macroSlider.setBounds(leftArea.removeFromTop(120));
    
    leftArea.removeFromTop(20);
    polarityLabel.setBounds(leftArea.removeFromTop(20));
    polarityBox.setBounds(leftArea.removeFromTop(30));
    
    leftArea.removeFromTop(10);
    lpgModeLabel.setBounds(leftArea.removeFromTop(20));
    lpgModeBox.setBounds(leftArea.removeFromTop(30));
    
    leftArea.removeFromTop(10);
    lpgDeepButton.setBounds(leftArea.removeFromTop(30));

    area.removeFromLeft(20); 
    
    const int cols = 5;
    const int rows = 2;
    const int w = area.getWidth() / cols;
    const int h = area.getHeight() / rows;

    auto place = [&](juce::Slider& s, juce::Label& l, int col, int row)
    {
        juce::Rectangle<int> cell(area.getX() + col * w, area.getY() + row * h, w, h);
        cell = cell.reduced(5);
        l.setBounds(cell.removeFromTop(20));
        s.setBounds(cell);
    };

    place(pitchSlider, pitchLabel, 0, 0);
    place(feedbackSlider, feedbackLabel, 1, 0);
    place(inputCutoffSlider, inputCutoffLabel, 2, 0);
    place(buchla292Slider, buchla292Label, 3, 0);
    place(dryWetSlider, dryWetLabel, 4, 0);
    
    place(jitterSlider, jitterLabel, 0, 1);
    place(crossFbSlider, crossFbLabel, 1, 1);
    place(selfModSlider, selfModLabel, 2, 1);
    place(wetGainSlider, wetGainLabel, 3, 1);
}
