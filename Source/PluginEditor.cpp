#include "PluginEditor.h"
#include "Params.h"
#include "UI/Widgets.h"

BlueSwanAudioProcessorEditor::BlueSwanAudioProcessorEditor(BlueSwanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto setupCombo = [this](juce::ComboBox& box, juce::Label& label, const juce::String& name) {
        addAndMakeVisible(box);
        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);
    };

    setupRotarySlider(macroSlider, macroLabel, "MACRO", *this);
    setupRotarySlider(pitchSlider, pitchLabel, "Pitch", *this);
    setupRotarySlider(feedbackSlider, feedbackLabel, "Feedback", *this);
    setupRotarySlider(inputCutoffSlider, inputCutoffLabel, "In Cutoff", *this);
    setupRotarySlider(loopCutoffSlider, loopCutoffLabel, "Loop Cutoff", *this);
    setupRotarySlider(resonanceSlider, resonanceLabel, "Resonance", *this);
    setupRotarySlider(dryWetSlider, dryWetLabel, "Dry/Wet", *this);
    setupRotarySlider(jitterSlider, jitterLabel, "Drift", *this);
    setupRotarySlider(crossFbSlider, crossFbLabel, "Cross FB", *this);
    setupRotarySlider(selfModSlider, selfModLabel, "Self-Mod", *this);
    setupRotarySlider(driveAmountSlider, driveAmountLabel, "Drive", *this);
    setupRotarySlider(omegaIntensitySlider, omegaLabel, "Omega 2N", *this);
    setupRotarySlider(wetGainSlider, wetGainLabel, "Wet Gain", *this);
    setupRotarySlider(freezeTiltSlider, freezeTiltLabel, "Freeze Tilt", *this);
    setupRotarySlider(lfoRateSlider, lfoRateLabel, "LFO Rate/Phase", *this);
    setupRotarySlider(lfoPhaseSlider, lfoPhaseLabel, "LFO Phase", *this);
    setupRotarySlider(lfoChaosSlider, lfoChaosLabel, "Chaos", *this);
    setupRotarySlider(lfoPulseSlider, lfoPulseLabel, "Pulse Width", *this);

    setupCombo(polarityBox, polarityLabel, "Polarity"); polarityBox.addItemList({"Positive", "Negative"}, 1);
    setupCombo(driveTypeBox, driveTypeLabel, "Drive Type"); driveTypeBox.addItemList({"Soft", "Asym", "Fold"}, 1);
    setupCombo(lfoSyncBox, lfoSyncLabel, "LFO Sync"); lfoSyncBox.addItemList({"Free", "1/1", "1/1 T", "1/1 D", "1/2", "1/2 T", "1/2 D", "1/4", "1/4 T", "1/4 D", "1/8", "1/8 T", "1/8 D", "1/16", "1/16 T", "1/16 D"}, 1);
    setupCombo(lfoShapeBox, lfoShapeLabel, "LFO Shape"); lfoShapeBox.addItemList({"Sine", "Triangle", "Saw Up", "Saw Down", "Square", "Pulse", "Ramp", "Exp Decay", "Exp Rise", "S&H", "Smooth Rnd", "Logistic", "Staircase", "Double Sine", "SOMA"}, 1);

    juce::StringArray dests = {"None", "Pitch", "Feedback", "In Cutoff", "Loop Cutoff", "Resonance", "Dry/Wet", "Drift", "Cross FB", "Self-Mod", "Drive", "Omega 2N", "Wet Gain", "Freeze Tilt"};
    auto setupMatrix = [&](int i, juce::ComboBox& box, juce::Label& label, juce::Slider& slider, juce::Label& sLabel) {
        setupCombo(box, label, "M" + juce::String(i) + " Dest"); box.addItemList(dests, 1);
        setupRotarySlider(slider, sLabel, "Depth", *this);
    };
    setupMatrix(1, m1DestBox, m1DestLabel, m1DepthSlider, m1DepthLabel);
    setupMatrix(2, m2DestBox, m2DestLabel, m2DepthSlider, m2DepthLabel);
    setupMatrix(3, m3DestBox, m3DestLabel, m3DepthSlider, m3DepthLabel);
    setupMatrix(4, m4DestBox, m4DestLabel, m4DepthSlider, m4DepthLabel);

    freezeButton.setButtonText("FREEZE"); freezeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::orange); addAndMakeVisible(freezeButton);
    oversampleButton.setButtonText("OS 2x"); oversampleButton.setColour(juce::ToggleButton::textColourId, juce::Colours::cyan); addAndMakeVisible(oversampleButton);

    // Attachements
    macroAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::macro, macroSlider);
    pitchAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::pitch, pitchSlider);
    feedbackAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::feedback, feedbackSlider);
    inputCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::inputCutoff, inputCutoffSlider);
    loopCutoffAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::loopCutoff, loopCutoffSlider);
    resonanceAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::resonance, resonanceSlider);
    dryWetAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::dryWet, dryWetSlider);
    jitterAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::jitter, jitterSlider);
    crossFbAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::crossFb, crossFbSlider);
    selfModAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::selfMod, selfModSlider);
    driveAmountAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::driveAmount, driveAmountSlider);
    omegaIntensityAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::omegaIntensity, omegaIntensitySlider);
    wetGainAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::wetGain, wetGainSlider);
    freezeTiltAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::freezeTilt, freezeTiltSlider);
    lfoRateAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfoRate, lfoRateSlider);
    lfoPhaseAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfoPhase, lfoPhaseSlider);
    lfoChaosAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfoChaos, lfoChaosSlider);
    lfoPulseAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::lfoPulseWidth, lfoPulseSlider);
    
    polarityAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::polarity, polarityBox);
    driveTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::driveType, driveTypeBox);
    lfoSyncAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::lfoSync, lfoSyncBox);
    lfoShapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::lfoShape, lfoShapeBox);
    
    m1DestAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::m1Dest, m1DestBox);
    m1DepthAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m1Depth, m1DepthSlider);
    m2DestAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::m2Dest, m2DestBox);
    m2DepthAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m2Depth, m2DepthSlider);
    m3DestAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::m3Dest, m3DestBox);
    m3DepthAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m3Depth, m3DepthSlider);
    m4DestAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, Params::m4Dest, m4DestBox);
    m4DepthAtt = std::make_unique<SliderAttachment>(processorRef.apvts, Params::m4Depth, m4DepthSlider);

    freezeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processorRef.apvts, Params::freeze, freezeButton);
    oversampleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processorRef.apvts, Params::omegaOversample, oversampleButton);

    setSize(1100, 600);
}

