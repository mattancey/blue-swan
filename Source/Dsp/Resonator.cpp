#include "Resonator.h"

Resonator::Resonator()
{
    pitchSemitonesSmoothed.setCurrentAndTargetValue(0.0f);
    feedbackSmoothed.setCurrentAndTargetValue(0.85f);
    dryWetSmoothed.setCurrentAndTargetValue(0.5f);
    jitterSmoothed.setCurrentAndTargetValue(0.0f);
    crossFbSmoothed.setCurrentAndTargetValue(0.0f);
    selfModSmoothed.setCurrentAndTargetValue(0.0f);
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
        ch.modFilter.setCutoff(20.0f, (float) sampleRate); // Filtre très bas pour le self-mod
    }

    pitchSemitonesSmoothed.reset(sampleRate, 0.05);
    feedbackSmoothed.reset(sampleRate, 0.02);
    dryWetSmoothed.reset(sampleRate, 0.02);
    jitterSmoothed.reset(sampleRate, 0.1); // Lissage plus lent pour le drift
    crossFbSmoothed.reset(sampleRate, 0.05);
    selfModSmoothed.reset(sampleRate, 0.05);
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
    }
    jitterState[0] = 0.0f;
    jitterState[1] = 0.0f;
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

    // Buffers pour stocker les sorties de la boucle (pour le cross-feedback et le self-mod)
    std::vector<float> loopOutputs((size_t) numChannels, 0.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        const float pitchSt = pitchSemitonesSmoothed.getNextValue();
        const float fb = feedbackSmoothed.getNextValue() * polarityMultiplier; // POLARITÉ ICI
        const float wetMix = dryWetSmoothed.getNextValue();
        const float drift = jitterSmoothed.getNextValue();
        const float crossFB = crossFbSmoothed.getNextValue();
        const float selfModAmt = selfModSmoothed.getNextValue();
        
        float delaySamples = semitonesToDelaySamples(pitchSt);
        
        // --- DRIFT (ex-Jitter) : modulation lente et lissée du temps de retard ---
        if (drift > 0.001f) {
            for (int ch = 0; ch < numChannels; ++ch) {
                // On génère un bruit aléatoire qu'on filtre très fortement pour n'avoir qu'une dérive lente
                float noise = jitterDist(jitterGen);
                jitterState[ch] += 0.001f * (noise - jitterState[ch]); // Lissage extrême
                // La profondeur du drift est proportionnelle à la valeur du knob (max ~5ms)
                delaySamples += jitterState[ch] * drift * (currentSampleRate * 0.005f);
            }
            delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        }

        // --- SELF-MOD (SOMA Lyra-8) : la sortie module le temps de retard ---
        if (selfModAmt > 0.001f && numChannels > 0) {
            // On utilise la somme des sorties filtrées (ou un canal spécifique) comme modulateur
            float modulator = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                modulator += channels[(size_t) ch].modFilter.process(loopOutputs[(size_t) ch]);
            }
            modulator /= (float) numChannels;
            // La modulation peut être positive ou négative, créant un effet de "warble" extrême
            delaySamples += modulator * selfModAmt * (currentSampleRate * 0.01f); // Profondeur max ~10ms
            delaySamples = juce::jlimit(4.0f, maxDelaySamples - 8.0f, delaySamples);
        }

        // --- PREMIÈRE PASSE : LECTURE DU DELAI ET FILTRAGE ---
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            state.delayLine.setDelay(delaySamples);
            const float delayed = state.delayLine.popSample(0);

            float loopFiltered = state.loopFilter.process(delayed);
            loopFiltered = state.dcBlocker.process(loopFiltered);
            
            // On stocke la sortie filtrée de la boucle. C'est CE signal qui va servir au cross-feedback et au self-mod.
            loopOutputs[(size_t) ch] = loopFiltered;
        }

        // --- SECONDE PASSE : ECRITURE DANS LE DELAI (avec cross-feedback) ---
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto& state = channels[(size_t) ch];
            const float dry = buffer.getReadPointer(ch)[i];
            
            // CROSS-FEEDBACK : on injecte la sortie filtrée de l'AUTRE canal
            float crossFeedbackSignal = 0.0f;
            if (numChannels == 2) {
                int otherCh = 1 - ch;
                crossFeedbackSignal = loopOutputs[(size_t) otherCh] * crossFB;
            }

            const float inputFiltered = state.inputFilter.processSample(0, dry);
            
            // LA FORMULE MAGIQUE : entrée + feedback local (avec polarité) + cross-feedback
            float toWrite = inputFiltered + (loopOutputs[(size_t) ch] * fb) + crossFeedbackSignal;
            
            // Saturation douce pour la stabilité (le tanh est notre ami)
            toWrite = std::tanh(toWrite);

            state.delayLine.pushSample(0, toWrite);

            // Mix dry/wet : on inclut le cross-feedback dans le signal wet pour plus de largeur
            float wetSignal = loopOutputs[(size_t) ch] + crossFeedbackSignal;
            buffer.getWritePointer(ch)[i] = dry * (1.0f - wetMix) + wetSignal * wetMix;
        }
    }
}
