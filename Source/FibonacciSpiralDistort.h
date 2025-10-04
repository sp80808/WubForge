#pragma once

#include "Module.h"
#include "../JUCE/modules/juce_dsp/juce_dsp.h"
#include "../JUCE/modules/juce_audio_processors/juce_audio_processors.h"

/**
 * Fibonacci Spiral Distort (FSD) - A novel hybrid filter-distortion algorithm
 *
 * This module creates self-similar, consonant harmonics using golden ratio (φ ≈ 1.618)
 * spacing and Fibonacci approximations for "pleasing" richness without dissonance.
 * Perfect for transforming sterile sines into evolving, resonant monsters for dubstep bass.
 */
class FibonacciSpiralDistort : public DistortionModule
{
public:
    // Constructor
    FibonacciSpiralDistort();

    // Destructor
    ~FibonacciSpiralDistort() override;

    // AudioModule interface
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Fibonacci Spiral Distort"; }
    ModuleType getType() const override { return ModuleType::Distortion; }

    // FSD-specific parameters
    void setDrive(float driveDB);
    void setTone(float toneFreqHz);
    void setMix(float wetMix);

    // FSD-specific parameters
    void setSpiralDepth(float depth);      // 0.0-1.0: φ-resonator bank mix
    void setFibDrive(float fibDrive);      // 0.0-2.0: Base distortion with Fib scaling
    void setBloomRate(float bloomRate);    // 0.001-2.0s: Envelope attack/release
    void setVeilCutoff(float veilCutoff);  // 200-5000Hz: Base frequency for spiral filter
    void setResonance(float resonance);    // 0.0-0.8: φ-resonator feedback
    void setFibDepth(int fibN);           // 5-15: Fibonacci ratio depth
    void setMidiNote(float midiNote);     // 0-127: Key-dependent frequency scaling
    void setMorphAmount(float morph);     // 0.0-1.0: Simple WT to full spiral morphing

    // Parameter accessors
    float getSpiralDepth() const { return spiralDepth; }
    float getFibDrive() const { return fibDrive; }
    float getBloomRate() const { return bloomRate; }
    float getVeilCutoff() const { return veilCutoff; }
    float getResonance() const { return resonance; }
    int getFibDepth() const { return fibN; }

private:
    // Core algorithm components
    void updateResonatorBank();
    void processResonatorBank(const float* input, float* output, int numSamples);
    void processFibonacciDistortion(float* buffer, int numSamples);
    void processSpiralVeilFilter(float* buffer, int numSamples);

    // Resonator bank (φ-spaced BP filters with self-oscillation)
    static constexpr int MAX_RESONATORS = 4;
    struct Resonator {
        float phase = 0.0f;
        float frequency = 100.0f;
        float amplitude = 0.25f;
        float feedback = 0.9f;
        float prevOutput = 0.0f;
        juce::dsp::IIR::Filter<float> bpFilter;
        juce::dsp::IIR::Coefficients<float>::Ptr bpCoefficients;
    };
    std::array<Resonator, MAX_RESONATORS> resonators;
    float currentFrequency = 55.0f; // Track input frequency

    // Fibonacci distortion cascade (4-stage waveshaper)
    static constexpr int DISTORTION_STAGES = 4;
    struct DistortionStage {
        float drive = 1.0f;
        float envelope = 0.0f;
        float attackCoeff = 0.999f;
        float releaseCoeff = 0.999f;
    };
    std::array<DistortionStage, DISTORTION_STAGES> distortionStages;

    // Spiral veil filter (cascaded LPs with φ-cutoff spacing)
    static constexpr int VEIL_FILTERS = 3;
    struct VeilFilter {
        juce::dsp::IIR::Filter<float> filter;
        float cutoff = 500.0f;
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
    };
    std::array<VeilFilter, VEIL_FILTERS> veilFilters;

    // Parameters
    float spiralDepth = 0.3f;      // φ-resonator bank mix amount
    float fibDrive = 1.0f;         // Base distortion intensity with Fib scaling
    float bloomRate = 0.01f;       // Envelope attack/release time
    float veilCutoff = 500.0f;     // Base frequency for spiral filter cascade
    float resonance = 0.4f;        // φ-resonator feedback amount
    int fibN = 8;                  // Fibonacci ratio depth (5-15)
    float midiNote = 69.0f;        // 0-127: Key-dependent frequency scaling (A4 = 69)
    float morphAmount = 0.5f;      // 0.0-1.0: Simple WT to full spiral morphing

    // DSP state
    double sampleRate = 44100.0;
    float phi = (1.0f + std::sqrt(5.0f)) / 2.0f; // Golden ratio ≈ 1.618034

    // Envelope follower for dynamic processing
    float envelopeFollower = 0.0f;
    float envAttackCoeff = 0.999f;
    float envReleaseCoeff = 0.999f;

    // Fibonacci ratios cache (pre-computed for efficiency)
    std::array<float, 16> fibRatios;

    // Psychoacoustic processing (A-weighting for ear-friendly response)
    juce::dsp::IIR::Filter<float> aWeightFilter;
    juce::dsp::IIR::Coefficients<float>::Ptr aWeightCoefficients;

    // Block processing for RMS leveling
    int blockSize = 512;
    std::vector<float> rmsBuffer;
    std::vector<float> aWeightBuffer;
    int blockPosition = 0;

    // Self-oscillation feedback states for resonators
    std::array<float, MAX_RESONATORS> prevResonatorOutput = {0.0f};

    // Psychoacoustic Fib envelope follower
    float fibEnvelope = 0.0f;
    float fibAlpha = 0.619f; // 13/21 Fibonacci ratio for smooth decay

    // Utility functions
    void updateFibonacciRatios();
    void updateEnvelopeCoefficients();
    void updateResonatorFrequencies();
    void updateVeilFilterCutoffs();
    float processEnvelopeFollower(float input);
    float fibonacciRatio(int n) const;
};