BlueSwanAudioProcessorEditor::~BlueSwanAudioProcessorEditor() = default;

void BlueSwanAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("BLUE SWAN", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
}

void BlueSwanAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);
    area.removeFromTop(40);

    // MACRO SECTION (Gauche)
    auto leftArea = area.removeFromLeft(180);
    macroLabel.setBounds(leftArea.removeFromTop(30));
    macroSlider.setBounds(leftArea.removeFromTop(120));
    leftArea.removeFromTop(20);
    freezeButton.setBounds(leftArea.removeFromTop(30));
    leftArea.removeFromTop(10);
    freezeTiltLabel.setBounds(leftArea.removeFromTop(20));
    freezeTiltSlider.setBounds(leftArea.removeFromTop(80));

    // DSP CORE (Centre Haut)
    area.removeFromLeft(20);
    int w = area.getWidth() / 4;
    int h = area.getHeight() / 4;
    
    auto place = [&](juce::Slider& s, juce::Label& l, int col, int row) {
        juce::Rectangle<int> cell(area.getX() + col * w, area.getY() + row * h, w, h);
        cell = cell.reduced(5);
        l.setBounds(cell.removeFromTop(20));
        s.setBounds(cell);
    };
    auto placeCombo = [&](juce::ComboBox& c, juce::Label& l, int col, int row) {
        juce::Rectangle<int> cell(area.getX() + col * w, area.getY() + row * h, w, h);
        cell = cell.reduced(5);
        l.setBounds(cell.removeFromTop(20));
        c.setBounds(cell.removeFromTop(30));
    };

    place(pitchSlider, pitchLabel, 0, 0);
    place(feedbackSlider, feedbackLabel, 1, 0);
    placeCombo(polarityBox, polarityLabel, 2, 0);
    place(dryWetSlider, dryWetLabel, 3, 0);

    place(inputCutoffSlider, inputCutoffLabel, 0, 1);
    place(loopCutoffSlider, loopCutoffLabel, 1, 1);
    place(resonanceSlider, resonanceLabel, 2, 1);
    place(wetGainSlider, wetGainLabel, 3, 1);

    place(jitterSlider, jitterLabel, 0, 2);
    place(crossFbSlider, crossFbLabel, 1, 2);
    place(selfModSlider, selfModLabel, 2, 2);
    place(omegaIntensitySlider, omegaLabel, 3, 2);

    place(driveAmountSlider, driveAmountLabel, 0, 3);
    placeCombo(driveTypeBox, driveTypeLabel, 1, 3);
    oversampleButton.setBounds(area.getX() + 2 * w + 5, area.getY() + 3 * h + 25, w - 10, 30);

    // LFO SECTION (Bas)
    auto lfoArea = area.removeFromBottom(120);
    int lw = lfoArea.getWidth() / 10;
    auto placeLfo = [&](juce::Component& c, juce::Label& l, int col, int widthMul = 1) {
        auto cell = lfoArea.removeFromLeft(lw * widthMul).reduced(2);
        l.setBounds(cell.removeFromTop(20));
        c.setBounds(cell);
    };

    placeLfo(lfoSyncBox, lfoSyncLabel, 0, 2);
    placeLfo(lfoShapeBox, lfoShapeLabel, 0, 2);
    placeLfo(lfoRateSlider, lfoRateLabel, 0, 1);
    placeLfo(lfoPhaseSlider, lfoPhaseLabel, 0, 1);
    placeLfo(lfoChaosSlider, lfoChaosLabel, 0, 1);
    placeLfo(lfoPulseSlider, lfoPulseLabel, 0, 1);

    // Matrice 4 slots
    auto placeSlot = [&](juce::ComboBox& dBox, juce::Label& dLabel, juce::Slider& s, juce::Label& sLabel) {
        auto cell = lfoArea.removeFromLeft(lw * 1.5f).reduced(2);
        dLabel.setBounds(cell.removeFromTop(20));
        dBox.setBounds(cell.removeFromTop(25));
        sLabel.setBounds(cell.removeFromTop(15));
        s.setBounds(cell);
    };
    placeSlot(m1DestBox, m1DestLabel, m1DepthSlider, m1DepthLabel);
    placeSlot(m2DestBox, m2DestLabel, m2DepthSlider, m2DepthLabel);
    placeSlot(m3DestBox, m3DestLabel, m3DepthSlider, m3DepthLabel);
    placeSlot(m4DestBox, m4DestLabel, m4DepthSlider, m4DepthLabel);
}
