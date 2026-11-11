#include "PluginEditor.h"
#include "Params.h"
#include "UI/Widgets.h"

BlueSwanAudioProcessorEditor::BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    macroSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(macroSlider);
    macroLabel.setText("MACRO (WARP)", juce::dontSendNotification);
    macroLabel.setJustificationType(juce::Justification::centred);
    macroLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
    addAndMakeVisible(macroLabel);

    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    setupRotarySlider(feedbackSlider, feedbackLabel, "Feedback", *this);
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "In Cutoff", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cutoff", *this);
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "Cross FB", *this);
    setupRotarySlider(selfModSlider, selfModLabel, "Self-Mod", *this);

    polarityBox.addItemList({"Positive", "Negative"}, 1);
    addAndMakeVisible(polarityBox);
    polarityLabel.setText("Polarity", juce::dontSendNotification);
    polarityLabel.setJustificationType(juce::Justification::centred);
    polarityLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(polarityLabel);

    macroAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    pitchAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    feedbackAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, feedbackSlider);
    inputCutoffAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    loopCutoffAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    dryWetAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    jitterAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);
    crossFbAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);
    selfModAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::selfMod, selfModSlider);
    polarityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::polarity, polarityBox);

    setSize(850, 400);
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

    // Macro en haut à gauche
    auto macroArea = area.removeFromLeft(200).reduced(10);
    macroLabel.setBounds(macroArea.removeFromTop(30));
    macroSlider.setBounds(macroArea.removeFromTop(120));

    // Polarity en dessous du Macro
    macroArea.removeFromTop(20);
    polarityLabel.setBounds(macroArea.removeFromTop(20));
    polarityBox.setBounds(macroArea.removeFromTop(30));

    // Le reste des boutons
    area.removeFromLeft(20); // spacer
    const int w = area.getWidth() / 4;
    const int h = area.getHeight() / 2;

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
    place(loopCutoffSlider, loopCutoffLabel, 3, 0);
    
    place(dryWetSlider, dryWetLabel, 0, 1);
    place(jitterSlider, jitterLabel, 1, 1);
    place(crossFbSlider, crossFbLabel, 2, 1);
    place(selfModSlider, selfModLabel, 3, 1);
}
