#pragma once

#include "Module.h"
#include <vector>
#include <array>
#include <complex>

//==============================================================================
/**
    Advanced spectral morphing engine inspired by Xfer Serum 2 and Vital.
    Enables real-time morphing between spectral snapshots with phase preservation
    for natural-sounding transitions between bass timbres.
*/
class SpectralMorphingModule : public FilterModule
{
public:
    SpectralMorphingModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Spectral Morpher"; }

    //==============================================================================
    // Spectral Morphing Parameters
    void setMorphAmount (float amount);        // 0.0 = Source A, 1.0 = Source B
    void setMorphSpeed (float speed);          // Transition speed (0.0 to 1.0)
    void setSpectralWarping (float warp);      // Spectral shape modification
    void setPhasePreservation (float preserve); // 0.0 = Free phase, 1.0 = Preserve phase

    // Spectral snapshot management
    void captureSpectralSnapshot (int slot);   // Capture current spectral state
    void setActiveSnapshots (int sourceSlot, int targetSlot);

private:
    void performFFT();
    void performIFFT();
    void updateSpectralMorphing();
    void applySpectralWarping();

    double sampleRate = 44100.0;
    int blockSize = 512;

    // FFT processing
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<std::complex<float>> frequencyDomain;
    std::vector<float> windowBuffer;

    // Spectral snapshots (A and B for morphing)
    struct SpectralSnapshot {
        std::vector<float> magnitude;
        std::vector<float> phase;
        float centroid = 0.0f;
        float spectralFlux = 0.0f;
    };

    std::array<SpectralSnapshot, 4> spectralSnapshots; // 4 snapshot slots
    int activeSourceSlot = 0;
    int activeTargetSlot = 1;

    // Morphing parameters
    float morphAmount = 0.0f;        // Current morph position
    float targetMorphAmount = 0.0f;  // Target morph position
    float morphSpeed = 0.1f;         // Morph transition speed
    float spectralWarping = 0.0f;    // Spectral shape modification
    float phasePreservation = 0.8f;  // How much to preserve phase

    // Analysis data
    float currentCentroid = 0.0f;
    float currentSpectralFlux = 0.0f;

    // FFT utilities
    static constexpr int fftSize = 2048;
    static constexpr int hopSize = 512;

    int bufferPosition = 0;
    int windowSize = fftSize;
};
