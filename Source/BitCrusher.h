#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class BitCrusher
{
public:
    BitCrusher();
    ~BitCrusher();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    //==============================================================================
    // Parameter setters
    void setBitDepth (float bitDepth);
    void setFilterCutoff (float cutoffHz);
    void setDryWetMix (float mix);

    //==============================================================================
    // Getters for visualization
    float getCurrentBitDepth() const { return currentBitDepth; }
    float getCurrentFilterCutoff() const { return currentFilterCutoff; }
    float getDryWetMix() const { return dryWetMix; }

private:
    //==============================================================================
    // DSP Components
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> antiAliasingFilter;
    juce::dsp::DryWetMixer<float> mixer;

    // Parameters
    float bitDepth = 8.0f;          // Bit depth (1-16 bits)
    float currentBitDepth = 8.0f;
    float filterCutoff = 8000.0f;   // Filter cutoff frequency (Hz)
    float currentFilterCutoff = 8000.0f;
    float dryWetMix = 1.0f;         // Dry/wet mix (0 = dry, 1 = wet)

    // State
    double sampleRate = 44100.0;
    float lastFilterCutoff = 8000.0f;

    //==============================================================================
    float processBitCrushing (float input);
    void updateFilter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitCrusher)
};