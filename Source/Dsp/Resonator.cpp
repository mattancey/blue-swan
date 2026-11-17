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
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 1 };

    channels.resize((size_t) numChannels);
    for (auto& ch : channels) {
        ch.delayLine.prepare(spec); ch.delayLine.reset();
        ch.pluckDelay.prepare(spec); ch.pluckDelay.reset();
        ch.inputFilter.prepare(spec); ch.inputFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); ch.inputFilter.reset();
        ch.loopFilter.reset(); ch.dcBlocker.reset();
        ch.modFilter.setCutoff(20.0f, (float) sampleRate);
        for (int i = 0; i < 24; ++i) ch.inharmAllpass[i].reset();
    }

    lfo1.prepare(sampleRate); lfo2.prepare(sampleRate);

    pitchSemitonesSmoothed.reset(sampleRate, 0.05);
    feedbackSmoothed.reset(sampleRate, 0.02);
    dryWetSmoothed.reset(sampleRate, 0.02);
    jitterSmoothed.reset(sampleRate, 0.1);
    crossFbSmoothed.reset(sampleRate, 0.05);
    selfModSmoothed.reset(sampleRate, 0.05);
    wetGainSmoothed.reset(sampleRate, 0.02);
    gateSmoothed.reset(sampleRate, 2.0);
    gateSmoothed.setCurrentAndTargetValue(0.0f);
    inharmSmoothed.reset(sampleRate, 0.05);
    pluckSmoothed.reset(sampleRate, 0.05);
}

void Resonator::reset()
{
    for (auto& ch : channels) {
        ch.delayLine.reset(); ch.pluckDelay.reset(); ch.inputFilter.reset(); ch.loopFilter.reset();
        ch.dcBlocker.reset(); ch.modFilter.reset();
        for (int i = 0; i < 24; ++i) ch.inharmAllpass[i].reset();
    }
    jitterState[0] = 0.0f; jitterState[1] = 0.0f;
    gateSmoothed.setCurrentAndTargetValue(0.0f);
    inharmSmoothed.setCurrentAndTargetValue(0.0f);
    pluckSmoothed.setCurrentAndTargetValue(0.0f);
}

