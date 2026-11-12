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
        
        // Appel avec les 32 paramètres (valeurs neutres)
        res.process(buffer, 
            0.0f, 0.85f, 8000.0f, 4000.0f, 0.707f, 0.5f, // Core
            0.0f, 0.0f, 0.0f, false,                     // Modulations
            1.0f, 0, 0.0f, false,                        // Saturation
            1.0f, 0.0f,                                  // Wet Gain & Tilt
            0, 1.0f, 0.0f, 3.6f, 0.5f, 0, 120.0f,        // LFO
            0, 0.0f, 0, 0.0f, 0, 0.0f, 0, 0.0f           // Matrix
        );
        
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 512; ++i)
                expect(std::isfinite(buffer.getSample(ch, i)));
    }
};

static ResonatorTest resonatorTest;
