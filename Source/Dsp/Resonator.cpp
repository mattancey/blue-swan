#include "Resonator.h"

Resonator::Resonator() { 
    gateSmoothed.setCurrentAndTargetValue(0.0f); 
    lfo1Smoothed.setCurrentAndTargetValue(0.0f);
    lfo2Smoothed.setCurrentAndTargetValue(0.0f);
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
        for(int i=0; i<24; ++i) ch.inharmAllpass[i].reset();
    }
    gateSmoothed.reset(sampleRate, 2.0);
    gateSmoothed.setCurrentAndTargetValue(0.0f);
    
    lfo1.prepare(sampleRate); lfo2.prepare(sampleRate);
    lfo1Smoothed.reset(sampleRate, 0.002); 
    lfo2Smoothed.reset(sampleRate, 0.002);
    lfo1Value = 0.0f; lfo2Value = 0.0f;
}

void Resonator::reset() { 
    for (auto& ch : channels) {
        ch.delayLine.reset(); ch.pluckDelay.reset(); ch.inputFilter.reset(); ch.loopFilter.reset();
        ch.dcBlocker.reset(); ch.modFilter.reset();
        for(int i=0; i<24; ++i) ch.inharmAllpass[i].reset();
    }
    gateSmoothed.setCurrentAndTargetValue(0.0f);
    jitterState[0] = 0.0f; jitterState[1] = 0.0f;
    lfo1Value = 0.0f; lfo2Value = 0.0f;
    lfo1Smoothed.setCurrentAndTargetValue(0.0f);
    lfo2Smoothed.setCurrentAndTargetValue(0.0f);
}

