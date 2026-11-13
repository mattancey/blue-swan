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

    float basePitch = apvts.getRawParameterValue(Params::pitch)->load();
    float baseFeedback = apvts.getRawParameterValue(Params::feedback)->load();
    float baseInputCutoff = apvts.getRawParameterValue(Params::inputCutoff)->load();
    float baseLoopCutoff = apvts.getRawParameterValue(Params::loopCutoff)->load();
    float dryWet = apvts.getRawParameterValue(Params::dryWet)->load();
    float jitter = apvts.getRawParameterValue(Params::jitter)->load();
    float crossFb = apvts.getRawParameterValue(Params::crossFb)->load();
    float selfMod = apvts.getRawParameterValue(Params::selfMod)->load();
    float wetGain = apvts.getRawParameterValue(Params::wetGain)->load();
    bool isNegativePolarity = apvts.getRawParameterValue(Params::polarity)->load() > 0.5f;
    float freezeTilt = apvts.getRawParameterValue(Params::freezeTilt)->load();
    
    float macroValue = apvts.getRawParameterValue(Params::macro)->load();
    if (macroValue > 0.001f) {
        basePitch += macroValue * 24.0f;
        baseInputCutoff *= (1.0f + macroValue * 3.0f);
        baseLoopCutoff *= (1.0f + macroValue * 3.0f);
        baseFeedback = juce::jmin(0.98f, baseFeedback + (macroValue * 0.1f));
    }

    resonator.setPitchSemitones(basePitch);
    resonator.setFeedback(baseFeedback);
    resonator.setInputCutoff(juce::jmin(20000.0f, baseInputCutoff));
    resonator.setLoopCutoff(juce::jmin(20000.0f, baseLoopCutoff));
    resonator.setDryWet(dryWet);
    resonator.setJitter(jitter);
    resonator.setCrossFeedback(crossFb);
    resonator.setSelfMod(selfMod);
    resonator.setPolarity(isNegativePolarity);
    resonator.setWetGain(wetGain);
    
    resonator.setFreeze(apvts.getRawParameterValue(Params::freeze)->load() > 0.5f);
    resonator.setFreezeTilt(freezeTilt);

    resonator.process(buffer);
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
