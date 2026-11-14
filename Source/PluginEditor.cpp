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

    eksEnableButton.setButtonText("EKS ACTIVE");
    eksEnableButton.setColour(juce::ToggleButton::textColourId, juce::Colours::cyan);
    addAndMakeVisible(eksEnableButton);

    setupRotarySlider(pluckSlider, pluckLabel, juce::String::fromUTF8("H\u03B2"), *this);
    setupRotarySlider(inharmSlider, inharmLabel, "Inharm", *this);
    setupRotarySlider(inharmJitSlider, inharmJitLabel, "Jit", *this);
    
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "In Cutoff", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cutoff", *this);
    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    
    setupRotarySlider(decaySlider, decayLabel, "Feedback", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "Cross FB", *this);
    crossFbSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black); // Base noire
    setupRotarySlider(selfModSlider, selfModLabel, "Self-Mod", *this);

    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(wetGainSlider, wetGainLabel, "Wet Gain", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);
    
    macroAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    polarityAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::polarity, polarityBox);
    eksEnableAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processorRef.apvts, Params::eksEnable, eksEnableButton);

    pluckAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pluck, pluckSlider);
    inharmAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inharm, inharmSlider);
    inharmJitAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inharmJit, inharmJitSlider);
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
    bool eksActive = processorRef.apvts.getRawParameterValue(Params::eksEnable)->load() > 0.5f;
    auto setActive = [](juce::Component& c, bool active) {
        c.setAlpha(active ? 1.0f : 0.3f);
        c.setEnabled(active);
    };

    setActive(pluckSlider, eksActive);
    setActive(pluckLabel, eksActive);
    setActive(inharmSlider, eksActive);
    setActive(inharmLabel, eksActive);
    setActive(inharmJitSlider, eksActive);
    setActive(inharmJitLabel, eksActive);

    // --- GESTION COULEUR CROSS FB ---
    float fb = processorRef.apvts.getRawParameterValue(Params::feedback)->load();
    float crossFb = processorRef.apvts.getRawParameterValue(Params::crossFb)->load();
    bool isNeg = processorRef.apvts.getRawParameterValue(Params::polarity)->load() > 0.5f;

    juce::Colour finalColor = juce::Colours::black; // Base noire

    if (!isNeg && fb > 0.6f && crossFb > 0.3f) {
        if (crossFb < 0.6f) {
            // Dégradé sur la partie supérieure : de noir vers rouge
            float normalized = (crossFb - 0.3f) / 0.3f;
            finalColor = finalColor.interpolatedWith(juce::Colours::red, normalized);
        } else {
            // Rouge vif tout en haut
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
    
    auto eksCell = juce::Rectangle<int>(area.getX() + colWidth, area.getY(), colWidth, rowHeight).reduced(10);
    eksEnableButton.setBounds(eksCell.removeFromTop(30));
    
    place(pluckSlider, pluckLabel, 1, 1);
    
    auto inharmRow = juce::Rectangle<int>(area.getX() + colWidth, area.getY() + 2 * rowHeight, colWidth, rowHeight).reduced(10);
    auto jitArea = inharmRow.removeFromRight(static_cast<int>(inharmRow.getWidth() * 0.35f));
    
    inharmJitLabel.setBounds(jitArea.removeFromTop(20));
    inharmJitSlider.setBounds(jitArea);
    
    inharmLabel.setBounds(inharmRow.removeFromTop(20));
    inharmSlider.setBounds(inharmRow);

    place(decaySlider, decayLabel, 2, 0);
    place(crossFbSlider, crossFbLabel, 2, 1);
    place(selfModSlider, selfModLabel, 2, 2);

    place(dryWetSlider, dryWetLabel, 3, 0);
    place(wetGainSlider, wetGainLabel, 3, 1);
    place(jitterSlider, jitterLabel, 3, 2);
}
