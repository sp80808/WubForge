#pragma once

#include "Module.h"
#include <vector>
#include <array>

//==============================================================================
/**
    Advanced fractal filter with multiple self-similar patterns inspired by
    FilterVerse and modern bass design techniques. Creates rich, organic
    harmonic structures using golden ratio and musical interval scaling.
*/
class FractalFilterModule : public FilterModule
{
public:
    // Fractal scaling patterns
    enum class FractalPattern
    {
        GoldenRatio = 0,    // φ = 1.618... (FilterVerse inspired)
        Fibonacci,          // Fibonacci sequence ratios
        HarmonicSeries,     // Natural harmonic series
        PrimeRatios,        // Prime number ratios for dissonance
        MusicalIntervals    // Musical intervals (3rd, 5th, 7th, etc.)
    };

    FractalFilterModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Fractal Filter Pro"; }

    //==============================================================================
    // Enhanced Parameter Setters
    void setType (int type); // 0: LP, 1: HP, 2: BP, 3: Notch, 4: Allpass
    void setBaseFrequency (float freqHz);
    void setQ (float q);
    void setDepth (int depth);
    void setRatio (float ratio);
    void setFractalPattern (FractalPattern pattern);
    void setFractalFeedback (float feedback); // 0.0 to 0.1
    void setHarmonicDrive (float drive);     // 0.0 to 2.0

private:
    void updateCoefficients();
    void updateFractalPattern();
    float calculateFractalFrequency(int stage, float baseFreq);

    double sampleRate = 44100.0;
    bool needsUpdate = true;

    // DSP components
    static constexpr int maxDepth = 8;
    std::vector<juce::dsp::IIR::Filter<float>> filterChain;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    std::vector<std::shared_ptr<Coefficients>> coefficients;

    // Enhanced parameters
    int filterType = 0;
    float baseFrequency = 100.0f;  // Lower base for bass processing
    float q = 0.707f;              // Butterworth Q for musical response
    int depth = 4;
    float ratio = PHI;             // Default to golden ratio
    FractalPattern currentPattern = FractalPattern::GoldenRatio;

    // Advanced features
    float fractalFeedback = 0.0f;  // Subtle feedback for resonance
    float harmonicDrive = 1.0f;    // Non-linear drive per stage

    // Pre-calculated fractal frequencies
    std::array<float, maxDepth> fractalFrequencies;

    // Fractal pattern definitions
    std::array<FractalPattern, 5> fractalPatterns;
};