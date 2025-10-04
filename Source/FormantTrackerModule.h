#pragma once

#include "Module.h"
#include "FormantTracker.h"

//==============================================================================
/**
    FormantTracker module that implements key-tracked formant peaks.
    This module provides vocal-like resonance effects that track the input pitch.
*/
class FormantTrackerModule : public FilterModule
{
public:
    FormantTrackerModule();
    ~FormantTrackerModule() override;

    //==============================================================================
    // AudioModule interface
    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    
    const juce::String getName() const override { return "Formant Tracker"; }

    //==============================================================================
    // Parameter setters
    void setKeyTrackAmount (float amount);
    void setFormantGain (float gainDb);
    void setFormantQ (float Q);
    void setBaseFormants (const std::array<double, 3>& formants);
    void setBaseFrequency (double baseFreq);

    //==============================================================================
    // Getters for visualization
    float getKeyTrackAmount() const;
    float getFormantGain() const;
    float getFormantQ() const;
    std::array<double, 3> getCurrentFormantFrequencies() const;

private:
    //==============================================================================
    FormantTracker formantTracker;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FormantTrackerModule)
};
