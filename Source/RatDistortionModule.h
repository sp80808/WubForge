#pragma once

#include "Module.h"

//==============================================================================
/**
    A distortion module that models the essential characteristics of the
    classic ProCo RAT distortion pedal, known for its aggressive hard-clipping.
*/
class RatDistortionModule : public DistortionModule
{
public:
    RatDistortionModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Rodent Distortion"; } // Avoid trademark

    //==============================================================================
    // Parameter Setters
    void setDrive (float drive); // 0-1
    void setTone (float tone);   // 0-1, maps to filter cutoff
    void setLevel (float level); // 0-1

private:
    double sampleRate = 44100.0;

    // DSP Components
    juce::dsp::Gain<float> inputGain;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> toneFilter;
    juce::dsp::Gain<float> outputGain;

    // Parameters
    float drive = 0.5f;
    float tone = 0.5f;
    float level = 0.5f;

    void updateToneFilter();
};