#include "FractalFilter.h"
#include <cmath>

// Golden ratio and musical constants
static constexpr float PHI = 1.618033988749895f;  // Golden ratio
static constexpr float PHI_INV = 0.618033988749895f;  // Golden ratio inverse

// Fibonacci sequence for fractal ratios
static constexpr std::array<float, 8> FIBONACCI_RATIOS = {
    1.0f, 2.0f, 1.5f, 1.666f, 1.6f, 1.625f, 1.615f, 1.619f
};

// Prime number ratios for dissonance
static constexpr std::array<float, 8> PRIME_RATIOS = {
    1.0f, 1.791f, 1.931f, 2.023f, 2.089f, 2.141f, 2.183f, 2.219f
};

// Musical intervals (just intonation)
static constexpr std::array<float, 8> MUSICAL_INTERVALS = {
    1.0f,    // Unison
    1.25f,   // Just major third
    1.5f,    // Perfect fifth
    1.75f,   // Just minor seventh
    2.0f,    // Octave
    2.25f,   // Octave + major third
    2.5f,    // Octave + fifth
    2.75f    // Octave + seventh
};

FractalFilterModule::FractalFilterModule()
{
    filterChain.resize (maxDepth);
    coefficients.resize (maxDepth);

    // Initialize fractal patterns
    fractalPatterns[0] = FractalPattern::GoldenRatio;
    fractalPatterns[1] = FractalPattern::Fibonacci;
    fractalPatterns[2] = FractalPattern::HarmonicSeries;
    fractalPatterns[3] = FractalPattern::PrimeRatios;
    fractalPatterns[4] = FractalPattern::MusicalIntervals;
}

void FractalFilterModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    for (auto& filter : filterChain)
        filter.prepare(spec);

    // Initialize fractal pattern frequencies
    updateFractalPattern();

    needsUpdate = true;
}

void FractalFilterModule::reset()
{
    for (auto& filter : filterChain)
        filter.reset();
}

void FractalFilterModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (needsUpdate)
        updateCoefficients();

    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    // Apply fractal filter chain with optional feedback
    for (int i = 0; i < depth; ++i)
    {
        filterChain[i].process(context);
    }

    // Add subtle feedback for fractal resonance
    if (fractalFeedback > 0.01f)
    {
        auto feedbackBlock = outputBlock;
        feedbackBlock *= fractalFeedback;
        outputBlock += feedbackBlock;
    }
}

void FractalFilterModule::setType (int type)
{
    if (type != filterType)
    {
        filterType = type;
        needsUpdate = true;
    }
}

void FractalFilterModule::setBaseFrequency (float freqHz)
{
    if (freqHz != baseFrequency)
    {
        baseFrequency = freqHz;
        needsUpdate = true;
    }
}

void FractalFilterModule::setQ (float newQ)
{
    if (newQ != q)
    {
        q = newQ;
        needsUpdate = true;
    }
}

void FractalFilterModule::setDepth (int newDepth)
{
    // Clamp depth to the valid range
    newDepth = juce::jlimit (1, (int)maxDepth, newDepth);
    if (newDepth != depth)
    {
        depth = newDepth;
        needsUpdate = true; // Coefficients for active filters might not change, but the chain length does
    }
}

void FractalFilterModule::setRatio (float newRatio)
{
    if (newRatio != ratio)
    {
        ratio = newRatio;
        needsUpdate = true;
    }
}

void FractalFilterModule::setFractalPattern (FractalPattern pattern)
{
    if (pattern != currentPattern)
    {
        currentPattern = pattern;
        updateFractalPattern();
        needsUpdate = true;
    }
}

void FractalFilterModule::setFractalFeedback (float feedback)
{
    fractalFeedback = juce::jlimit (0.0f, 0.1f, feedback);
}

void FractalFilterModule::setHarmonicDrive (float drive)
{
    harmonicDrive = juce::jlimit (0.0f, 2.0f, drive);
}

void FractalFilterModule::updateCoefficients()
{
    for (int i = 0; i < depth; ++i)
    {
        float freq = fractalFrequencies[i];

        switch (filterType)
        {
            case 0: // Low-pass
                *coefficients[i] = *Coefficients::makeLowPass (sampleRate, freq, q);
                break;
            case 1: // High-pass
                *coefficients[i] = *Coefficients::makeHighPass (sampleRate, freq, q);
                break;
            case 2: // Band-pass
                *coefficients[i] = *Coefficients::makeBandPass (sampleRate, freq, q);
                break;
            case 3: // Notch
                *coefficients[i] = *Coefficients::makeNotch (sampleRate, freq, q);
                break;
            case 4: // Allpass
                *coefficients[i] = *Coefficients::makeAllPass (sampleRate, freq, q);
                break;
        }

        filterChain[i].coefficients = *coefficients[i];
    }

    needsUpdate = false;
}

void FractalFilterModule::updateFractalPattern()
{
    for (int i = 0; i < maxDepth; ++i)
    {
        fractalFrequencies[i] = calculateFractalFrequency(i, baseFrequency);
    }
}

float FractalFilterModule::calculateFractalFrequency(int stage, float baseFreq)
{
    switch (currentPattern)
    {
        case FractalPattern::GoldenRatio:
            return baseFreq * pow(PHI, (float)stage);

        case FractalPattern::Fibonacci:
            return baseFreq * FIBONACCI_RATIOS[stage % FIBONACCI_RATIOS.size()];

        case FractalPattern::HarmonicSeries:
            return baseFreq * (stage + 1); // Harmonic series: f, 2f, 3f, 4f...

        case FractalPattern::PrimeRatios:
            return baseFreq * PRIME_RATIOS[stage % PRIME_RATIOS.size()];

        case FractalPattern::MusicalIntervals:
            return baseFreq * MUSICAL_INTERVALS[stage % MUSICAL_INTERVALS.size()];

        default:
            return baseFreq * pow(PHI, (float)stage);
    }
}