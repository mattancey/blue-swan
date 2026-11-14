#include <juce_core/juce_core.h>
#include "../Dsp/Resonator.h"

class ResonatorTest : public juce::UnitTest
{
public:
    ResonatorTest() : juce::UnitTest("Resonator DSP Test") {}

    void runTest() override
    {
        beginTest("Process Silence");
        Resonator res;
        res.prepare(44100.0, 512, 2);
        
        juce::AudioBuffer<float> buffer(2, 512);
        buffer.clear();
        
        // Création d'un ModParams avec des valeurs neutres pour le test
        ModParams p;
        p.pitch = 0.0f;
        p.feedback = 0.85f;
        p.pluck = 0.0f;
        p.inharm = 0.0f;
        p.inputCutoff = 8000.0f;
        p.loopCutoff = 4000.0f;
        p.dryWet = 0.5f;
        p.jitter = 0.0f;
        p.crossFb = 0.0f;
        p.selfMod = 0.0f;
        p.wetGain = 1.0f;
        p.macro = 0.0f;
        
        p.lfo1Sync = 0; p.lfo2Sync = 0;
        p.lfo1Shape = 0; p.lfo2Shape = 0;
        p.lfo1Rate = 1.0f; p.lfo2Rate = 1.0f;
        p.lfo1Chaos = 3.6f; p.lfo2Chaos = 3.6f;
        p.lfo1Pw = 0.5f; p.lfo2Pw = 0.5f;
        
        p.m1_pitch = 0.0f; p.m2_pitch = 0.0f;
        p.m1_feed = 0.0f; p.m2_feed = 0.0f;
        p.m1_incut = 0.0f; p.m2_incut = 0.0f;
        p.m1_loopcut = 0.0f; p.m2_loopcut = 0.0f;
        p.m1_hbeta = 0.0f; p.m2_hbeta = 0.0f;
        p.m1_inharm = 0.0f; p.m2_inharm = 0.0f;
        p.m1_cross = 0.0f; p.m2_cross = 0.0f;
        p.m1_self = 0.0f; p.m2_self = 0.0f;
        p.m1_drift = 0.0f; p.m2_drift = 0.0f;
        p.m1_drywet = 0.0f; p.m2_drywet = 0.0f;
        p.m1_wetgain = 0.0f; p.m2_wetgain = 0.0f;
        p.m1_macro = 0.0f; p.m2_macro = 0.0f;
        
        // Appel avec les 4 arguments attendus
        res.process(buffer, p, 120.0, false);
        
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 512; ++i)
                expect(std::isfinite(buffer.getSample(ch, i)));
    }
};

static ResonatorTest resonatorTest;
