#pragma once

#include <chowdsp_eq/chowdsp_eq.h>
#include "Module.h"

//==============================================================================
/**
    ChowEQModule - Professional EQ using chowdsp_utils

    Provides high-quality parametric EQ processing with multiple bands,
    integrated with WubForge's modular architecture.
*/
class ChowEQModule : public AudioModule
{
public:
    ChowEQModule();
    ~ChowEQModule() override = default;

    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    //==============================================================================
    const juce::String getName() const override { return "Chow EQ"; }
    ModuleType getType() const override { return ModuleType::Filter; }

    //==============================================================================
    // EQ Controls
    void setLowGain(float gainDB) { lowBand.setGain(gainDB); }
    void setMidGain(float gainDB) { midBand.setGain(gainDB); }
    void setHighGain(float gainDB) { highBand.setGain(gainDB); }

    void setMidFreq(float frequencyHz);
    void setLowQ(float quality) { lowBand.setQValue(quality); }
    void setMidQ(float quality) { midBand.setQValue(quality); }
    void setHighQ(float quality) { highBand.setQValue(quality); }

private:
    //==============================================================================
    // EQ Processing (using chowdsp_utils)
    chowdsp::EQ::EQBand<float> lowBand;
    chowdsp::EQ::EQBand<float> midBand;
    chowdsp::EQ::EQBand<float> highBand;

    float currentSampleRate = 44100.0f;
    float midFrequency = 1000.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowEQModule)
};
