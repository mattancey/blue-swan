#include "Resonator.h"

Resonator::Resonator()
{
    pitchSemitonesSmoothed.setCurrentAndTargetValue(0.0f);
    feedbackSmoothed.setCurrentAndTargetValue(0.85f);
    dryWetSmoothed.setCurrentAndTargetValue(0.5f);
    jitterSmoothed.setCurrentAndTargetValue(0.0f);
    crossFbSmoothed.setCurrentAndTargetValue(0.0f);
    selfModSmoothed.setCurrentAndTargetValue(0.0f);
    wetGainSmoothed.setCurrentAndTargetValue(1.0f);
    gateSmoothed.setCurrentAndTargetValue(0.0f);
    freezeTiltSmoothed.setCurrentAndTargetValue(0.0f);
    jitterGen.seed(std::random_device{}());
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
        ch.modFilter.setCutoff(20.0f, (float) sampleRate);
        ch.tiltFilter.reset();
    }

    pitchSemitonesSmoothed.reset(sampleRate, 0.05);
    feedbackSmoothed.reset(sampleRate, 0.02);
    dryWetSmoothed.reset(sampleRate, 0.02);
    jitterSmoothed.reset(sampleRate, 0.1);
    crossFbSmoothed.reset(sampleRate, 0.05);
    selfModSmoothed.reset(sampleRate, 0.05);
    wetGainSmoothed.reset(sampleRate, 0.02);
    gateSmoothed.reset(sampleRate, 2.0);
    gateSmoothed.setCurrentAndTargetValue(0.0f);
    freezeTiltSmoothed.reset(sampleRate, 0.05);
}

void Resonator::reset()
{
    for (auto& ch : channels)
    {
        ch.delayLine.reset();
        ch.inputFilter.reset();
        ch.loopFilter.reset();
        ch.dcBlocker.reset();
        ch.modFilter.reset();
        ch.tiltFilter.reset();
    }
    jitterState[0] = 0.0f;
    jitterState[1] = 0.0f;
    gateSmoothed.setCurrentAndTargetValue(0.0f);
}

void Resonator::setPitchSemitones(float semitones) { pitchSemitonesSmoothed.setTargetValue(juce::jlimit(-36.0f, 36.0f, semitones)); }
void Resonator::setFeedback(float feedback) { feedbackSmoothed.setTargetValue(juce::jlimit(0.0f, 0.98f, feedback)); }
void Resonator::setInputCutoff(float cutoffHz) { currentInputCutoff = juce::jlimit(20.0f, 20000.0f, cutoffHz); }
void Resonator::setLoopCutoff(float cutoffHz) { currentLoopCutoff = juce::jlimit(20.0f, 20000.0f, cutoffHz); }
void Resonator::setDryWet(float dryWet) { dryWetSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, dryWet)); }
void Resonator::setJitter(float jitterAmount) { jitterSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, jitterAmount)); }
void Resonator::setCrossFeedback(float crossAmt) { crossFbSmoothed.setTargetValue(juce::jlimit(0.0f, 0.98f, crossAmt)); }
void Resonator::setSelfMod(float selfModAmount) { selfModSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, selfModAmount)); }
void Resonator::setPolarity(bool isNegative) { polarityMultiplier = isNegative ? -1.0f : 1.0f; }
void Resonator::setWetGain(float gain) { wetGainSmoothed.setTargetValue(juce::jlimit(0.0f, 2.0f, gain)); }
void Resonator::setFreeze(bool shouldFreeze) { isFrozen = shouldFreeze; }
void Resonator::setFreezeTilt(float tilt) { freezeTiltSmoothed.setTargetValue(juce::jlimit(-1.0f, 1.0f, tilt)); }

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

    float loopOutputs[2] = { 0.0f, 0.0f };

    for (int i = 0; i < numSamples; ++i)
    {
        const float pitchSt = pitchSemitonesSmoothed.getNextValue();
        const float fb = isFrozen ? 0.98f : feedbackSmoothed.getNextValue() * polarityMultiplier;
        const float wetMix = dryWetSmoothed.getNextValue();
        const float drift = jitterSmoothed.getNextValue();
        const float crossFB = crossFbSmoothed.getNextValue();
        const float selfModAmt = selfModSmoothed.getNextValue();
        const float currentWetGain = wetGainSmoothed.getNextValue();
        const float currentTilt = freezeTiltSmoothed.getNextValue();
        
        float delaySamples = semitonesToDelaySamples(pitchSt);
        
        if (drift > 0.001f) {
            for (int ch = 0; ch < numChannels; ++ch) {
                float noise = jitterDist(jitterGen);
                jitterState[ch] += 0.001f * (noise - jitterState[ch]);
                delaySamples += jitterState[ch] * drift * (float)(currentSampleRate * 0.005);
            }
            delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        }

        if (selfModAmt > 0.001f && numChannels > 0) {
            float modulator = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                modulator += channels[(size_t) ch].modFilter.process(loopOutputs[ch]);
            }
            modulator /= (float) numChannels;
            delaySamples += modulator * selfModAmt * (float)(currentSampleRate * 0.01);
            delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        }

        float inputAbs = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            inputAbs = juce::jmax(inputAbs, std::abs(buffer.getReadPointer(ch)[i]));
        }
        
        if (inputAbs > 0.00398107f) {
            gateSmoothed.setCurrentAndTargetValue(1.0f);
        } else {
            gateSmoothed.setTargetValue(0.0f);
        }
        float currentGate = gateSmoothed.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            state.delayLine.setDelay(delaySamples);
            const float delayed = state.delayLine.popSample(0);

            float loopFiltered = state.loopFilter.process(delayed);
            loopFiltered = state.dcBlocker.process(loopFiltered);
            loopOutputs[ch] = loopFiltered;
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            
            // Séparation du signal pour le mix Dry et l'injection dans la boucle
            const float outputDry = buffer.getReadPointer(ch)[i];
            
            // INJECTION FANTÔME : Si freeze, on atténue l'entrée à -30dB (environ 0.0316)
            const float inputToLoop = isFrozen ? outputDry * 0.0316f : outputDry;
            
            float crossFeedbackSignal = 0.0f;
            if (numChannels == 2) {
                int otherCh = 1 - ch;
                crossFeedbackSignal = loopOutputs[otherCh] * crossFB;
            }

            const float inputFiltered = state.inputFilter.processSample(0, inputToLoop);
            float toWrite = inputFiltered + (loopOutputs[ch] * fb) + crossFeedbackSignal;
            toWrite = std::tanh(toWrite);

            state.delayLine.pushSample(0, toWrite);
            
            float wetSignal = loopOutputs[ch] + crossFeedbackSignal;
            
            // --- FILTRE TILT (Freeze) ---
            if (currentTilt > 0.001f) {
                float cutoff = 20.0f * std::pow(100.0f, currentTilt);
                state.tiltFilter.setCutoff(cutoff, (float)currentSampleRate);
                float lp = state.tiltFilter.process(wetSignal);
                wetSignal = wetSignal - lp; // HPF
            } else if (currentTilt < -0.001f) {
                float cutoff = 20000.0f * std::pow(0.01f, -currentTilt);
                state.tiltFilter.setCutoff(cutoff, (float)currentSampleRate);
                wetSignal = state.tiltFilter.process(wetSignal); // LPF
            }
            
            wetSignal *= currentWetGain * currentGate;
            
            // Le mix utilise le outputDry non atténué
            buffer.getWritePointer(ch)[i] = outputDry * (1.0f - wetMix) + wetSignal * wetMix;
        }
    }
}
