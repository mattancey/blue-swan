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
        
        res.process(buffer);
        
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 512; ++i)
                expect(std::isfinite(buffer.getSample(ch, i)));
    }
};

static ResonatorTest resonatorTest;
