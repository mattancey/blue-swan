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
    
    inharmSmoothed.setCurrentAndTargetValue(0.0f);
    pluckSmoothed.setCurrentAndTargetValue(0.0f);
    
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
        ch.pluckDelay.prepare(spec);
        ch.pluckDelay.reset();
        ch.inputFilter.prepare(spec);
        ch.inputFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        ch.inputFilter.reset();
        ch.loopFilter.reset();
        ch.dcBlocker.reset();
        ch.modFilter.setCutoff(20.0f, (float) sampleRate);
        for (int i = 0; i < 24; ++i) ch.inharmAllpass[i].reset();
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
    
    // Lissage sur 50ms pour les boutons EKS
    inharmSmoothed.reset(sampleRate, 0.05);
    pluckSmoothed.reset(sampleRate, 0.05);
}

void Resonator::reset()
{
    for (auto& ch : channels)
    {
        ch.delayLine.reset();
        ch.pluckDelay.reset();
        ch.inputFilter.reset();
        ch.loopFilter.reset();
        ch.dcBlocker.reset();
        ch.modFilter.reset();
        for (int i = 0; i < 24; ++i) ch.inharmAllpass[i].reset();
    }
    jitterState[0] = 0.0f;
    jitterState[1] = 0.0f;
    gateSmoothed.setCurrentAndTargetValue(0.0f);
    inharmSmoothed.setCurrentAndTargetValue(0.0f);
    pluckSmoothed.setCurrentAndTargetValue(0.0f);
}

void Resonator::process(juce::AudioBuffer<float>& buffer, const ModParams& p, double bpm, bool isNegative)
{
    const int numChannels = juce::jmin((int) channels.size(), buffer.getNumChannels());
    const int numSamples = buffer.getNumSamples();
    polarityMultiplier = isNegative ? -1.0f : 1.0f;

    // Mise à jour des lissages
    pitchSemitonesSmoothed.setTargetValue(juce::jlimit(-36.0f, 36.0f, p.pitch));
    feedbackSmoothed.setTargetValue(juce::jlimit(0.0f, 0.98f, p.feedback));
    dryWetSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, p.dryWet));
    jitterSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, p.jitter));
    crossFbSmoothed.setTargetValue(juce::jlimit(0.0f, 0.98f, p.crossFb));
    selfModSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, p.selfMod));
    wetGainSmoothed.setTargetValue(juce::jlimit(0.0f, 2.0f, p.wetGain));
    
    inharmSmoothed.setTargetValue(p.inharm);
    pluckSmoothed.setTargetValue(p.pluck);

    currentInputCutoff = juce::jlimit(20.0f, 20000.0f, p.inputCutoff);
    currentLoopCutoff = juce::jlimit(20.0f, 20000.0f, p.loopCutoff);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        channels[(size_t) ch].inputFilter.setCutoffFrequency(currentInputCutoff);
        channels[(size_t) ch].loopFilter.setCutoff(currentLoopCutoff, (float) currentSampleRate);
    }

    float loopOutputs[2] = { 0.0f, 0.0f };

    for (int i = 0; i < numSamples; ++i)
    {
        const float pitchSt = pitchSemitonesSmoothed.getNextValue();
        const float fb = feedbackSmoothed.getNextValue() * polarityMultiplier;
        const float wetMix = dryWetSmoothed.getNextValue();
        const float drift = jitterSmoothed.getNextValue();
        const float crossFB = crossFbSmoothed.getNextValue();
        const float selfModAmt = selfModSmoothed.getNextValue();
        const float currentWetGain = wetGainSmoothed.getNextValue();
        
        const float inharm = inharmSmoothed.getNextValue();
        const float pluck = pluckSmoothed.getNextValue();
        
        float delaySamples = semitonesToDelaySamples(pitchSt);
        
        // Compensation de l'Inharm
        float a = juce::jlimit(-0.99f, 0.99f, inharm * 0.99f);
        float groupDelayPerFilter = (1.0f + a) / (1.0f - a + 0.0001f);
        float totalGroupDelay = 24.0f * groupDelayPerFilter;
        delaySamples -= totalGroupDelay;

        if (drift > 0.001f) {
            for (int ch = 0; ch < numChannels; ++ch) {
                float noise = jitterDist(jitterGen);
                jitterState[ch] += 0.001f * (noise - jitterState[ch]);
                delaySamples += jitterState[ch] * drift * (float)(currentSampleRate * 0.005);
            }
        }

        if (selfModAmt > 0.001f && numChannels > 0) {
            float modulator = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                modulator += channels[(size_t) ch].modFilter.process(loopOutputs[ch]);
            }
            modulator /= (float) numChannels;
            delaySamples += modulator * selfModAmt * (float)(currentSampleRate * 0.01);
        }
        
        // Clamp de sécurité après tous les calculs de retard
        delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        float pluckDelayTime = delaySamples * 0.5f;

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
            float delayed = state.delayLine.popSample(0);

            // On laisse tourner les 24 allpass en permanence
            for (int j = 0; j < 24; ++j) {
                state.inharmAllpass[j].setCoeff(a);
                delayed = state.inharmAllpass[j].process(delayed);
            }

            float loopFiltered = state.loopFilter.process(delayed);
            loopFiltered = state.dcBlocker.process(loopFiltered);
            loopOutputs[ch] = loopFiltered;
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            float dry = buffer.getReadPointer(ch)[i];
            
            float crossFeedbackSignal = 0.0f;
            if (numChannels == 2) {
                int otherCh = 1 - ch;
                crossFeedbackSignal = loopOutputs[otherCh] * crossFB;
            }

            // On laisse tourner le Pluck en permanence
            state.pluckDelay.setDelay(pluckDelayTime);
            float pluckDelayed = state.pluckDelay.popSample(0);
            float mixAmt = juce::jlimit(0.0f, 1.0f, pluck * 1.2f);
            float pluckFiltered = dry * (1.0f - mixAmt) + (dry - pluckDelayed) * mixAmt;
            pluckFiltered *= (1.0f + (pluck * 0.5f)); // Makeup Gain
            
            state.pluckDelay.pushSample(0, dry);

            const float inputFiltered = state.inputFilter.processSample(0, pluckFiltered);
            float toWrite = inputFiltered + (loopOutputs[ch] * fb) + crossFeedbackSignal;
            toWrite = std::tanh(toWrite);

            state.delayLine.pushSample(0, toWrite);
            
            float wetSignal = loopOutputs[ch] + crossFeedbackSignal;
            wetSignal *= currentWetGain * currentGate;
            
            buffer.getWritePointer(ch)[i] = dry * (1.0f - wetMix) + wetSignal * wetMix;
        }
    }
}

float Resonator::semitonesToDelaySamples(float semitones) const noexcept
{
    const float freq = baseFreqHz * std::pow(2.0f, semitones / 12.0f);
    const float delay = (float) (currentSampleRate / freq);
    return juce::jlimit(4.0f, maxDelaySamples - 8.0f, delay);
}
