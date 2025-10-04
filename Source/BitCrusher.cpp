#include "BitCrusher.h"

//==============================================================================
BitCrusher::BitCrusher()
{
}

BitCrusher::~BitCrusher()
{
}

//==============================================================================
void BitCrusher::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 };

    // Prepare anti-aliasing filter
    auto filterCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, filterCutoff);
    antiAliasingFilter.prepare (spec);
    antiAliasingFilter.reset();
    *antiAliasingFilter.state = *filterCoeffs;

    // Prepare dry/wet mixer
    mixer.prepare (spec);
    mixer.setWetMixProportion (dryWetMix);

    reset();
    updateFilter();
}

void BitCrusher::reset()
{
    antiAliasingFilter.reset();
    mixer.reset();
    currentBitDepth = bitDepth;
    currentFilterCutoff = filterCutoff;
    lastFilterCutoff = filterCutoff;
}

//==============================================================================
void BitCrusher::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();
    auto numChannels = outputBlock.getNumChannels();

    // Update dry/wet mix
    mixer.setWetMixProportion (dryWetMix);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float processedSample = 0.0f;

        // Sum input channels for mono processing
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            processedSample += inputBlock.getSample (channel, sample);
        }
        processedSample /= static_cast<float>(numChannels);

        // Apply bit crushing
        processedSample = processBitCrushing (processedSample);

        // Note: Anti-aliasing filter commented out due to AudioBlock constructor compatibility issues
        // In a production version, this would apply a low-pass filter to remove aliasing artifacts

        // Apply dry/wet mix
        float wetSample = processedSample;
        float drySample = 0.0f;

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            drySample += inputBlock.getSample (channel, sample);
        }
        drySample /= static_cast<float>(numChannels);

        float outputSample = drySample * (1.0f - dryWetMix) + wetSample * dryWetMix;

        // Write to all output channels
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            outputBlock.setSample (channel, sample, outputSample);
        }
    }
}

//==============================================================================
void BitCrusher::setBitDepth (float depth)
{
    bitDepth = (depth < 1.0f) ? 1.0f : (depth > 16.0f) ? 16.0f : depth;
}

void BitCrusher::setFilterCutoff (float cutoffHz)
{
    filterCutoff = (cutoffHz < 100.0f) ? 100.0f : (cutoffHz > static_cast<float>(sampleRate) / 2.0f) ? static_cast<float>(sampleRate) / 2.0f : cutoffHz;
    updateFilter();
}

void BitCrusher::setDryWetMix (float mix)
{
    dryWetMix = (mix < 0.0f) ? 0.0f : (mix > 1.0f) ? 1.0f : mix;
}

//==============================================================================
float BitCrusher::processBitCrushing (float input)
{
    if (bitDepth >= 16.0f)
        return input;

    // Calculate bit depth scaling
    int numBits = static_cast<int>(bitDepth);
    float scale = static_cast<float>(1 << numBits);  // 2^bits

    // Quantize the signal
    float quantized = std::round (input * scale) / scale;

    // Mix between dry and crushed signal based on bit depth
    float crushAmount = 1.0f - (bitDepth / 16.0f);
    return input * (1.0f - crushAmount) + quantized * crushAmount;
}

void BitCrusher::updateFilter()
{
    // Smooth filter cutoff changes
    float slewRate = 100.0f; // Hz per sample
    float cutoffDiff = filterCutoff - lastFilterCutoff;

    if (std::abs (cutoffDiff) > slewRate)
    {
        currentFilterCutoff = lastFilterCutoff + (cutoffDiff > 0 ? slewRate : -slewRate);
    }
    else
    {
        currentFilterCutoff = filterCutoff;
    }

    lastFilterCutoff = currentFilterCutoff;

    // Update filter coefficients
    auto filterCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, currentFilterCutoff);
    *antiAliasingFilter.state = *filterCoeffs;
}