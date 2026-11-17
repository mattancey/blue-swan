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
    setupRotarySlider(pluckSlider, pluckLabel, juce::String::fromUTF8("H\u03B2"), *this);
    setupRotarySlider(inharmSlider, inharmLabel, "Inharm", *this);
    setupRotarySlider(decaySlider, decayLabel, "Feedbk", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "CrossFB", *this); crossFbSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::black);
    setupRotarySlider(selfModSlider, selfModLabel, "SelfMod", *this);
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(wetGainSlider, wetGainLabel, "WetGain", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);

    polarityBox.addItemList({"Positive", "Negative"}, 1); addAndMakeVisible(polarityBox);
    polarityLabel.setText("Polarity", juce::dontSendNotification); polarityLabel.setJustificationType(juce::Justification::centred); polarityLabel.setColour(juce::Label::textColourId, juce::Colours::white); addAndMakeVisible(polarityLabel);

    drawerButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    drawerButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    drawerButton.onClick = [this]() {
        isDrawerOpen = !isDrawerOpen;
        targetHeight = isDrawerOpen ? 830.0f : 480.0f; 
    };
    addAndMakeVisible(drawerButton);

    juce::StringArray names = {"Pitch", "Feed", "InCut", "LoopC", "Hβ", "Inharm", "Cross", "Self", "Drift", "DryWet", "WetGain", "Macro", "LFO 1", "LFO 2"};
    juce::StringArray m1Ids = {Params::m1_pitch, Params::m1_feed, Params::m1_incut, Params::m1_loopcut, Params::m1_hbeta, Params::m1_inharm, Params::m1_cross, Params::m1_self, Params::m1_drift, Params::m1_drywet, Params::m1_wetgain, Params::m1_macro};
    juce::StringArray m2Ids = {Params::m2_pitch, Params::m2_feed, Params::m2_incut, Params::m2_loopcut, Params::m2_hbeta, Params::m2_inharm, Params::m2_cross, Params::m2_self, Params::m2_drift, Params::m2_drywet, Params::m2_wetgain, Params::m2_macro};

    for (int i = 0; i < 14; ++i) {
        matrixLabels[i].setText(names[i], juce::dontSendNotification);
        matrixLabels[i].setJustificationType(juce::Justification::centred);
        matrixLabels[i].setColour(juce::Label::textColourId, juce::Colours::white);
        addChildComponent(matrixLabels[i]);
    }

    auto setupMini = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addChildComponent(s);
    };

    for (int i = 0; i < 12; ++i) {
        setupMini(m1Sliders[i]); m1Atts[i] = std::make_unique<SliderAttachment>(processorRef.apvts, m1Ids[i], m1Sliders[i]);
        setupMini(m2Sliders[i]); m2Atts[i] = std::make_unique<SliderAttachment>(processorRef.apvts, m2Ids[i], m2Sliders[i]);
    }
    
    setupMini(m2Sliders[12]); m2Atts[12] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m2_lfo1, m2Sliders[12]);
    setupMini(m1Sliders[13]); m1Atts[13] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m1_lfo2, m1Sliders[13]);

    lfo1BypassButton.setButtonText("LFO\n1");
    lfo1BypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    lfo1BypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    lfo1BypassButton.setColour(juce::TextButton::textColourOnId, juce::Colours::red);
    lfo1BypassButton.setClickingTogglesState(true);
    addChildComponent(lfo1BypassButton);

    lfo2BypassButton.setButtonText("LFO\n2");
    lfo2BypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    lfo2BypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    lfo2BypassButton.setColour(juce::TextButton::textColourOnId, juce::Colours::red);
    lfo2BypassButton.setClickingTogglesState(true);
    addChildComponent(lfo2BypassButton);

    lfo1BypassAtt = std::make_unique<ButtonAttachment>(processorRef.apvts, Params::lfo1Bypass, lfo1BypassButton);
    lfo2BypassAtt = std::make_unique<ButtonAttachment>(processorRef.apvts, Params::lfo2Bypass, lfo2BypassButton);

    lfo1RowLabel.setText("LFO 1", juce::dontSendNotification); lfo1RowLabel.setJustificationType(juce::Justification::centredLeft); lfo1RowLabel.setColour(juce::Label::textColourId, juce::Colours::white); lfo1RowLabel.setFont(juce::Font(juce::FontOptions(16.0f))); addChildComponent(lfo1RowLabel);
    lfo2RowLabel.setText("LFO 2", juce::dontSendNotification); lfo2RowLabel.setJustificationType(juce::Justification::centredLeft); lfo2RowLabel.setColour(juce::Label::textColourId, juce::Colours::white); lfo2RowLabel.setFont(juce::Font(juce::FontOptions(16.0f))); addChildComponent(lfo2RowLabel);

    juce::StringArray syncs = {"Free", "4 bars", "2 bars", "1 bar", "1/1", "1/1 T", "1/1 D", "1/2", "1/2 T", "1/2 D", "1/4", "1/4 T", "1/4 D", "1/8", "1/8 T", "1/8 D", "1/16", "1/16 T", "1/16 D", "5/4", "6/4", "7/4", "9/4", "18/4"};
    juce::StringArray shapes = {"Sine", "Tri", "Saw Up", "Sqr", "Pulse", "Ramp", "Exp Rse", "S&H", "Smth Rnd", "Logistic", "Stairs", "Dbl Sine", "SOMA"};
    
    lfo1SyncBox.addItemList(syncs, 1); addChildComponent(lfo1SyncBox);
    lfo1ShapeBox.addItemList(shapes, 1); addChildComponent(lfo1ShapeBox);
    lfo2SyncBox.addItemList(syncs, 1); addChildComponent(lfo2SyncBox);
    lfo2ShapeBox.addItemList(shapes, 1); addChildComponent(lfo2ShapeBox);

    auto setupLfoKnob = [this](juce::Slider& s, juce::Label& nameLabel, juce::Label& valLabel, const juce::String& name) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addChildComponent(s);
        
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        addChildComponent(nameLabel);

        valLabel.setJustificationType(juce::Justification::centred);
        valLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        valLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
        valLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
        addChildComponent(valLabel);
    };

    setupLfoKnob(lfo1RateSlider, lfo1RateNameLabel, lfo1RateValueLabel, "Rate");
    setupLfoKnob(lfo1PhaseSlider, lfo1PhaseNameLabel, lfo1PhaseValueLabel, "Phase");
    setupLfoKnob(lfo1PwSlider, lfo1PwNameLabel, lfo1PwValueLabel, "PW");
    
    setupLfoKnob(lfo2RateSlider, lfo2RateNameLabel, lfo2RateValueLabel, "Rate");
    setupLfoKnob(lfo2PhaseSlider, lfo2PhaseNameLabel, lfo2PhaseValueLabel, "Phase");
    setupLfoKnob(lfo2PwSlider, lfo2PwNameLabel, lfo2PwValueLabel, "PW");

    lfo1SyncAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo1Sync, lfo1SyncBox);
    lfo1ShapeAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo1Shape, lfo1ShapeBox);
    lfo1RateAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo1Rate, lfo1RateSlider);
    lfo1PhaseAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo1Phase, lfo1PhaseSlider);
    lfo1PwAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo1Pw, lfo1PwSlider);
    
    lfo2SyncAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo2Sync, lfo2SyncBox);
    lfo2ShapeAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo2Shape, lfo2ShapeBox);
    lfo2RateAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo2Rate, lfo2RateSlider);
    lfo2PhaseAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo2Phase, lfo2PhaseSlider);
    lfo2PwAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo2Pw, lfo2PwSlider);

    polarityAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::polarity, polarityBox);
    mainAtts[0] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    mainAtts[1] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, decaySlider);
    mainAtts[2] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    mainAtts[3] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    mainAtts[4] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pluck, pluckSlider);
    mainAtts[5] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inharm, inharmSlider);
    mainAtts[6] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);
    mainAtts[7] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::selfMod, selfModSlider);
    mainAtts[8] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);
    mainAtts[9] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    mainAtts[10] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::wetGain, wetGainSlider);
    mainAtts[11] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);

    setSize(850, 480);
    startTimerHz(60); 
}

