#include "Resonator.h"

Resonator::Resonator()
{
    envFollower.setCurrentAndTargetValue(0.0f);
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
    for (auto& ch : channels) {
        ch.delayLine.prepare(spec); ch.delayLine.reset();
        ch.inputFilter.prepare(spec); ch.inputFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); ch.inputFilter.reset();
        ch.loopFilter.prepare(spec); ch.loopFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); ch.loopFilter.reset();
        ch.tiltFilter.prepare(spec); ch.tiltFilter.setResonance(0.707f); ch.tiltFilter.reset();
        ch.dcBlocker.reset();
        ch.modFilter.setCutoff(20.0f, (float) sampleRate);
    }

    envFollower.reset(sampleRate, 1.0);
    lfo.prepare(sampleRate);
    saturation.prepare(sampleRate, samplesPerBlock, numChannels);
    
    satBuffer.setSize(numChannels, samplesPerBlock);
}

void Resonator::reset()
{
    for (auto& ch : channels) {
        ch.delayLine.reset(); ch.inputFilter.reset(); ch.loopFilter.reset(); ch.tiltFilter.reset(); ch.dcBlocker.reset(); ch.modFilter.reset();
    }
    jitterState[0] = 0.0f; jitterState[1] = 0.0f;
    saturation.reset();
}

void Resonator::setFreeze(bool shouldFreeze) { isFrozen = shouldFreeze; }

float Resonator::semitonesToDelaySamples(float semitones) const noexcept
{
    const float freq = baseFreqHz * std::pow(2.0f, semitones / 12.0f);
    return juce::jlimit(4.0f, maxDelaySamples - 8.0f, (float)(currentSampleRate / freq));
}

