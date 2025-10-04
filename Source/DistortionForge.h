#pragma once

#include "Module.h"
#include <juce_gui_extra/juce_gui_extra.h> // Temporarily for basic dependencies

//==============================================================================
/**
    DistortionForge - Professional distortion module using chowdsp_utils

    Provides multiple high-quality distortion algorithms with professional
    gain staging, tone control, and dry/wet mixing for seamless integration
    into the WubForge modular architecture.
*/
class DistortionForge : public DistortionModule
{
public:
    enum class Algorithm
    {
        Tanh,           // Hyperbolic tangent soft clipping
        HardClip,       // Hard clipping with anti-aliasing
        SoftClip,       // Polynomial soft clipping
        Wavefold,       // West Coast style wavefolding
        BitCrush        // Bit reduction with sample rate reduction
    };

    DistortionForge();
    ~DistortionForge() override = default;

    //==============================================================================
    // AudioModule interface
    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Distortion Forge"; }

    //==============================================================================
    // Parameter Controls
    void setAlgorithm (Algorithm algorithm);
    void setDrive (float driveDB);
    void setTone (float toneFreqHz);
    void setMix (float wetMix);

    //==============================================================================
    // Advanced Controls
    void setBias (float biasAmount);        // DC offset for asymmetric distortion
    void setBitDepth (float bitDepth);      // For bit crushing (1-16 bits)
    void setSampleRateReduction (float reduction); // Sample rate reduction factor

private:
    //==============================================================================
    // Internal Processing
    void processTanh (const juce::dsp::ProcessContextReplacing<float>& context);
    void processHardClip (const juce::dsp::ProcessContextReplacing<float>& context);
    void processSoftClip (const juce::dsp::ProcessContextReplacing<float>& context);
    void processWavefold (const juce::dsp::ProcessContextReplacing<float>& context);
    void processBitCrush (const juce::dsp::ProcessContextReplacing<float>& context);

    void updateFilters();
    void updateGainStaging();

    //==============================================================================
    // State & Parameters
    double sampleRate = 44100.0;
    Algorithm currentAlgorithm = Algorithm::Tanh;

    // Core parameters
    float driveDB = 0.0f;           // Input gain in dB (-20 to +40 dB)
    float toneFreqHz = 2000.0f;     // Tone filter cutoff (200-8000 Hz)
    float wetMix = 1.0f;           // Dry/wet mix (0-1)

    // Advanced parameters
    float biasAmount = 0.0f;        // DC bias for asymmetric distortion
    float bitDepth = 16.0f;         // Bit depth for bit crushing
    float sampleRateReduction = 1.0f; // Sample rate reduction factor

    //==============================================================================
    // DSP Components using chowdsp_utils

    // Main distortion processors
    chowdsp::ADAAHardClipper<float> hardClipper;
    chowdsp::ADAATanhClipper<float> tanhClipper;
    chowdsp::ADAASoftClipper<float> softClipper;
    chowdsp::WestCoastWavefolder<float> wavefolder;

    // Tone filtering
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> toneFilter;

    // Gain staging for professional audio quality
    juce::dsp::Gain<float> inputGain;      // Pre-distortion gain
    juce::dsp::Gain<float> outputGain;     // Post-distortion gain compensation
    juce::dsp::DryWetMixer<float> dryWetMixer;

    // Bit crushing components
    std::vector<float> bitCrushBuffer;      // For sample-and-hold processing
    int bitCrushCounter = 0;
    float bitCrushPhase = 0.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionForge)
};
