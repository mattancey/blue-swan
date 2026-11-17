#include "PluginEditor.h"
#include "Params.h"
#include "UI/Widgets.h"

BlueSwanAudioProcessorEditor::BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setupRotarySlider(macroSlider, macroLabel, "MACRO", *this); macroLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "In Cut", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cut", *this);
    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    setupRotarySlider(decaySlider, decayLabel, "Feedbk", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "CrossFB", *this); crossFbSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black);
    setupRotarySlider(selfModSlider, selfModLabel, "SelfMod", *this);
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(wetGainSlider, wetGainLabel, "WetGain", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);

    polarityBox.addItemList({"Positive", "Negative"}, 1); addAndMakeVisible(polarityBox);
    polarityLabel.setText("Polarity", juce::dontSendNotification); polarityLabel.setJustificationType(juce::Justification::centred); polarityLabel.setColour(juce::Label::textColourId, juce::Colours::white); addAndMakeVisible(polarityLabel);

    macroAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    pitchAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    decayAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, decaySlider);
    inputCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    loopCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    dryWetAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    wetGainAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::wetGain, wetGainSlider);
    jitterAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);
    crossFbAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);
    selfModAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::selfMod, selfModSlider);
    polarityAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::polarity, polarityBox);

    setSize(750, 350);
    startTimerHz(10);
}

BlueSwanAudioProcessorEditor::~BlueSwanAudioProcessorEditor() { stopTimer(); }

void BlueSwanAudioProcessorEditor::timerCallback()
{
    float fb = processorRef.apvts.getRawParameterValue(Params::feedback)->load();
    float crossFb = processorRef.apvts.getRawParameterValue(Params::crossFb)->load();
    juce::Colour finalColor = juce::Colours::black;
    if (fb > 0.6f && crossFb > 0.3f) {
        if (crossFb < 0.6f) finalColor = finalColor.interpolatedWith(juce::Colours::red, (crossFb - 0.3f) / 0.3f);
        else finalColor = juce::Colours::red;
    }
    crossFbSlider.setColour(juce::Slider::rotarySliderFillColourId, finalColor);
    crossFbSlider.repaint();
}

void BlueSwanAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(20.0f)));
    g.drawFittedText("Blue Swan", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
    
    g.setColour(juce::Colours::darkgrey);
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40);
    
    auto leftArea = area.removeFromLeft(150); 
    g.drawVerticalLine((int)leftArea.getRight() + 10, (float)area.getY(), (float)area.getBottom());

    float colWidth = area.getWidth() / 4.0f;
    g.drawVerticalLine((int)(area.getX() + colWidth), (float)area.getY(), (float)area.getBottom());
    g.drawVerticalLine((int)(area.getX() + colWidth * 2), (float)area.getY(), (float)area.getBottom());
    g.drawVerticalLine((int)(area.getX() + colWidth * 3), (float)area.getY(), (float)area.getBottom());
}

void BlueSwanAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40);

    auto leftArea = area.removeFromLeft(150).reduced(10);
    macroLabel.setBounds(leftArea.removeFromTop(30)); macroSlider.setBounds(leftArea.removeFromTop(120));
    leftArea.removeFromTop(20); polarityLabel.setBounds(leftArea.removeFromTop(20)); polarityBox.setBounds(leftArea.removeFromTop(30));

    area.removeFromLeft(20); 
    const int cols = 4; const int colWidth = area.getWidth() / cols; const int rowHeight = area.getHeight() / 3;

    auto place = [&](juce::Slider& s, juce::Label& l, int col, int row) {
        juce::Rectangle<int> cell(area.getX() + col * colWidth, area.getY() + row * rowHeight, colWidth, rowHeight);
        cell = cell.reduced(10); l.setBounds(cell.removeFromTop(20)); s.setBounds(cell);
    };

    place(inputCutoffSlider, inputCutoffLabel, 0, 0); place(loopCutoffSlider, loopCutoffLabel, 0, 1); place(pitchSlider, pitchLabel, 0, 2);
    place(decaySlider, decayLabel, 1, 0);
    place(crossFbSlider, crossFbLabel, 2, 0); place(selfModSlider, selfModLabel, 2, 1);
    place(dryWetSlider, dryWetLabel, 3, 0); place(wetGainSlider, wetGainLabel, 3, 1); place(jitterSlider, jitterLabel, 3, 2);
}