void Resonator::process(juce::AudioBuffer<float>& buffer, const ModParams& p, double bpm, bool isNegative)
{
    const int numChannels = juce::jmin((int) channels.size(), buffer.getNumChannels());
    const int numSamples = buffer.getNumSamples();
    const float polarityMult = isNegative ? -1.0f : 1.0f;

    auto getLfoHz = [&](int syncIdx, float rate) {
        if (syncIdx == 0) return rate; 
        
        float beatsPerSec = (float)bpm / 60.0f;
        float beatsPerCycle = 1.0f;
        
        switch (syncIdx) {
            case 1: beatsPerCycle = 16.0f; break; 
            case 2: beatsPerCycle = 8.0f; break;  
            case 3: beatsPerCycle = 4.0f; break;  
            case 4: beatsPerCycle = 4.0f; break;  
            case 5: beatsPerCycle = 4.0f * (2.0f/3.0f); break; 
            case 6: beatsPerCycle = 4.0f * 1.5f; break; 
            case 7: beatsPerCycle = 2.0f; break; 
            case 8: beatsPerCycle = 2.0f * (2.0f/3.0f); break; 
            case 9: beatsPerCycle = 2.0f * 1.5f; break; 
            case 10: beatsPerCycle = 1.0f; break; 
            case 11: beatsPerCycle = 2.0f/3.0f; break; 
            case 12: beatsPerCycle = 1.5f; break; 
            case 13: beatsPerCycle = 0.5f; break; 
            case 14: beatsPerCycle = 0.5f * (2.0f/3.0f); break; 
            case 15: beatsPerCycle = 0.5f * 1.5f; break; 
            case 16: beatsPerCycle = 0.25f; break; 
            case 17: beatsPerCycle = 0.25f * (2.0f/3.0f); break; 
            case 18: beatsPerCycle = 0.25f * 1.5f; break; 
            case 19: beatsPerCycle = 5.0f; break; 
            case 20: beatsPerCycle = 6.0f; break; 
            case 21: beatsPerCycle = 7.0f; break; 
            case 22: beatsPerCycle = 9.0f; break; 
            case 23: beatsPerCycle = 18.0f; break; 
            default: beatsPerCycle = 1.0f; break;
        }
        return beatsPerSec / beatsPerCycle;
    };

    float lfo1HzBase = getLfoHz(p.lfo1Sync, p.lfo1Rate);
    float lfo2HzBase = getLfoHz(p.lfo2Sync, p.lfo2Rate);
    float lfo1Phase = (p.lfo1Sync > 0) ? (p.lfo1Rate / 100.0f) * 360.0f : 0.0f;
    float lfo2Phase = (p.lfo2Sync > 0) ? (p.lfo2Rate / 100.0f) * 360.0f : 0.0f;

    lfo1.setShape(p.lfo1Shape); lfo1.setPulseWidth(p.lfo1Pw);
    lfo2.setShape(p.lfo2Shape); lfo2.setPulseWidth(p.lfo2Pw);

    float loopOutputs[2] = { 0.0f, 0.0f };

    for (int i = 0; i < numSamples; ++i)
    {
        float modLfo1 = lfo2Value * p.m2_lfo1 * 50.0f;
        float modLfo2 = lfo1Value * p.m1_lfo2 * 50.0f;
        
        float lfo1Hz = juce::jlimit(0.01f, 100.0f, lfo1HzBase + modLfo1);
        float lfo2Hz = juce::jlimit(0.01f, 100.0f, lfo2HzBase + modLfo2);

        lfo1Value = lfo1.process(lfo1Hz, lfo1Phase);
        lfo2Value = lfo2.process(lfo2Hz, lfo2Phase);

        lfo1Smoothed.setTargetValue(lfo1Value);
        lfo2Smoothed.setTargetValue(lfo2Value);
        
        // Application du bypass
        float l1 = p.lfo1Bypass ? 0.0f : lfo1Smoothed.getNextValue();
        float l2 = p.lfo2Bypass ? 0.0f : lfo2Smoothed.getNextValue();

        auto mod = [&](float base, float d1, float d2, float maxVal) {
            return juce::jlimit(0.0f, maxVal, base + (l1 * d1) + (l2 * d2));
        };

        float mMacro = mod(p.macro, p.m1_macro, p.m2_macro, 1.0f);
        float macroBoost = mMacro > 0.001f ? mMacro : 0.0f;
        
        float mPitch = mod(p.pitch + (macroBoost * 24.0f), p.m1_pitch * 24.0f, p.m2_pitch * 24.0f, 36.0f);
        if (p.pitch + (l1*p.m1_pitch) + (l2*p.m2_pitch) < 0) mPitch = -mPitch; 
        
        float mFb = mod(p.feedback + (macroBoost * 0.1f), p.m1_feed, p.m2_feed, 0.99f);
        float mInc = mod(p.inputCutoff * (1.0f + macroBoost * 3.0f), p.m1_incut * 10000.0f, p.m2_incut * 10000.0f, 20000.0f);
        float mLoop = mod(p.loopCutoff * (1.0f + macroBoost * 3.0f), p.m1_loopcut * 10000.0f, p.m2_loopcut * 10000.0f, 20000.0f);
        float mHbeta = mod(p.pluck, p.m1_hbeta, p.m2_hbeta, 1.0f);
        float mInharm = mod(p.inharm, p.m1_inharm, p.m2_inharm, 0.99f);
        float mCross = mod(p.crossFb, p.m1_cross, p.m2_cross, 0.98f);
        float mSelf = mod(p.selfMod, p.m1_self, p.m2_self, 1.0f);
        float mDrift = mod(p.jitter, p.m1_drift, p.m2_drift, 1.0f);
        float mDryWet = mod(p.dryWet, p.m1_drywet, p.m2_drywet, 1.0f);
        float mWetGain = mod(p.wetGain, p.m1_wetgain, p.m2_wetgain, 2.0f);

        float delaySamples = (float)(currentSampleRate / (261.63f * std::pow(2.0f, mPitch / 12.0f)));
        delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        float pluckDelayTime = delaySamples * 0.5f;

        if (mDrift > 0.001f) {
            for (int ch = 0; ch < numChannels; ++ch) {
                float noise = jitterDist(jitterGen);
                jitterState[ch] += 0.001f * (noise - jitterState[ch]);
                delaySamples += jitterState[ch] * mDrift * (float)(currentSampleRate * 0.005);
            }
            delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        }

        if (mSelf > 0.001f) {
            float modulator = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) modulator += channels[ch].modFilter.process(loopOutputs[ch]);
            modulator /= (float)numChannels;
            delaySamples += modulator * mSelf * (float)(currentSampleRate * 0.01);
            delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        }

        float inputAbs = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) inputAbs = juce::jmax(inputAbs, std::abs(buffer.getReadPointer(ch)[i]));
        gateSmoothed.setTargetValue(inputAbs > 0.00398107f ? 1.0f : 0.0f);
        float currentGate = gateSmoothed.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch) {
            auto& state = channels[ch];
            state.inputFilter.setCutoffFrequency(mInc);
            state.loopFilter.setCutoff(mLoop, (float)currentSampleRate);
        }

        for (int ch = 0; ch < numChannels; ++ch) {
            auto& state = channels[ch];
            state.delayLine.setDelay(delaySamples);
            float delayed = state.delayLine.popSample(0);

            for (int j = 0; j < 24; ++j) {
                state.inharmAllpass[j].setCoeff(mInharm * 0.99f);
                delayed = state.inharmAllpass[j].process(delayed);
            }

            float loopFiltered = state.loopFilter.process(delayed);
            loopFiltered = state.dcBlocker.process(loopFiltered);
            loopOutputs[ch] = loopFiltered * mFb;
        }

        for (int ch = 0; ch < numChannels; ++ch) {
            auto& state = channels[ch];
            float dry = buffer.getReadPointer(ch)[i];

            state.pluckDelay.setDelay(pluckDelayTime);
            float pluckDelayed = state.pluckDelay.popSample(0);
            float mixAmt = juce::jlimit(0.0f, 1.0f, mHbeta * 1.2f);
            float pluckFiltered = dry * (1.0f - mixAmt) + (dry - pluckDelayed) * mixAmt;
            state.pluckDelay.pushSample(0, dry);

            float crossSig = (numChannels == 2) ? loopOutputs[1 - ch] * mCross : 0.0f;
            float inputFilt = state.inputFilter.processSample(0, pluckFiltered);
            
            float toWrite = std::tanh(inputFilt + (loopOutputs[ch] * polarityMult) + crossSig);
            state.delayLine.pushSample(0, toWrite);

            float wetSignal = (loopOutputs[ch] + crossSig) * mWetGain * currentGate;
            buffer.getWritePointer(ch)[i] = dry * (1.0f - mDryWet) + wetSignal * mDryWet;
        }
    }
}