void Resonator::process(juce::AudioBuffer<float>& buffer, 
                        float pitch, float fb, float inCut, float loopCut, float res, float mix, 
                        float drift, float crossFB, float selfMod, bool isNeg, 
                        float driveAmt, int driveType, float omegaInt, bool oversample,
                        float wetGain, float tilt,
                        int lfoShape, float lfoRate, float lfoPhase, float lfoChaos, float lfoPulse, int lfoSyncIdx, float bpm,
                        int m1D, float m1Dep, int m2D, float m2Dep, int m3D, float m3Dep, int m4D, float m4Dep)
{
    const int numChannels = juce::jmin((int) channels.size(), buffer.getNumChannels());
    const int numSamples = buffer.getNumSamples();
    
    lfo.setShape(lfoShape); lfo.setChaos(lfoChaos); lfo.setPulseWidth(lfoPulse);
    float lfoHz = 0.0f;
    float currentLfoPhase = lfoPhase;

    if (lfoSyncIdx > 0) {
        float beatsPerSecond = bpm / 60.0f;
        float multiplier = 1.0f;
        if (lfoSyncIdx == 1) multiplier = 1.0f;
        else if (lfoSyncIdx == 2) multiplier = 1.5f;
        else if (lfoSyncIdx == 3) multiplier = 0.666f;
        else if (lfoSyncIdx == 4) multiplier = 2.0f;
        else if (lfoSyncIdx == 5) multiplier = 3.0f;
        else if (lfoSyncIdx == 6) multiplier = 1.333f;
        else if (lfoSyncIdx == 7) multiplier = 4.0f;
        else if (lfoSyncIdx == 8) multiplier = 6.0f;
        else if (lfoSyncIdx == 9) multiplier = 2.666f;
        else if (lfoSyncIdx == 10) multiplier = 8.0f;
        else if (lfoSyncIdx == 11) multiplier = 12.0f;
        else if (lfoSyncIdx == 12) multiplier = 5.333f;
        else if (lfoSyncIdx == 13) multiplier = 16.0f;
        else if (lfoSyncIdx == 14) multiplier = 24.0f;
        else if (lfoSyncIdx == 15) multiplier = 10.666f;
        lfoHz = beatsPerSecond * multiplier;
        // Le knob Rate devient un contrôle de Phase (0 à 360 deg)
        currentLfoPhase = (lfoRate / 30.0f) * 360.0f; 
    } else {
        lfoHz = lfoRate;
    }

    float lfoValue = lfo.process(lfoHz, currentLfoPhase);
    
    float m_pitch = pitch; float m_fb = fb; float m_inCut = inCut; float m_loopCut = loopCut; 
    float m_res = res; float m_mix = mix; float m_drift = drift; float m_crossFB = crossFB; 
    float m_selfMod = selfMod; float m_drive = driveAmt; float m_omega = omegaInt; float m_wetGain = wetGain; float m_tilt = tilt;

    auto applyMod = [&](int dest, float depth) {
        if (dest == 0 || depth == 0.0f) return;
        float amt = lfoValue * depth;
        if (dest == 1) m_pitch += amt * 24.0f;
        else if (dest == 2) m_fb += amt * 0.5f;
        else if (dest == 3) m_inCut += amt * 10000.0f;
        else if (dest == 4) m_loopCut += amt * 10000.0f;
        else if (dest == 5) m_res += amt * 5.0f;
        else if (dest == 6) m_mix += amt * 0.5f;
        else if (dest == 7) m_drift += amt * 0.5f;
        else if (dest == 8) m_crossFB += amt * 0.5f;
        else if (dest == 9) m_selfMod += amt * 0.5f;
        else if (dest == 10) m_drive += amt * 10.0f;
        else if (dest == 11) m_omega += amt * 0.5f;
        else if (dest == 12) m_wetGain += amt * 1.0f;
        else if (dest == 13) m_tilt += amt * 1.0f;
    };

    applyMod(m1D, m1Dep); applyMod(m2D, m2Dep); applyMod(m3D, m3Dep); applyMod(m4D, m4Dep);

    m_pitch = juce::jlimit(-36.0f, 36.0f, m_pitch);
    m_fb = juce::jlimit(0.0f, 0.99f, m_fb);
    m_inCut = juce::jlimit(20.0f, 20000.0f, m_inCut);
    m_loopCut = juce::jlimit(20.0f, 20000.0f, m_loopCut);
    m_res = juce::jlimit(0.1f, 10.0f, m_res);
    m_mix = juce::jlimit(0.0f, 1.0f, m_mix);
    m_drift = juce::jlimit(0.0f, 1.0f, m_drift);
    m_crossFB = juce::jlimit(0.0f, 0.98f, m_crossFB);
    m_selfMod = juce::jlimit(0.0f, 1.0f, m_selfMod);
    m_drive = juce::jlimit(1.0f, 20.0f, m_drive);
    m_omega = juce::jlimit(0.0f, 1.0f, m_omega);
    m_wetGain = juce::jlimit(0.0f, 2.0f, m_wetGain);
    m_tilt = juce::jlimit(-1.0f, 1.0f, m_tilt);

    // Préparation du buffer de saturation (sans allocation)
    for(int ch = 0; ch < numChannels; ++ch) {
        satBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }
    saturation.process(satBuffer, driveType, m_drive, m_omega, oversample);

    for (int ch = 0; ch < numChannels; ++ch) {
        channels[(size_t) ch].inputFilter.setCutoffFrequency(m_inCut);
        channels[(size_t) ch].loopFilter.setCutoffFrequency(m_loopCut);
        channels[(size_t) ch].loopFilter.setResonance(m_res);
    }

    std::vector<float> loopOutputs((size_t) numChannels, 0.0f);
    const float fbValue = isFrozen ? 0.99f : m_fb * (isNeg ? -1.0f : 1.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        float delaySamples = semitonesToDelaySamples(m_pitch);
        
        if (m_drift > 0.001f) {
            for (int ch = 0; ch < numChannels; ++ch) {
                jitterState[ch] += 0.001f * (jitterDist(jitterGen) - jitterState[ch]);
                delaySamples += jitterState[ch] * m_drift * (float)(currentSampleRate * 0.005);
            }
        }

        if (m_selfMod > 0.001f && numChannels > 0) {
            float modulator = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) modulator += channels[(size_t) ch].modFilter.process(loopOutputs[(size_t) ch]);
            modulator /= (float) numChannels;
            delaySamples += modulator * m_selfMod * (float)(currentSampleRate * 0.01);
        }
        
        delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);

        float drySum = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) drySum += std::abs(buffer.getReadPointer(ch)[i]);
        envFollower.setTargetValue(drySum / numChannels);
        float env = envFollower.getNextValue();
        float gateGain = env < 0.00316f ? 0.0f : 1.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            state.delayLine.setDelay(delaySamples);
            const float delayed = state.delayLine.popSample(0);
            float loopFiltered = state.loopFilter.processSample(0, delayed);
            loopFiltered = state.dcBlocker.process(loopFiltered);
            loopOutputs[(size_t) ch] = loopFiltered;
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            float dry = isFrozen ? 0.0f : satBuffer.getReadPointer(ch)[i];
            
            float crossFeedbackSignal = 0.0f;
            if (numChannels == 2) crossFeedbackSignal = loopOutputs[(size_t) (1 - ch)] * m_crossFB;

            const float inputFiltered = state.inputFilter.processSample(0, dry);
            float toWrite = inputFiltered + (loopOutputs[(size_t) ch] * fbValue) + crossFeedbackSignal;
            toWrite = std::tanh(toWrite);
            state.delayLine.pushSample(0, toWrite);

            float wetSignal = loopOutputs[(size_t) ch] + crossFeedbackSignal;
            
            // Filtre Tilt (Freeze)
            if (m_tilt > 0.01f) {
                state.tiltFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
                state.tiltFilter.setCutoffFrequency(20.0f + m_tilt * 2000.0f);
                wetSignal = state.tiltFilter.processSample(0, wetSignal);
            } else if (m_tilt < -0.01f) {
                state.tiltFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                state.tiltFilter.setCutoffFrequency(20000.0f + m_tilt * 19000.0f);
                wetSignal = state.tiltFilter.processSample(0, wetSignal);
            }

            float finalWet = wetSignal * m_wetGain * gateGain;
            buffer.getWritePointer(ch)[i] = dry * (1.0f - m_mix) + finalWet * m_mix;
        }
    }
}
