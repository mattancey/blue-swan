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
        targetHeight = isDrawerOpen ? 850.0f : 480.0f;
    };
    addAndMakeVisible(drawerButton);

    // Noms des 14 colonnes pour les titres
    juce::StringArray names = {"Pitch", "Feed", "InCut", "LoopC", "Hβ", "Inharm", "Cross", "Self", "Drift", "DryWet", "WetGain", "Macro", "LFO 1", "LFO 2"};
    
    // Attachements pour les 12 paramètres normaux
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

    // On crée 12 sliders pour la ligne 1
    for (int i = 0; i < 12; ++i) {
        setupMini(m1Sliders[i]);
        m1Atts[i] = std::make_unique<SliderAttachment>(processorRef.apvts, m1Ids[i], m1Sliders[i]);
    }
    // 13ème slider Ligne 1 = LFO 1 module LFO 2
    setupMini(m1Sliders[12]); // Non utilisé mais évite l'index out of bounds si mal utilisé plus tard
    m1Atts[12] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m1_lfo2, m1Sliders[12]);
    
    // On crée 12 sliders pour la ligne 2
    for (int i = 0; i < 12; ++i) {
        setupMini(m2Sliders[i]);
        m2Atts[i] = std::make_unique<SliderAttachment>(processorRef.apvts, m2Ids[i], m2Sliders[i]);
    }
    // 13ème slider Ligne 2 = LFO 2 module LFO 1
    setupMini(m2Sliders[12]);
    m2Atts[12] = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m2_lfo1, m2Sliders[12]);

    lfo1NameLabel.setText("LFO 1", juce::dontSendNotification); 
    lfo1NameLabel.setJustificationType(juce::Justification::centred); 
    lfo1NameLabel.setColour(juce::Label::textColourId, juce::Colours::white); 
    lfo1NameLabel.setFont(juce::Font(juce::FontOptions(16.0f))); 
    addChildComponent(lfo1NameLabel);
    
    lfo2NameLabel.setText("LFO 2", juce::dontSendNotification); 
    lfo2NameLabel.setJustificationType(juce::Justification::centred); 
    lfo2NameLabel.setColour(juce::Label::textColourId, juce::Colours::white); 
    lfo2NameLabel.setFont(juce::Font(juce::FontOptions(16.0f))); 
    addChildComponent(lfo2NameLabel);

    juce::StringArray syncs = {"Free", "1/1", "1/1 T", "1/1 D", "1/2", "1/2 T", "1/2 D", "1/4", "1/4 T", "1/4 D", "1/8", "1/8 T", "1/8 D", "1/16", "1/16 T", "1/16 D"};
    juce::StringArray shapes = {"Sine", "Tri", "Saw Up", "Saw Dn", "Sqr", "Pulse", "Ramp", "Exp Dec", "Exp Rse", "S&H", "Smth Rnd", "Logistic", "Stairs", "Dbl Sine", "SOMA"};
    
    lfo1SyncBox.addItemList(syncs, 1); addChildComponent(lfo1SyncBox);
    lfo1ShapeBox.addItemList(shapes, 1); addChildComponent(lfo1ShapeBox);
    lfo2SyncBox.addItemList(syncs, 1); addChildComponent(lfo2SyncBox);
    lfo2ShapeBox.addItemList(shapes, 1); addChildComponent(lfo2ShapeBox);

    setupRotarySlider(lfo1RateSlider, lfo1RateLabel, "Rate/Ph", *this); addChildComponent(lfo1RateSlider); addChildComponent(lfo1RateLabel);
    setupRotarySlider(lfo1ChaosSlider, lfo1ChaosLabel, "Chaos", *this); addChildComponent(lfo1ChaosSlider); addChildComponent(lfo1ChaosLabel);
    setupRotarySlider(lfo1PwSlider, lfo1PwLabel, "PW", *this); addChildComponent(lfo1PwSlider); addChildComponent(lfo1PwLabel);
    setupRotarySlider(lfo2RateSlider, lfo2RateLabel, "Rate/Ph", *this); addChildComponent(lfo2RateSlider); addChildComponent(lfo2RateLabel);
    setupRotarySlider(lfo2ChaosSlider, lfo2ChaosLabel, "Chaos", *this); addChildComponent(lfo2ChaosSlider); addChildComponent(lfo2ChaosLabel);
    setupRotarySlider(lfo2PwSlider, lfo2PwLabel, "PW", *this); addChildComponent(lfo2PwSlider); addChildComponent(lfo2PwLabel);

    lfo1SyncAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo1Sync, lfo1SyncBox);
    lfo1ShapeAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo1Shape, lfo1ShapeBox);
    lfo1RateAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo1Rate, lfo1RateSlider);
    lfo1ChaosAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo1Chaos, lfo1ChaosSlider);
    lfo1PwAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo1Pw, lfo1PwSlider);
    lfo2SyncAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo2Sync, lfo2SyncBox);
    lfo2ShapeAtt = std::make_unique<ComboAttachment>(processorRef.apvts, Params::lfo2Shape, lfo2ShapeBox);
    lfo2RateAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo2Rate, lfo2RateSlider);
    lfo2ChaosAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfo2Chaos, lfo2ChaosSlider);
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
    if (isDrawerOpen) area.removeFromBottom(370);
    
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
    if (isDrawerOpen) area.removeFromBottom(370);

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
        auto drawerArea = getLocalBounds().removeFromBottom(370).reduced(10);
        drawerArea.removeFromBottom(25);

        drawerArea.removeFromTop(15); // 5mm de marge en haut

        auto matrixArea = drawerArea.removeFromTop(110); 
        int mCols = 14;
        
        // Noms des LFO à gauche
        auto labelArea = matrixArea.removeFromLeft(60);
        lfo1NameLabel.setBounds(labelArea.getX(), labelArea.getY(), 50, 55); lfo1NameLabel.setVisible(true);
        lfo2NameLabel.setBounds(labelArea.getX(), labelArea.getY() + 55, 50, 55); lfo2NameLabel.setVisible(true);

        int mWidth = matrixArea.getWidth() / mCols;
        int mRowHeight = matrixArea.getHeight() / 2;

        for (int i = 0; i < mCols; ++i) {
            matrixLabels[i].setBounds(matrixArea.getX() + i * mWidth, matrixArea.getY() - 20, mWidth, 20);
            matrixLabels[i].setVisible(true);
        }
        
        for (int i = 0; i < 12; ++i) {
            m1Sliders[i].setBounds(matrixArea.getX() + i * mWidth, matrixArea.getY(), mWidth, mRowHeight);
            m1Sliders[i].setVisible(true);
            
            m2Sliders[i].setBounds(matrixArea.getX() + i * mWidth, matrixArea.getY() + mRowHeight, mWidth, mRowHeight);
            m2Sliders[i].setVisible(true);
        }

        // Placements des sliders croisés
        // Ligne 1 : LFO 1 (col 12) n'existe pas, LFO 2 (col 13) est m1_lfo2
        m1Sliders[12].setBounds(matrixArea.getX() + 13 * mWidth, matrixArea.getY(), mWidth, mRowHeight); 
        m1Sliders[12].setVisible(true);
        
        // Ligne 2 : LFO 1 (col 12) est m2_lfo1, LFO 2 (col 13) n'existe pas
        m2Sliders[12].setBounds(matrixArea.getX() + 12 * mWidth, matrixArea.getY() + mRowHeight, mWidth, mRowHeight); 
        m2Sliders[12].setVisible(true);

        drawerArea.removeFromTop(20);

        // LFO 1 (Horizontal, réparti)
        auto lfo1Area = drawerArea.removeFromTop(70).reduced(10, 0);
        auto placeLfoKnob = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int> a) {
            auto cell = a.reduced(5); l.setBounds(cell.removeFromTop(15)); l.setVisible(true); s.setBounds(cell); s.setVisible(true);
        };
        
        int lfoW = lfo1Area.getWidth() / 5;
        lfo1SyncBox.setBounds(lfo1Area.removeFromLeft(lfoW).reduced(5)); lfo1SyncBox.setVisible(true);
        lfo1ShapeBox.setBounds(lfo1Area.removeFromLeft(lfoW).reduced(5)); lfo1ShapeBox.setVisible(true);
        placeLfoKnob(lfo1RateSlider, lfo1RateLabel, lfo1Area.removeFromLeft(lfoW));
        placeLfoKnob(lfo1ChaosSlider, lfo1ChaosLabel, lfo1Area.removeFromLeft(lfoW));
        placeLfoKnob(lfo1PwSlider, lfo1PwLabel, lfo1Area.removeFromLeft(lfoW));

        // LFO 2 (Horizontal)
        auto lfo2Area = drawerArea.removeFromTop(70).reduced(10, 0);
        int lfoW2 = lfo2Area.getWidth() / 5;
        lfo2SyncBox.setBounds(lfo2Area.removeFromLeft(lfoW2).reduced(5)); lfo2SyncBox.setVisible(true);
        lfo2ShapeBox.setBounds(lfo2Area.removeFromLeft(lfoW2).reduced(5)); lfo2ShapeBox.setVisible(true);
        placeLfoKnob(lfo2RateSlider, lfo2RateLabel, lfo2Area.removeFromLeft(lfoW2));
        placeLfoKnob(lfo2ChaosSlider, lfo2ChaosLabel, lfo2Area.removeFromLeft(lfoW2));
        placeLfoKnob(lfo2PwSlider, lfo2PwLabel, lfo2Area.removeFromLeft(lfoW2));

    } else {
        for (int i = 0; i < 14; ++i) matrixLabels[i].setVisible(false);
        for (int i = 0; i < 12; ++i) { m1Sliders[i].setVisible(false); m2Sliders[i].setVisible(false); }
        m1Sliders[12].setVisible(false); m2Sliders[12].setVisible(false);
        
        lfo1NameLabel.setVisible(false); lfo2NameLabel.setVisible(false);
        lfo1SyncBox.setVisible(false); lfo1ShapeBox.setVisible(false); lfo1RateSlider.setVisible(false); lfo1ChaosSlider.setVisible(false); lfo1PwSlider.setVisible(false);
        lfo1RateLabel.setVisible(false); lfo1ChaosLabel.setVisible(false); lfo1PwLabel.setVisible(false);
        lfo2SyncBox.setVisible(false); lfo2ShapeBox.setVisible(false); lfo2RateSlider.setVisible(false); lfo2ChaosSlider.setVisible(false); lfo2PwSlider.setVisible(false);
        lfo2RateLabel.setVisible(false); lfo2ChaosLabel.setVisible(false); lfo2PwLabel.setVisible(false);
    }
}