BlueSwanAudioProcessorEditor::~BlueSwanAudioProcessorEditor() { stopTimer(); }

void BlueSwanAudioProcessorEditor::timerCallback()
{
    if (std::abs(currentHeight - targetHeight) > 1.0f) {
        currentHeight += (targetHeight - currentHeight) * 0.15f;
        setSize(850, (int)currentHeight);
    } else if ((int)currentHeight != (int)targetHeight) {
        currentHeight = targetHeight;
        setSize(850, (int)currentHeight);
    }

    lfo1RateValueLabel.setText(juce::String(lfo1RateSlider.getValue(), 2), juce::dontSendNotification);
    lfo1PhaseValueLabel.setText(juce::String(lfo1PhaseSlider.getValue(), 0) + "°", juce::dontSendNotification);
    lfo1PwValueLabel.setText(juce::String(lfo1PwSlider.getValue(), 2), juce::dontSendNotification);
    
    lfo2RateValueLabel.setText(juce::String(lfo2RateSlider.getValue(), 2), juce::dontSendNotification);
    lfo2PhaseValueLabel.setText(juce::String(lfo2PhaseSlider.getValue(), 0) + "°", juce::dontSendNotification);
    lfo2PwValueLabel.setText(juce::String(lfo2PwSlider.getValue(), 2), juce::dontSendNotification);

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
    g.setFont(juce::Font(juce::FontOptions(24.0f)));
    g.drawFittedText("BLUE SWAN", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
    
    g.setColour(juce::Colours::darkgrey);
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40);
    if (isDrawerOpen) area.removeFromBottom(350);
    
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
    if (isDrawerOpen) area.removeFromBottom(350);
    
    area.removeFromBottom(30); 

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
    place(pluckSlider, pluckLabel, 1, 0); place(inharmSlider, inharmLabel, 1, 1);
    place(decaySlider, decayLabel, 2, 0); place(crossFbSlider, crossFbLabel, 2, 1); place(selfModSlider, selfModLabel, 2, 2);
    place(dryWetSlider, dryWetLabel, 3, 0); place(wetGainSlider, wetGainLabel, 3, 1); place(jitterSlider, jitterLabel, 3, 2);

    drawerButton.setBounds(getLocalBounds().removeFromBottom(25).reduced(0, 2));

    if (isDrawerOpen) {
        auto drawerArea = getLocalBounds().removeFromBottom(350).reduced(10); 
        drawerArea.removeFromBottom(25);
        drawerArea.removeFromTop(15); 

        auto matrixArea = drawerArea.removeFromTop(110); 
        int mCols = 14;
        
        auto labelArea = matrixArea.removeFromLeft(40);
        lfo1BypassButton.setBounds(labelArea.getX(), labelArea.getY(), 35, 55); lfo1BypassButton.setVisible(true);
        lfo2BypassButton.setBounds(labelArea.getX(), labelArea.getY() + 55, 35, 55); lfo2BypassButton.setVisible(true);

        int mWidth = matrixArea.getWidth() / mCols;
        int mRowHeight = matrixArea.getHeight() / 2;

        for (int i = 0; i < mCols; ++i) {
            matrixLabels[i].setBounds(matrixArea.getX() + i * mWidth, matrixArea.getY() - 20, mWidth, 20);
            matrixLabels[i].setVisible(true);
        }
        
        for (int i = 0; i < 12; ++i) {
            m1Sliders[i].setBounds(matrixArea.getX() + i * mWidth, matrixArea.getY(), mWidth, mRowHeight); m1Sliders[i].setVisible(true);
            m2Sliders[i].setBounds(matrixArea.getX() + i * mWidth, matrixArea.getY() + mRowHeight, mWidth, mRowHeight); m2Sliders[i].setVisible(true);
        }

        m1Sliders[12].setVisible(false); 
        m1Sliders[13].setBounds(matrixArea.getX() + 13 * mWidth, matrixArea.getY(), mWidth, mRowHeight); m1Sliders[13].setVisible(true);
        
        m2Sliders[12].setBounds(matrixArea.getX() + 12 * mWidth, matrixArea.getY() + mRowHeight, mWidth, mRowHeight); m2Sliders[12].setVisible(true);
        m2Sliders[13].setVisible(false);

        drawerArea.removeFromTop(15);

        auto lfo1Area = drawerArea.removeFromTop(90); 
        auto lfo1Row = lfo1Area.removeFromLeft(60);
        lfo1RowLabel.setBounds(lfo1Row.getX(), lfo1Row.getY() + 30, 50, 20); lfo1RowLabel.setVisible(true);
        
        lfo1SyncBox.setBounds(lfo1Area.removeFromLeft(100).reduced(0, 25)); lfo1SyncBox.setVisible(true);
        lfo1ShapeBox.setBounds(lfo1Area.removeFromLeft(100).reduced(0, 25)); lfo1ShapeBox.setVisible(true);
        
        // POTARDS AGRANDIS (130px au lieu de 90px)
        auto rate1Box = lfo1Area.removeFromLeft(130).reduced(10, 0);
        auto rate1Left = rate1Box.removeFromLeft(45);
        lfo1RateNameLabel.setBounds(rate1Left.removeFromTop(25)); lfo1RateNameLabel.setVisible(true);
        lfo1RateValueLabel.setBounds(rate1Left.removeFromTop(20).reduced(0, 2)); lfo1RateValueLabel.setVisible(true);
        lfo1RateSlider.setBounds(rate1Box); lfo1RateSlider.setVisible(true);
        
        auto phase1Box = lfo1Area.removeFromLeft(130).reduced(10, 0);
        auto phase1Left = phase1Box.removeFromLeft(45);
        lfo1PhaseNameLabel.setBounds(phase1Left.removeFromTop(25)); lfo1PhaseNameLabel.setVisible(true);
        lfo1PhaseValueLabel.setBounds(phase1Left.removeFromTop(20).reduced(0, 2)); lfo1PhaseValueLabel.setVisible(true);
        lfo1PhaseSlider.setBounds(phase1Box); lfo1PhaseSlider.setVisible(true);
        
        auto pw1Box = lfo1Area.removeFromLeft(130).reduced(10, 0);
        auto pw1Left = pw1Box.removeFromLeft(45);
        lfo1PwNameLabel.setBounds(pw1Left.removeFromTop(25)); lfo1PwNameLabel.setVisible(true);
        lfo1PwValueLabel.setBounds(pw1Left.removeFromTop(20).reduced(0, 2)); lfo1PwValueLabel.setVisible(true);
        lfo1PwSlider.setBounds(pw1Box); lfo1PwSlider.setVisible(true);

        drawerArea.removeFromTop(5);

        auto lfo2Area = drawerArea.removeFromTop(90);
        auto lfo2Row = lfo2Area.removeFromLeft(60);
        lfo2RowLabel.setBounds(lfo2Row.getX(), lfo2Row.getY() + 30, 50, 20); lfo2RowLabel.setVisible(true);
        
        lfo2SyncBox.setBounds(lfo2Area.removeFromLeft(100).reduced(0, 25)); lfo2SyncBox.setVisible(true);
        lfo2ShapeBox.setBounds(lfo2Area.removeFromLeft(100).reduced(0, 25)); lfo2ShapeBox.setVisible(true);
        
        // POTARDS AGRANDIS
        auto rate2Box = lfo2Area.removeFromLeft(130).reduced(10, 0);
        auto rate2Left = rate2Box.removeFromLeft(45);
        lfo2RateNameLabel.setBounds(rate2Left.removeFromTop(25)); lfo2RateNameLabel.setVisible(true);
        lfo2RateValueLabel.setBounds(rate2Left.removeFromTop(20).reduced(0, 2)); lfo2RateValueLabel.setVisible(true);
        lfo2RateSlider.setBounds(rate2Box); lfo2RateSlider.setVisible(true);
        
        auto phase2Box = lfo2Area.removeFromLeft(130).reduced(10, 0);
        auto phase2Left = phase2Box.removeFromLeft(45);
        lfo2PhaseNameLabel.setBounds(phase2Left.removeFromTop(25)); lfo2PhaseNameLabel.setVisible(true);
        lfo2PhaseValueLabel.setBounds(phase2Left.removeFromTop(20).reduced(0, 2)); lfo2PhaseValueLabel.setVisible(true);
        lfo2PhaseSlider.setBounds(phase2Box); lfo2PhaseSlider.setVisible(true);
        
        auto pw2Box = lfo2Area.removeFromLeft(130).reduced(10, 0);
        auto pw2Left = pw2Box.removeFromLeft(45);
        lfo2PwNameLabel.setBounds(pw2Left.removeFromTop(25)); lfo2PwNameLabel.setVisible(true);
        lfo2PwValueLabel.setBounds(pw2Left.removeFromTop(20).reduced(0, 2)); lfo2PwValueLabel.setVisible(true);
        lfo2PwSlider.setBounds(pw2Box); lfo2PwSlider.setVisible(true);

    } else {
        for (int i = 0; i < 14; ++i) matrixLabels[i].setVisible(false);
        for (int i = 0; i < 14; ++i) { m1Sliders[i].setVisible(false); m2Sliders[i].setVisible(false); }
        
        lfo1BypassButton.setVisible(false); lfo2BypassButton.setVisible(false);
        lfo1RowLabel.setVisible(false); lfo2RowLabel.setVisible(false);
        
        lfo1SyncBox.setVisible(false); lfo1ShapeBox.setVisible(false);
        lfo1RateSlider.setVisible(false); lfo1RateNameLabel.setVisible(false); lfo1RateValueLabel.setVisible(false);
        lfo1PhaseSlider.setVisible(false); lfo1PhaseNameLabel.setVisible(false); lfo1PhaseValueLabel.setVisible(false);
        lfo1PwSlider.setVisible(false); lfo1PwNameLabel.setVisible(false); lfo1PwValueLabel.setVisible(false);
        
        lfo2SyncBox.setVisible(false); lfo2ShapeBox.setVisible(false);
        lfo2RateSlider.setVisible(false); lfo2RateNameLabel.setVisible(false); lfo2RateValueLabel.setVisible(false);
        lfo2PhaseSlider.setVisible(false); lfo2PhaseNameLabel.setVisible(false); lfo2PhaseValueLabel.setVisible(false);
        lfo2PwSlider.setVisible(false); lfo2PwNameLabel.setVisible(false); lfo2PwValueLabel.setVisible(false);
    }
}
