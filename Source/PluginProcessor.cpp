#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"

BlueSwanAudioProcessor::BlueSwanAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", Params::createLayout())
{
}

BlueSwanAudioProcessor::~BlueSwanAudioProcessor() {}

void BlueSwanAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    resonator.prepare(sampleRate, samplesPerBlock, 2);
}

void BlueSwanAudioProcessor::releaseResources()
{
    resonator.reset();
}

bool BlueSwanAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet()  != juce::AudioChannelSet::stereo()) return false;
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    return true;
}

void BlueSwanAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Récupération BPM
    double bpm = 120.0;
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            if (auto bpmOpt = pos->getBpm()) bpm = *bpmOpt;
        }
    }

    // Appel au Resonator avec TOUS les paramètres
    resonator.process(buffer,
        apvts.getRawParameterValue(Params::pitch)->load(),
        apvts.getRawParameterValue(Params::feedback)->load(),
        apvts.getRawParameterValue(Params::inputCutoff)->load(),
        apvts.getRawParameterValue(Params::loopCutoff)->load(),
        apvts.getRawParameterValue(Params::resonance)->load(),
        apvts.getRawParameterValue(Params::dryWet)->load(),
        apvts.getRawParameterValue(Params::jitter)->load(),
        apvts.getRawParameterValue(Params::crossFb)->load(),
        apvts.getRawParameterValue(Params::selfMod)->load(),
        apvts.getRawParameterValue(Params::polarity)->load() > 0.5f,
        apvts.getRawParameterValue(Params::driveAmount)->load(),
        (int)apvts.getRawParameterValue(Params::driveType)->load(),
        apvts.getRawParameterValue(Params::omegaIntensity)->load(),
        apvts.getRawParameterValue(Params::omegaOversample)->load() > 0.5f,
        apvts.getRawParameterValue(Params::wetGain)->load(),
        apvts.getRawParameterValue(Params::freezeTilt)->load(),
        (int)apvts.getRawParameterValue(Params::lfoShape)->load(),
        apvts.getRawParameterValue(Params::lfoRate)->load(), // Utilisé comme index en Sync
        apvts.getRawParameterValue(Params::lfoPhase)->load(),
        apvts.getRawParameterValue(Params::lfoChaos)->load(),
        apvts.getRawParameterValue(Params::lfoPulseWidth)->load(),
        apvts.getRawParameterValue(Params::lfoSync)->load() > 0.5f,
        (float)bpm,
        (int)apvts.getRawParameterValue(Params::m1Dest)->load(), apvts.getRawParameterValue(Params::m1Depth)->load(),
        (int)apvts.getRawParameterValue(Params::m2Dest)->load(), apvts.getRawParameterValue(Params::m2Depth)->load(),
        (int)apvts.getRawParameterValue(Params::m3Dest)->load(), apvts.getRawParameterValue(Params::m3Depth)->load(),
        (int)apvts.getRawParameterValue(Params::m4Dest)->load(), apvts.getRawParameterValue(Params::m4Depth)->load()
    );
    
    // Freeze parameter update
    resonator.setFreeze(apvts.getRawParameterValue(Params::freeze)->load() > 0.5f);
}

juce::AudioProcessorEditor* BlueSwanAudioProcessor::createEditor()
{
    return new BlueSwanAudioProcessorEditor(*this);
}

void BlueSwanAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BlueSwanAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BlueSwanAudioProcessor();
}
