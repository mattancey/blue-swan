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

    polarityBox.addItemList({"Positive", "Negative"}, 1);
    addAndMakeVisible(polarityBox);
    polarityLabel.setText("Polarity", juce::dontSendNotification);
    polarityLabel.setJustificationType(juce::Justification::centred);
    polarityLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(polarityLabel);

    setupRotarySlider(pluckSlider, pluckLabel, juce::String::fromUTF8("H\u03B2"), *this);
    setupRotarySlider(inharmSlider, inharmLabel, "Inharm", *this);
    
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "In Cutoff", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cutoff", *this);
    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    
    setupRotarySlider(decaySlider, decayLabel, "Feedback", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "Cross FB", *this);
    crossFbSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black);
    setupRotarySlider(selfModSlider, selfModLabel, "Self-Mod", *this);

    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(wetGainSlider, wetGainLabel, "Wet Gain", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);
    
    macroAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    polarityAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::polarity, polarityBox);

    pluckAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pluck, pluckSlider);
    inharmAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inharm, inharmSlider);
    inputCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    loopCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    pitchAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    decayAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, decaySlider);
    crossFbAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);
    selfModAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::selfMod, selfModSlider);
    dryWetAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    wetGainAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::wetGain, wetGainSlider);
    jitterAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);

    setSize(850, 480);
    startTimerHz(10);
}

BlueSwanAudioProcessorEditor::~BlueSwanAudioProcessorEditor() 
{
    stopTimer();
}

void BlueSwanAudioProcessorEditor::timerCallback()
{
    float fb = processorRef.apvts.getRawParameterValue(Params::feedback)->load();
    float crossFb = processorRef.apvts.getRawParameterValue(Params::crossFb)->load();

    juce::Colour finalColor = juce::Colours::black;

    if (fb > 0.6f && crossFb > 0.3f) {
        if (crossFb < 0.6f) {
            float normalized = (crossFb - 0.3f) / 0.3f;
            finalColor = finalColor.interpolatedWith(juce::Colours::red, normalized);
        } else {
            finalColor = juce::Colours::red;
        }
    }

    crossFbSlider.setColour(juce::Slider::rotarySliderFillColourId, finalColor);
    crossFbSlider.repaint();
}

void BlueSwanAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("BLUE SWAN", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
    
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
    
    macroLabel.setBounds(leftArea.removeFromTop(30));
    macroSlider.setBounds(leftArea.removeFromTop(120));
    
    leftArea.removeFromTop(20);
    polarityLabel.setBounds(leftArea.removeFromTop(20));
    polarityBox.setBounds(leftArea.removeFromTop(30));

    area.removeFromLeft(20); 
    
    const int cols = 4;
    const int colWidth = area.getWidth() / cols;
    const int rowHeight = area.getHeight() / 3;

    auto place = [&](juce::Slider& s, juce::Label& l, int col, int row) {
        juce::Rectangle<int> cell(area.getX() + col * colWidth, area.getY() + row * rowHeight, colWidth, rowHeight);
        cell = cell.reduced(10);
        l.setBounds(cell.removeFromTop(20));
        s.setBounds(cell);
    };

    place(inputCutoffSlider, inputCutoffLabel, 0, 0);
    place(loopCutoffSlider, loopCutoffLabel, 0, 1);
    place(pitchSlider, pitchLabel, 0, 2);
    
    place(pluckSlider, pluckLabel, 1, 0);
    place(inharmSlider, inharmLabel, 1, 1);

    place(decaySlider, decayLabel, 2, 0);
    place(crossFbSlider, crossFbLabel, 2, 1);
    place(selfModSlider, selfModLabel, 2, 2);

    place(dryWetSlider, dryWetLabel, 3, 0);
    place(wetGainSlider, wetGainLabel, 3, 1);
    place(jitterSlider, jitterLabel, 3, 2);
}
