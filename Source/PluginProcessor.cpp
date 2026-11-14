#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"

BlueSwanAudioProcessor::BlueSwanAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", Params::createLayout())
{}

BlueSwanAudioProcessor::~BlueSwanAudioProcessor() {}

void BlueSwanAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) { resonator.prepare(sampleRate, samplesPerBlock, 2); }
void BlueSwanAudioProcessor::releaseResources() { resonator.reset(); }

bool BlueSwanAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return (layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void BlueSwanAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) buffer.clear(i, 0, buffer.getNumSamples());

    double bpm = 120.0;
    if (auto* playHead = getPlayHead()) if (auto pos = playHead->getPosition()) if (auto bpmOpt = pos->getBpm()) bpm = *bpmOpt;

    ModParams p{}; // Initialisation à zéro pour éviter les NaN
    auto get = [&](const juce::String& id) { return apvts.getRawParameterValue(id)->load(); };

    p.pitch = get(Params::pitch); p.feedback = get(Params::feedback); p.pluck = get(Params::pluck); p.inharm = get(Params::inharm);
    p.inputCutoff = get(Params::inputCutoff); p.loopCutoff = get(Params::loopCutoff); p.dryWet = get(Params::dryWet);
    p.jitter = get(Params::jitter); p.crossFb = get(Params::crossFb); p.selfMod = get(Params::selfMod); p.wetGain = get(Params::wetGain);
    p.macro = get(Params::macro);

    p.lfo1Shape = (int)get(Params::lfo1Shape); p.lfo1Sync = (int)get(Params::lfo1Sync); p.lfo1Rate = get(Params::lfo1Rate); p.lfo1Chaos = get(Params::lfo1Chaos); p.lfo1Pw = get(Params::lfo1Pw);
    p.lfo2Shape = (int)get(Params::lfo2Shape); p.lfo2Sync = (int)get(Params::lfo2Sync); p.lfo2Rate = get(Params::lfo2Rate); p.lfo2Chaos = get(Params::lfo2Chaos); p.lfo2Pw = get(Params::lfo2Pw);

    p.m1_pitch = get(Params::m1_pitch); p.m2_pitch = get(Params::m2_pitch);
    p.m1_feed = get(Params::m1_feed); p.m2_feed = get(Params::m2_feed);
    p.m1_incut = get(Params::m1_incut); p.m2_incut = get(Params::m2_incut);
    p.m1_loopcut = get(Params::m1_loopcut); p.m2_loopcut = get(Params::m2_loopcut);
    p.m1_hbeta = get(Params::m1_hbeta); p.m2_hbeta = get(Params::m2_hbeta);
    p.m1_inharm = get(Params::m1_inharm); p.m2_inharm = get(Params::m2_inharm);
    p.m1_cross = get(Params::m1_cross); p.m2_cross = get(Params::m2_cross);
    p.m1_self = get(Params::m1_self); p.m2_self = get(Params::m2_self);
    p.m1_drift = get(Params::m1_drift); p.m2_drift = get(Params::m2_drift);
    p.m1_drywet = get(Params::m1_drywet); p.m2_drywet = get(Params::m2_drywet);
    p.m1_wetgain = get(Params::m1_wetgain); p.m2_wetgain = get(Params::m2_wetgain);
    p.m1_macro = get(Params::m1_macro); p.m2_macro = get(Params::m2_macro);
    
    // Ajout des 4 paramètres manquants qui faisaient crasher Ableton
    p.m1_lfo1 = get(Params::m1_lfo1); p.m2_lfo1 = get(Params::m2_lfo1);
    p.m1_lfo2 = get(Params::m1_lfo2); p.m2_lfo2 = get(Params::m2_lfo2);

    bool isNeg = get(Params::polarity) > 0.5f;
    resonator.process(buffer, p, bpm, isNeg);
}

juce::AudioProcessorEditor* BlueSwanAudioProcessor::createEditor() { return new BlueSwanAudioProcessorEditor(*this); }
void BlueSwanAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml, destData);
}
void BlueSwanAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BlueSwanAudioProcessor(); }
