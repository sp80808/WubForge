#include "BandpassFractalFilter.h"
#include <algorithm>

//==============================================================================
BandpassFractalFilter::BandpassFractalFilter()
{
}

BandpassFractalFilter::~BandpassFractalFilter()
{
}

//==============================================================================
void BandpassFractalFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    reset();
    updateCoefficients();
}

void BandpassFractalFilter::reset()
{
    for (auto& filter : fractalFilters)
    {
        filter.reset();
    }
}

//==============================================================================
void BandpassFractalFilter::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();
    auto numChannels = outputBlock.getNumChannels();

    if (fractalFilters.empty() || mix <= 0.0f)
        return;

    // Store original input for parallel processing
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto inputChannel = inputBlock.getChannelPointer (ch);
        auto outputChannel = outputBlock.getChannelPointer (ch);

        // Copy input to output initially (dry signal)
        std::copy (inputChannel, inputChannel + numSamples, outputChannel);
    }

    if (fractalFilters.empty() || mix <= 0.0f)
        return;

    // Process each fractal level in parallel (each processes original input)
    for (size_t level = 0; level < fractalFilters.size(); ++level)
    {
        // Create temporary buffer for this level's output
        std::vector<float> levelOutput (numChannels * numSamples, 0.0f);

        // Copy original input to temp buffer
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto inputChannel = inputBlock.getChannelPointer (ch);
            std::copy (inputChannel, inputChannel + numSamples, levelOutput.data() + ch * numSamples);
        }

        juce::dsp::AudioBlock<float> levelBlock (levelOutput);
        juce::dsp::ProcessContextReplacing<float> levelContext (levelBlock);

        // Apply this fractal level's bandpass filter
        fractalFilters[level].process (levelContext);

        // Add to output block (accumulating parallel branches)
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto outputChannel = outputBlock.getChannelPointer (ch);
            auto levelChannel = levelOutput.data() + ch * numSamples;
            for (int sample = 0; sample < numSamples; ++sample)
            {
                outputChannel[sample] += levelChannel[sample];
            }
        }
    }

    // Apply wet/dry mix
    if (mix < 1.0f)
    {
        // Blend: output = (dry * (1-mix)) + (wet * mix)
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto outputChannel = outputBlock.getChannelPointer (ch);
            auto inputChannel = inputBlock.getChannelPointer (ch);

            for (int sample = 0; sample < numSamples; ++sample)
            {
                float drySample = inputChannel[sample] * (1.0f - mix);
                outputChannel[sample] = drySample + (outputChannel[sample] * mix);
            }
        }
    }
}

//==============================================================================
void BandpassFractalFilter::setCurrentFreq (double freq)
{
    if (std::abs (currentFreq - freq) > 1.0)
    {
        currentFreq = freq;
        updateCoefficients();
    }
}

void BandpassFractalFilter::setDepth (int newDepth)
{
    depth = std::max (2, std::min (maxDepth, newDepth));
    updateCoefficients();
}

void BandpassFractalFilter::setScaleFactor (float scale)
{
    scaleFactor = std::max (1.0f, std::min (3.0f, scale));
    updateCoefficients();
}

void BandpassFractalFilter::setMix (float newMix)
{
    mix = std::max (0.0f, std::min (1.0f, newMix));
}

void BandpassFractalFilter::setBaseCenter (float centerHz)
{
    baseCenter = std::max (50.0f, std::min (static_cast<float>(sampleRate) / 4.0f, centerHz));
    updateCoefficients();
}

void BandpassFractalFilter::setBaseQ (float q)
{
    baseQ = std::max (0.5f, std::min (20.0f, q));
    updateCoefficients();
}

//==============================================================================
void BandpassFractalFilter::updateCoefficients()
{
    fractalFilters.clear();

    // Calculate key-tracked base center frequency
    double trackedBase = baseCenter * (currentFreq / 100.0);  // Scale to bass reference
    trackedBase = std::max (50.0, std::min (sampleRate / 2.0 - 100.0, trackedBase));
    currentBaseCenter = static_cast<float>(trackedBase);

    // Auto-depth for musicality: Shallower for higher frequencies to avoid fizz
    int autoDepth = std::max (2, std::min (6, 4 - static_cast<int>(std::log2 (currentFreq / 200.0))));
    int effectiveDepth = (depth > 0) ? depth : autoDepth;

    // Build fractal bandpass filter chain
    double currentCenter = trackedBase;
    float currentQ = baseQ;

    for (int level = 0; level < effectiveDepth; ++level)
    {
        // Taper Q wider at deeper levels for harmonic spread and warmth
        float taperedQ = currentQ / (1.0f + level * 0.2f);

        // Create bandpass coefficients for this level
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass (
            static_cast<float>(sampleRate),
            static_cast<float>(currentCenter),
            taperedQ
        );

        // Create and configure filter
        juce::dsp::IIR::Filter<float> newFilter;
        newFilter.coefficients = *coeffs;
        fractalFilters.push_back (std::move (newFilter));

        // Scale center frequency for next level (fractal self-similarity)
        currentCenter *= scaleFactor;
        currentCenter = std::min (currentCenter, sampleRate / 2.0 - 50.0);  // Clamp to Nyquist
    }
}
