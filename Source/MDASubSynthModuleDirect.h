#pragma once

#include "Module.h"

//==============================================================================
/**
    MDASubSynthModuleDirect - Direct implementation of MDA Sub-Bass synthesizer

    Recreated from the MDA SubSynth algorithm, this module generates subharmonic
    content for bass enhancement using octave division and pulse-width modulation.

    Based on the classic MDA SubSynth processor.
*/
class MDASubSynthModuleDirect : public AudioModule
{
public:
    MDASubSynthModuleDirect();
    ~MDASubSynthModuleDirect() override = default;

    //==============================================================================
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    //==============================================================================
    const juce::String getName() const override { return "MDA SubSynth"; }
    ModuleType getType() const override { return ModuleType::Filter; }

    //==============================================================================
    // SubSynth Controls
    void setType(int type);              // 0=Distort, 1=Divide, 2=Invert, 3=Key Osc
    void setWetMix(float wet);           // Sub-bass level (0.0 - 1.0)
    void setDryMix(float dry);           // Original signal level (0.0 - 1.0)
    void setThreshold(float thresholdDB); // Trigger threshold in dB (-60 to 0)
    void setTune(float tune);            // Filter frequency/tune parameter (0.0 - 1.0)

private:
    //==============================================================================
    // MDA Algorithm parameters
    int _type = 0;           // 0=Distort, 1=Divide, 2=Invert, 3=Key Osc
    float _wet = 0.3f;       // Amount of sub-bass to add
    float _dry = 1.0f;       // Amount of original signal
    float _threshold = 0.06f; // Threshold level (linear)
    float _tune = 0.6f;      // Filter frequency parameter

    // Internal state variables (from MDA algorithm)
    float _sign = 1.0f;
    float _phase = 1.0f;
    float _oscPhase = 0.0f;
    float _phaseInc = 0.0f;
    float _env = 0.0f;
    float _decay = 0.0f;

    // Low-pass filter coefficients
    float _filti = 0.0f;
    float _filto = 0.0f;

    // Filter delays (four stages)
    float _filt1 = 0.0f;
    float _filt2 = 0.0f;
    float _filt3 = 0.0f;
    float _filt4 = 0.0f;

    // Configuration
    double _sampleRate = 44100.0;

    //==============================================================================
    void updateParameters();
    void updateFilterCoefficients();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDASubSynthModuleDirect)
};
