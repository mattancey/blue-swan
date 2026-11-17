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
        
        ModParams p{};
        p.pitch = 0.0f; p.feedback = 0.85f; p.pluck = 0.0f; p.inharm = 0.0f;
        p.inputCutoff = 8000.0f; p.loopCutoff = 4000.0f; p.dryWet = 0.5f;
        p.jitter = 0.0f; p.crossFb = 0.0f; p.selfMod = 0.0f; p.wetGain = 1.0f; p.macro = 0.0f;
        
        p.lfo1Sync = 0; p.lfo2Sync = 0; p.lfo1Shape = 0; p.lfo2Shape = 0;
        p.lfo1Rate = 1.0f; p.lfo2Rate = 1.0f; p.lfo1Pw = 0.5f; p.lfo2Pw = 0.5f;
        p.lfo1Phase = 0.0f; p.lfo2Phase = 0.0f; // Ajouté pour correspondre au nouveau ModParams
        p.lfo1Bypass = false; p.lfo2Bypass = false;
        
        res.process(buffer, p, 120.0, false);
        
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 512; ++i)
                expect(std::isfinite(buffer.getSample(ch, i)));
    }
};

static ResonatorTest resonatorTest;
