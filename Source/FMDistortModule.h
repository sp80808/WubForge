#pragma once

#include "Module.h"

//==============================================================================
/**
    Applies FM-style distortion by using the input signal to modulate the
    phase of an oscillator, creating new metallic and bell-like sidebands.
*/
class FMDistortModule : public DistortionModule
{
public:
    FMDistortModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "FM Distort"; }

    //==============================================================================
    // Parameter Setters
    void setRatio (float ratio);
    void setModIndex (float index);

private:
    double sampleRate = 44100.0;

    // Manual phase management for the oscillator
    float phase = 0.0f;
    float phaseDelta = 0.0f;

    // Parameters
    float frequencyRatio = 1.0f;
    float modIndex = 1.0f; // Controls the intensity of the modulation

    void updatePhaseDelta();
};