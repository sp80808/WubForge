#pragma once

#include "Module.h"
#include <vector>

//==============================================================================
/**
    A filter that applies a series of IIR filters recursively, creating a
    self-similar, fractal-like frequency response.
*/
class FractalFilterModule : public FilterModule
{
public:
    FractalFilterModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Fractal Filter"; }

    //==============================================================================
    // Parameter Setters
    void setType (int type); // 0: LP, 1: HP, 2: BP
    void setBaseFrequency (float freqHz);
    void setQ (float q);
    void setDepth (int depth);
    void setRatio (float ratio);

private:
    void updateCoefficients();

    double sampleRate = 44100.0;
    bool needsUpdate = true;

    // DSP components
    static constexpr int maxDepth = 8;
    std::vector<juce::dsp::IIR::Filter<float>> filterChain;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    std::vector<std::shared_ptr<Coefficients>> coefficients;

    // Parameters
    int filterType = 0;
    float baseFrequency = 1000.0f;
    float q = 1.0f;
    int depth = 4;
    float ratio = 0.5f;
};