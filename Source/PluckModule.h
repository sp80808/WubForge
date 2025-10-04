#pragma once

#include "Module.h"
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
    A physical modeling module that simulates a plucked string using the
    Karplus-Strong algorithm.
*/
class PluckModule : public FilterModule
{
public:
    PluckModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Karplus-Strong Pluck"; }

    //==============================================================================
    // Parameter Setters
    void setDecay (float decay);     // 0-1, controls filter cutoff
    void setDamping (float damping); // 0-1, controls filter Q

    // Trigger the pluck with a burst of noise
    void pluck();

private:
    void updateFilter();

    double sampleRate = 44100.0;

    // DSP Components
    juce::dsp::DelayLine<float> delayLine { 44100 };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter;

    // State & Parameters
    bool needsToPluck = true;
    float decay = 0.5f;
    float damping = 0.5f;
};