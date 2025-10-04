#pragma once

#include "Module.h"
#include "FractalFilter.h"

//==============================================================================
/**
    FractalFilterModule - Self-similar parallel low-pass filter stack

    Creates harmonic richness through fractal geometry principles:
    - Each filter level processes the original input with scaled cutoff frequencies
    - Golden ratio scaling (φ ≈ 1.618) for natural harmonic buildup
    - Key-tracked base cutoff for musical consistency
    - Parallel summing avoids attenuation issues of serial recursion
*/
class FractalFilterModule : public FilterModule
{
public:
    FractalFilterModule();
    ~FractalFilterModule() override;

    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    //==============================================================================
    const juce::String getName() const override { return "Fractal Filter"; }
    ModuleType getType() const override { return ModuleType::Filter; }

    //==============================================================================
    // Parameter setters
    void setCurrentFreq (double freq);
    void setDepth (int depth);
    void setScaleFactor (float scale);
    void setMix (float mix);
    void setBaseCutoff (float cutoffHz);

    //==============================================================================
    // Getters for visualization
    int getDepth() const { return fractalFilter.getDepth(); }
    float getCurrentBaseCutoff() const { return fractalFilter.getCurrentBaseCutoff(); }
    double getCurrentFreq() const { return fractalFilter.getCurrentFreq(); }

private:
    //==============================================================================
    FractalFilter fractalFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FractalFilterModule)
};
