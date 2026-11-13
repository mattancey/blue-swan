#include "Resonator.h"

Resonator::Resonator()
{
    pitchSemitonesSmoothed.setCurrentAndTargetValue(0.0f);
    decaySmoothed.setCurrentAndTargetValue(0.85f);
    pluckSmoothed.setCurrentAndTargetValue(0.0f);
    inharmSmoothed.setCurrentAndTargetValue(0.0f);
    dryWetSmoothed.setCurrentAndTargetValue(0.5f);
    jitterSmoothed.setCurrentAndTargetValue(0.0f);
    crossFbSmoothed.setCurrentAndTargetValue(0.0f);
    selfModSmoothed.setCurrentAndTargetValue(0.0f);
    wetGainSmoothed.setCurrentAndTargetValue(1.0f);
    gateSmoothed.setCurrentAndTargetValue(0.0f);
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
        ch.pluckDelay.setDelay(2.0f); // Délai fixe de 2 ms pour le peigne
        
        ch.inputFilter.prepare(spec);
        ch.inputFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        ch.inputFilter.reset();
        
        ch.loopFilter.reset();
        ch.dcBlocker.reset();
        ch.modFilter.setCutoff(20.0f, (float) sampleRate);
        ch.inharmAllpass.reset();
    }

    pitchSemitonesSmoothed.reset(sampleRate, 0.05);
    decaySmoothed.reset(sampleRate, 0.05);
    pluckSmoothed.reset(sampleRate, 0.05);
    inharmSmoothed.reset(sampleRate, 0.05);
    dryWetSmoothed.reset(sampleRate, 0.02);
    jitterSmoothed.reset(sampleRate, 0.1);
    crossFbSmoothed.reset(sampleRate, 0.05);
    selfModSmoothed.reset(sampleRate, 0.05);
    wetGainSmoothed.reset(sampleRate, 0.02);
    gateSmoothed.reset(sampleRate, 2.0);
    gateSmoothed.setCurrentAndTargetValue(0.0f);
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
        ch.inharmAllpass.reset();
    }
    jitterState[0] = 0.0f;
    jitterState[1] = 0.0f;
    gateSmoothed.setCurrentAndTargetValue(0.0f);
}

void Resonator::setPitchSemitones(float semitones) { pitchSemitonesSmoothed.setTargetValue(juce::jlimit(-36.0f, 36.0f, semitones)); }
void Resonator::setDecay(float decay) { decaySmoothed.setTargetValue(juce::jlimit(0.0f, 0.99f, decay)); }
void Resonator::setPluck(float pluck) { pluckSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, pluck)); }
void Resonator::setInharm(float inharm) { inharmSmoothed.setTargetValue(juce::jlimit(0.0f, 0.99f, inharm)); }
void Resonator::setInputCutoff(float cutoffHz) { currentInputCutoff = juce::jlimit(20.0f, 20000.0f, cutoffHz); }
void Resonator::setLoopCutoff(float cutoffHz) { currentLoopCutoff = juce::jlimit(20.0f, 20000.0f, cutoffHz); }
void Resonator::setDryWet(float dryWet) { dryWetSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, dryWet)); }
void Resonator::setJitter(float jitterAmount) { jitterSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, jitterAmount)); }
void Resonator::setCrossFeedback(float crossAmt) { crossFbSmoothed.setTargetValue(juce::jlimit(0.0f, 0.98f, crossAmt)); }
void Resonator::setSelfMod(float selfModAmount) { selfModSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, selfModAmount)); }
void Resonator::setPolarity(bool isNegative) { polarityMultiplier = isNegative ? -1.0f : 1.0f; }
void Resonator::setWetGain(float gain) { wetGainSmoothed.setTargetValue(juce::jlimit(0.0f, 2.0f, gain)); }

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
        const float decay = decaySmoothed.getNextValue();
        const float pluck = pluckSmoothed.getNextValue();
        const float inharm = inharmSmoothed.getNextValue();
        
        const float wetMix = dryWetSmoothed.getNextValue();
        const float drift = jitterSmoothed.getNextValue();
        const float crossFB = crossFbSmoothed.getNextValue();
        const float selfModAmt = selfModSmoothed.getNextValue();
        const float currentWetGain = wetGainSmoothed.getNextValue();
        
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

        // --- Lecture de la boucle ---
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            state.delayLine.setDelay(delaySamples);
            float delayed = state.delayLine.popSample(0);

            // 1. Inharmonicité (Allpass de raideur)
            float a = inharm * 0.9f;
            state.inharmAllpass.setCoeff(a);
            delayed = state.inharmAllpass.process(delayed);

            // 2. Filtre de boucle
            float loopFiltered = state.loopFilter.process(delayed);
            loopFiltered = state.dcBlocker.process(loopFiltered);
            
            // 3. Application du Decay (Facteur de perte)
            loopFiltered *= decay;
            
            loopOutputs[ch] = loopFiltered;
        }

        // --- Écriture dans la boucle ---
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            float dry = buffer.getReadPointer(ch)[i];

            // 4. Pluck (Filtre en peigne Feed-Forward)
            float pluckDelayed = state.pluckDelay.popSample(0);
            float pluckFiltered = dry - (pluck * pluckDelayed);
            state.pluckDelay.pushSample(0, dry);

            float crossFeedbackSignal = 0.0f;
            if (numChannels == 2) {
                int otherCh = 1 - ch;
                crossFeedbackSignal = loopOutputs[otherCh] * crossFB;
            }

            const float inputFiltered = state.inputFilter.processSample(0, pluckFiltered);
            float toWrite = inputFiltered + (loopOutputs[ch] * polarityMultiplier) + crossFeedbackSignal;
            toWrite = std::tanh(toWrite);

            state.delayLine.pushSample(0, toWrite);
            
            float wetSignal = loopOutputs[ch] + crossFeedbackSignal;
            wetSignal *= currentWetGain * currentGate;
            
            buffer.getWritePointer(ch)[i] = dry * (1.0f - wetMix) + wetSignal * wetMix;
        }
    }
}
