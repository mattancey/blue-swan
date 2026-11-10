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

    resonator.setPitchSemitones(apvts.getRawParameterValue(Params::pitch)->load());
    resonator.setFeedback(apvts.getRawParameterValue(Params::feedback)->load());
    resonator.setInputCutoff(apvts.getRawParameterValue(Params::inputCutoff)->load());
    resonator.setLoopCutoff(apvts.getRawParameterValue(Params::loopCutoff)->load());
    resonator.setDryWet(apvts.getRawParameterValue(Params::dryWet)->load());

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
