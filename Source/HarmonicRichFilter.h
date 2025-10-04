#pragma once

#include "Module.h"
#include <vector>
#include <array>
#include <memory>
#include <cstdint>

//==============================================================================
/**
    Advanced harmonically rich filter with three novel filter shapes designed
    for modern bass synthesis and sound design. Features CPU monitoring,
    auto-Q clamping, and real-time performance optimization.

    Filter Shapes:
    1. Helical Sine Veil: 6 parallel sine oscillators with golden-ratio spacing
    2. Cascade Harmonic Bloom: 3 serial asymmetric LP filters with sine bloom
    3. Spectral Sine Helix: 7 sines with Gaussian-shaved LP and all-pass helix
*/
class HarmonicRichFilter : public FilterModule
{
public:
    // Filter shape enumeration
    enum class FilterShape
    {
        HelicalSineVeil = 0,
        CascadeHarmonicBloom,
        SpectralSineHelix
    };

    HarmonicRichFilter();
    ~HarmonicRichFilter() override;

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Harmonic Rich Filter"; }

    //==============================================================================
    // Parameter Setters
    void setFilterShape (FilterShape shape);
    void setCutoffFrequency (float freqHz);
    void setResonance (float resonance);
    void setDrive (float drive);
    void setMix (float mix);
    void setCurrentFreq (float freqHz); // Key tracking frequency

    //==============================================================================
    // Advanced Parameters
    void setHelicalVeilDepth (float depth);      // 0.0 to 1.0
    void setBloomIntensity (float intensity);    // 0.0 to 2.0
    void setHelixPhaseMod (float modAmount);     // 0.0 to 1.0
    void setEnvelopeSensitivity (float sensitivity); // 0.0 to 1.0

private:
    //==============================================================================
    // Core DSP Processing Methods
    void processHelicalSineVeil (const float* input, float* output, int numSamples);
    void processCascadeHarmonicBloom (const float* input, float* output, int numSamples);
    void processSpectralSineHelix (const float* input, float* output, int numSamples);

    //==============================================================================
    // Utility Classes
    class EnvelopeFollower
    {
    public:
        void prepare (float sampleRate);
        void reset();
        float process (float input);
        void setAttackTime (float attackMs);
        void setReleaseTime (float releaseMs);

    private:
        float attackCoeff = 0.0f;
        float releaseCoeff = 0.0f;
        float envelope = 0.0f;
        float sampleRate = 44100.0f;
    };

    class PerformanceMonitor
    {
    public:
        void prepare (float sampleRate);
        void update (int numSamples);
        float getCpuUsage() const { return cpuUsage; }
        bool isOverloaded() const { return cpuUsage > 5.0f; }

    private:
        double sampleRate = 44100.0;
        int64_t samplesProcessed = 0;
        int64_t lastUpdateTime = 0;
        float cpuUsage = 0.0f;
        static constexpr float updateInterval = 0.1f; // Update every 100ms
    };

    //==============================================================================
    // DSP Components
    static constexpr int maxHelicalOscillators = 6;
    static constexpr int maxBloomStages = 3;
    static constexpr int maxHelixSines = 7;

    // Helical Sine Veil Components
    std::array<juce::dsp::Oscillator<float>, maxHelicalOscillators> helicalOscillators;
    juce::dsp::StateVariableTPTFilter<float> veilFilter;
    juce::dsp::Oscillator<float> veilLFO;
    EnvelopeFollower veilEnvelope;

    // Cascade Harmonic Bloom Components
    std::array<juce::dsp::StateVariableTPTFilter<float>, maxBloomStages> bloomFilters;
    std::array<juce::dsp::Oscillator<float>, maxBloomStages> bloomModulators;
    float bloomFeedback[maxBloomStages] = {0.0f};

    // Spectral Sine Helix Components
    std::array<juce::dsp::Oscillator<float>, maxHelixSines> helixOscillators;
    juce::dsp::StateVariableTPTFilter<float> helixFilter;
    std::array<juce::dsp::IIR::Filter<float>, maxHelixSines> allpassHelix;

    //==============================================================================
    // Parameters and State
    FilterShape currentShape = FilterShape::HelicalSineVeil;
    float cutoffFreq = 1000.0f;
    float resonance = 0.707f;
    float drive = 1.0f;
    float mix = 1.0f;
    float currentKeyFreq = 440.0f;

    // Advanced parameters
    float helicalVeilDepth = 0.5f;
    float bloomIntensity = 1.0f;
    float helixPhaseMod = 0.3f;
    float envelopeSensitivity = 0.7f;

    // Performance and monitoring
    PerformanceMonitor performanceMonitor;
    double sampleRate = 44100.0;
    bool needsUpdate = true;

    //==============================================================================
    // Helper Methods
    void updateCoefficients();
    void clampQValue();
    float calculateGoldenRatioPhase(int oscillatorIndex);
    float processWithSaturation(float input, float driveAmount);

    //==============================================================================
    // Constants
    static constexpr float goldenRatio = 1.618033988749f;
    static constexpr float minQ = 0.5f;
    static constexpr float maxQ = 5.0f;
    static constexpr float maxCpuUsage = 5.0f; // 5% target
};
