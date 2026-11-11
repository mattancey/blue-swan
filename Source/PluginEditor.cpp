#include "PluginEditor.h"
#include "Params.h"
#include "UI/Widgets.h"

BlueSwanAudioProcessorEditor::BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Configuration du gros bouton Macro
    macroSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(macroSlider);
    macroLabel.setText("MACRO (WARP)", juce::dontSendNotification);
    macroLabel.setJustificationType(juce::Justification::centred);
    macroLabel.setColour(juce::Label::textColourId, juce::Colours::cyan); // Mise en valeur
    addAndMakeVisible(macroLabel);

    // Configuration des autres boutons
    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    setupRotarySlider(feedbackSlider, feedbackLabel, "Feedback", *this);
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "Input Cutoff", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cutoff", *this);
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Jitter", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "Cross FB", *this);

    macroAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    pitchAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    feedbackAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, feedbackSlider);
    inputCutoffAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    loopCutoffAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    dryWetAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    jitterAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);
    crossFbAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);

    setSize(800, 320); // Un peu plus large pour tout faire tenir
}

BlueSwanAudioProcessorEditor::~BlueSwanAudioProcessorEditor() = default;

void BlueSwanAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Blue Swan", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
}

void BlueSwanAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(30);

    // Zone du Macro (1/4 de la largeur)
    auto macroArea = bounds.removeFromLeft(bounds.getWidth() / 4).reduced(10);
    macroLabel.setBounds(macroArea.removeFromTop(30));
    macroSlider.setBounds(macroArea);

    // Zone des paramètres fins (3/4 restants)
    const int numKnobs = 7;
    const int w = bounds.getWidth() / numKnobs;

    auto place = [&](juce::Slider& s, juce::Label& l)
    {
        auto area = bounds.removeFromLeft(w);
        l.setBounds(area.removeFromTop(20));
        s.setBounds(area.reduced(2));
    };

    place(pitchSlider, pitchLabel);
    place(feedbackSlider, feedbackLabel);
    place(inputCutoffSlider, inputCutoffLabel);
    place(loopCutoffSlider, loopCutoffLabel);
    place(dryWetSlider, dryWetLabel);
    place(jitterSlider, jitterLabel);
    place(crossFbSlider, crossFbLabel);
}
