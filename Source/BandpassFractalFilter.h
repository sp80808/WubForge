#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
    BandpassFractalFilter - Self-similar parallel bandpass filter stack

    Creates "resonant shells" and singing textures through fractal geometry:
    - Each filter level targets a fractal-scaled center frequency
    - Golden ratio scaling (φ ≈ 1.618) for natural harmonic spacing
    - Key-tracked base center for musical consistency across MIDI notes
    - Parallel bandpass filters emphasize harmonics without broadband loss
    - Perfect for formant-like evolutions and metallic bass rings
*/
class BandpassFractalFilter
{
public:
    BandpassFractalFilter();
    ~BandpassFractalFilter();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    //==============================================================================
    // Parameter setters
    void setCurrentFreq (double freq);
    void setDepth (int depth);
    void setScaleFactor (float scale);
    void setMix (float mix);
    void setBaseCenter (float centerHz);
    void setBaseQ (float q);

    //==============================================================================
    // Getters for visualization
    int getDepth() const { return depth; }
    float getCurrentBaseCenter() const { return currentBaseCenter; }
    double getCurrentFreq() const { return currentFreq; }

private:
    //==============================================================================
    void updateCoefficients();

    //==============================================================================
    // DSP Components - Parallel bandpass filter bank
    static constexpr int maxDepth = 8;
    std::vector<juce::dsp::IIR::Filter<float>> fractalFilters;

    // Parameters
    double sampleRate = 44100.0;
    double currentFreq = 100.0;     // Current MIDI note frequency
    int depth = 4;                  // Number of fractal levels (2-8)
    float scaleFactor = 1.618f;     // Golden ratio for natural scaling
    float mix = 0.5f;               // Dry/wet mix (0-1)
    float baseCenter = 200.0f;      // Base center frequency
    float currentBaseCenter = 200.0f;
    float baseQ = 2.0f;             // Base Q factor for bandwidth

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandpassFractalFilter)
};