void Resonator::process(juce::AudioBuffer<float>& buffer, const ModParams& p, double bpm, bool isNegative)
{
    const int numChannels = juce::jmin((int) channels.size(), buffer.getNumChannels());
    const int numSamples = buffer.getNumSamples();
    polarityMultiplier = isNegative ? -1.0f : 1.0f;

    auto getLfoHz = [&](int syncIdx, float rate) {
        if (syncIdx == 0) return rate; 
        float beatsPerSec = (float)bpm / 60.0f;
        float beatsPerCycle = 1.0f;
        switch (syncIdx) {
            case 1: beatsPerCycle = 16.0f; break; case 2: beatsPerCycle = 8.0f; break; case 3: beatsPerCycle = 4.0f; break;  
            case 4: beatsPerCycle = 4.0f; break; case 5: beatsPerCycle = 4.0f * (2.0f/3.0f); break; case 6: beatsPerCycle = 4.0f * 1.5f; break; 
            case 7: beatsPerCycle = 2.0f; break; case 8: beatsPerCycle = 2.0f * (2.0f/3.0f); break; case 9: beatsPerCycle = 2.0f * 1.5f; break; 
            case 10: beatsPerCycle = 1.0f; break; case 11: beatsPerCycle = 2.0f/3.0f; break; case 12: beatsPerCycle = 1.5f; break; 
            case 13: beatsPerCycle = 0.5f; break; case 14: beatsPerCycle = 0.5f * (2.0f/3.0f); break; case 15: beatsPerCycle = 0.5f * 1.5f; break; 
            case 16: beatsPerCycle = 0.25f; break; case 17: beatsPerCycle = 0.25f * (2.0f/3.0f); break; case 18: beatsPerCycle = 0.25f * 1.5f; break; 
            case 19: beatsPerCycle = 5.0f; break; case 20: beatsPerCycle = 6.0f; break; case 21: beatsPerCycle = 7.0f; break; 
            case 22: beatsPerCycle = 9.0f; break; case 23: beatsPerCycle = 18.0f; break; default: beatsPerCycle = 1.0f; break;
        }
        return beatsPerSec / beatsPerCycle;
    };

    float lfo1Hz = getLfoHz(p.lfo1Sync, p.lfo1Rate);
    float lfo2Hz = getLfoHz(p.lfo2Sync, p.lfo2Rate);

    lfo1.setShape(p.lfo1Shape); lfo1.setPulseWidth(p.lfo1Pw);
    lfo2.setShape(p.lfo2Shape); lfo2.setPulseWidth(p.lfo2Pw);

    float l1 = p.lfo1Bypass ? 0.0f : lfo1.processBlock(lfo1Hz, numSamples, p.lfo1Phase);
    float l2 = p.lfo2Bypass ? 0.0f : lfo2.processBlock(lfo2Hz, numSamples, p.lfo2Phase);

    auto mod = [&](float base, float d1, float d2, float minVal, float maxVal) {
        return juce::jlimit(minVal, maxVal, base + (l1 * d1) + (l2 * d2));
    };

    float mMacro = mod(p.macro, p.m1_macro, p.m2_macro, 0.0f, 1.0f);
    float macroBoost = mMacro > 0.001f ? mMacro : 0.0f;
    
    float targetPitch = mod(p.pitch + (macroBoost * 24.0f), p.m1_pitch * 24.0f, p.m2_pitch * 24.0f, -36.0f, 36.0f);
    float targetFb = mod(p.feedback + (macroBoost * 0.1f), p.m1_feed, p.m2_feed, 0.0f, 0.98f); // Aligné sur V0.4
    float targetInCut = mod(p.inputCutoff * (1.0f + macroBoost * 3.0f), p.m1_incut * 10000.0f, p.m2_incut * 10000.0f, 20.0f, 20000.0f);
    float targetLoopCut = mod(p.loopCutoff * (1.0f + macroBoost * 3.0f), p.m1_loopcut * 10000.0f, p.m2_loopcut * 10000.0f, 20.0f, 20000.0f);
    float targetHbeta = mod(p.pluck, p.m1_hbeta, p.m2_hbeta, 0.0f, 1.0f);
    float targetInharm = mod(p.inharm, p.m1_inharm, p.m2_inharm, 0.0f, 0.99f);
    float targetCross = mod(p.crossFb, p.m1_cross, p.m2_cross, 0.0f, 0.98f);
    float targetSelf = mod(p.selfMod, p.m1_self, p.m2_self, 0.0f, 1.0f);
    float targetDrift = mod(p.jitter, p.m1_drift, p.m2_drift, 0.0f, 1.0f);
    float targetDryWet = mod(p.dryWet, p.m1_drywet, p.m2_drywet, 0.0f, 1.0f);
    float targetWetGain = mod(p.wetGain, p.m1_wetgain, p.m2_wetgain, 0.0f, 2.0f);

    pitchSemitonesSmoothed.setTargetValue(targetPitch);
    feedbackSmoothed.setTargetValue(targetFb);
    dryWetSmoothed.setTargetValue(targetDryWet);
    jitterSmoothed.setTargetValue(targetDrift);
    crossFbSmoothed.setTargetValue(targetCross);
    selfModSmoothed.setTargetValue(targetSelf);
    wetGainSmoothed.setTargetValue(targetWetGain);
    inharmSmoothed.setTargetValue(targetInharm);
    pluckSmoothed.setTargetValue(targetHbeta);

    currentInputCutoff = targetInCut;
    currentLoopCutoff = targetLoopCut;

    for (int ch = 0; ch < numChannels; ++ch) {
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
        
        // BYPASS ABSOLU DE L'INHARM SI A ZERO
        if (inharm > 0.001f) {
            float a = juce::jlimit(-0.99f, 0.99f, inharm * 0.99f);
            float groupDelayPerFilter = (1.0f + a) / (1.0f - a + 0.0001f);
            float totalGroupDelay = 24.0f * groupDelayPerFilter;
            delaySamples -= totalGroupDelay;
        }

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

            // BYPASS ABSOLU DES FILTRES SI INHARM EST A ZERO
            if (inharm > 0.001f) {
                float a = juce::jlimit(-0.99f, 0.99f, inharm * 0.99f);
                for (int j = 0; j < 24; ++j) {
                    state.inharmAllpass[j].setCoeff(a);
                    delayed = state.inharmAllpass[j].process(delayed);
                }
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

            float pluckFiltered = dry;
            // BYPASS ABSOLU DU PLUCK SI A ZERO
            if (pluck > 0.001f) {
                state.pluckDelay.setDelay(pluckDelayTime);
                float pluckDelayed = state.pluckDelay.popSample(0);
                float mixAmt = juce::jlimit(0.0f, 1.0f, pluck * 1.2f);
                pluckFiltered = dry * (1.0f - mixAmt) + (dry - pluckDelayed) * mixAmt;
                pluckFiltered *= (1.0f + (pluck * 0.5f)); 
                state.pluckDelay.pushSample(0, dry);
            }

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
