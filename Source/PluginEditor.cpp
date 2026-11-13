#include "PluginEditor.h"
#include "Params.h"
#include "UI/Widgets.h"

BlueSwanAudioProcessorEditor::BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Macro
    macroSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    macroSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(macroSlider);
    macroLabel.setText("MACRO", juce::dontSendNotification);
    macroLabel.setJustificationType(juce::Justification::centred);
    macroLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
    addAndMakeVisible(macroLabel);

    // Boutons rotatifs
    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    setupRotarySlider(feedbackSlider, feedbackLabel, "Feedback", *this);
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "In Cutoff", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cutoff", *this);
    setupRotarySlider(loopResSlider, loopResLabel, "Resonance", *this); // Nouveau
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(wetGainSlider, wetGainLabel, "Wet Gain", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "Cross FB", *this);
    setupRotarySlider(selfModSlider, selfModLabel, "Self-Mod", *this);

    // Polarity
    polarityBox.addItemList({"Positive", "Negative"}, 1);
    addAndMakeVisible(polarityBox);
    polarityLabel.setText("Polarity", juce::dontSendNotification);
    polarityLabel.setJustificationType(juce::Justification::centred);
    polarityLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(polarityLabel);

    // Attachements
    macroAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    pitchAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    feedbackAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, feedbackSlider);
    inputCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    loopCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    loopResAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopRes, loopResSlider); // Nouveau
    dryWetAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    wetGainAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::wetGain, wetGainSlider);
    jitterAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);
    crossFbAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);
    selfModAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::selfMod, selfModSlider);
    polarityAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::polarity, polarityBox);

    setSize(750, 350); // Taille conservée, on va serrer un peu la grille
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

    // Zone de gauche (Macro, Polarity)
    auto leftArea = area.removeFromLeft(150).reduced(10);
    macroLabel.setBounds(leftArea.removeFromTop(30));
    macroSlider.setBounds(leftArea.removeFromTop(120));
    
    leftArea.removeFromTop(20);
    polarityLabel.setBounds(leftArea.removeFromTop(20));
    polarityBox.setBounds(leftArea.removeFromTop(30));

    // Zone de droite (Grille)
    area.removeFromLeft(20); 
    
    // On utilise une grille de 5 colonnes et 2 rangées pour 10 boutons
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
    place(loopCutoffSlider, loopCutoffLabel, 3, 0);
    place(loopResSlider, loopResLabel, 4, 0); // Nouveau
    
    place(jitterSlider, jitterLabel, 0, 1);
    place(crossFbSlider, crossFbLabel, 1, 1);
    place(selfModSlider, selfModLabel, 2, 1);
    place(dryWetSlider, dryWetLabel, 3, 1);
    place(wetGainSlider, wetGainLabel, 4, 1);
}
