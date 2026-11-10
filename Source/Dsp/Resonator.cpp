#include "Resonator.h"

Resonator::Resonator()
{
    pitchSemitonesSmoothed.setCurrentAndTargetValue(0.0f);
    feedbackSmoothed.setCurrentAndTargetValue(0.85f);
    dryWetSmoothed.setCurrentAndTargetValue(0.5f);
}

void Resonator::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 1;

    channels.resize((size_t) numChannels);
    for (auto& ch : channels)
    {
        ch.delayLine.prepare(spec);
        ch.delayLine.reset();
        ch.inputFilter.prepare(spec);
        ch.inputFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        ch.inputFilter.reset();
        ch.loopFilter.reset();
        ch.dcBlocker.reset();
    }

    pitchSemitonesSmoothed.reset(sampleRate, 0.05);
    feedbackSmoothed.reset(sampleRate, 0.02);
    dryWetSmoothed.reset(sampleRate, 0.02);
}

void Resonator::reset()
{
    for (auto& ch : channels)
    {
        ch.delayLine.reset();
        ch.inputFilter.reset();
        ch.loopFilter.reset();
        ch.dcBlocker.reset();
    }
}

void Resonator::setPitchSemitones(float semitones) { pitchSemitonesSmoothed.setTargetValue(juce::jlimit(-36.0f, 36.0f, semitones)); }
void Resonator::setFeedback(float feedback) { feedbackSmoothed.setTargetValue(juce::jlimit(0.0f, 0.98f, feedback)); }
void Resonator::setInputCutoff(float cutoffHz) { currentInputCutoff = juce::jlimit(20.0f, 20000.0f, cutoffHz); }
void Resonator::setLoopCutoff(float cutoffHz) { currentLoopCutoff = juce::jlimit(20.0f, 20000.0f, cutoffHz); }
void Resonator::setDryWet(float dryWet) { dryWetSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, dryWet)); }

float Resonator::semitonesToDelaySamples(float semitones) const noexcept
{
    const float freq = baseFreqHz * std::pow(2.0f, semitones / 12.0f);
    const float delay = (float) (currentSampleRate / freq);
    return juce::jlimit(4.0f, maxDelaySamples - 8.0f, delay);
}

void Resonator::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = juce::jmin((int) channels.size(), buffer.getNumChannels());
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        channels[(size_t) ch].inputFilter.setCutoffFrequency(currentInputCutoff);
        channels[(size_t) ch].loopFilter.setCutoff(currentLoopCutoff, (float) currentSampleRate);
    }

    for (int i = 0; i < numSamples; ++i)
    {
        const float pitchSt = pitchSemitonesSmoothed.getNextValue();
        const float fb = feedbackSmoothed.getNextValue();
        const float wetMix = dryWetSmoothed.getNextValue();
        const float delaySamples = semitonesToDelaySamples(pitchSt);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            const float dry = buffer.getReadPointer(ch)[i];

            state.delayLine.setDelay(delaySamples);
            const float delayed = state.delayLine.popSample(0);

            float loopFiltered = state.loopFilter.process(delayed);
            loopFiltered = state.dcBlocker.process(loopFiltered);

            const float inputFiltered = state.inputFilter.processSample(0, dry);
            float toWrite = inputFiltered + loopFiltered * fb;
            toWrite = std::tanh(toWrite);

            state.delayLine.pushSample(0, toWrite);
            buffer.getWritePointer(ch)[i] = dry * (1.0f - wetMix) + loopFiltered * wetMix;
        }
    }
}
