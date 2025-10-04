#pragma once

#include "Module.h"
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
    SpectralMorphingModule - Core spectral morphing system for WubForge

    Provides real-time spectral morphing between source and target spectra:
    - FFT-based analysis and reconstruction for <5ms latency
    - XY morphing control (source→target morphing)
    - Formant preservation during morphing
    - Key-tracked spectral response
    - Optimized for bass processing and creative sound design
*/
class SpectralMorphingModule : public AudioModule
{
public:
    SpectralMorphingModule();
    ~SpectralMorphingModule() override = default;

    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    //==============================================================================
    const juce::String getName() const override { return "Spectral Morphing"; }
    ModuleType getType() const override { return ModuleType::Filter; }

    //==============================================================================
    // Core morphing controls
    void setMorphPosition (float x, float y);  // XY morphing control
    void setMorphTime (float timeMs);          // Morph transition time
    void setSpectralRange (float range);       // 0=full, 1=high, 2=mid, 3=low
    void setWetDryMix (float mix);

    //==============================================================================
    // Advanced spectral controls
    void setFFTSize (int size);                // 256, 512, 1024, 2048
    void setOverlapFactor (float overlap);     // 0.25, 0.5, 0.75
    void setFormantPreservation (float amount); // 0-1
    void setSpectralSmoothing (float smoothing); // 0-1

    //==============================================================================
    // Key tracking integration
    void setCurrentFreq (double freq);

    //==============================================================================
    // Getters for visualization
    float getCurrentMorphX() const { return morphX; }
    float getCurrentMorphY() const { return morphY; }
    int getCurrentFFTSize() const { return fftSize; }
    float getCurrentLatency() const;

    //==============================================================================
    // Spectral data access for spectrogram
    const std::vector<float>& getCurrentMagnitudeBuffer() const { return magnitudeBuffer; }

private:
    //==============================================================================
    void updateFFTSize();
    void analyzeInputSpectrum();
    void applySpectralMorphing();
    void reconstructOutput();

    //==============================================================================
    // FFT processing components
    std::unique_ptr<juce::dsp::FFT> fft;
    std::unique_ptr<juce::dsp::FFT> ifft;

    // Windowing for overlap-add
    std::vector<float> windowBuffer;

    // Spectral data buffers
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<float> spectralBuffer;
    std::vector<float> magnitudeBuffer;
    std::vector<float> phaseBuffer;

    // Morphing state
    std::vector<float> sourceSpectrum;
    std::vector<float> targetSpectrum;
    std::vector<float> currentSpectrum;

    // Parameters
    float morphX = 0.0f;           // X position in morph space
    float morphY = 0.0f;           // Y position in morph space
    float morphTime = 100.0f;      // Morph transition time in ms
    float spectralRange = 0.0f;    // Spectral range focus
    float wetDryMix = 1.0f;        // Wet/dry mix

    // Advanced parameters
    int fftSize = 512;             // FFT size for latency control
    float overlapFactor = 0.5f;    // Overlap for smooth processing
    float formantPreservation = 0.8f; // Formant preservation amount
    float spectralSmoothing = 0.3f;   // Spectral smoothing factor

    // State
    double sampleRate = 44100.0;
    int hopSize = 256;             // Hop size for overlap-add
    int bufferPosition = 0;
    double currentFreq = 100.0;    // Current MIDI note frequency

    //==============================================================================
    void initializeBuffers();
    void applyWindowing();
    void removeWindowing();
    void updateMorphing();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralMorphingModule)
};
