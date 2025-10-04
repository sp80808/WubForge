#include "DistortionForge.h"

//==============================================================================
DistortionForge::DistortionForge()
{
}

DistortionForge::~DistortionForge()
{
}

//==============================================================================
void DistortionForge::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 };

    // Prepare formant filter
    auto formantCoeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, formantFreq, 0.5f);
    formantFilter.prepare (spec);
    formantFilter.reset();
    *formantFilter.state = *formantCoeffs;

    // Prepare gain stages
    inputGain.prepare (spec);
    inputGain.setRampDurationSeconds (0.05);
    inputGain.setGainLinear (1.0f);

    outputGain.prepare (spec);
    outputGain.setRampDurationSeconds (0.05);
    outputGain.setGainLinear (1.0f);

    reset();
    updateFormantFilter();
}

void DistortionForge::reset()
{
    formantFilter.reset();
    inputGain.reset();
    outputGain.reset();
    currentFormantFreq = formantFreq;
    lastFormantFreq = formantFreq;
    hammerCounter = 0;
}

//==============================================================================
void DistortionForge::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();
    auto numChannels = outputBlock.getNumChannels();

    // Handle hammer mode randomization
    if (hammerMode)
    {
        hammerCounter++;
        if (hammerCounter >= hammerInterval)
        {
            hammerCounter = 0;
            // Randomize distortion parameters slightly for chaotic effect
            wavefoldAmount += (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
            clipAmount += (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.1f;
            wavefoldAmount = juce::jlimit (0.0f, 1.0f, wavefoldAmount);
            clipAmount = juce::jlimit (0.0f, 1.0f, clipAmount);
        }
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float processedSample = 0.0f;

        // Sum input channels
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            processedSample += inputBlock.getSample (channel, sample);
        }
        processedSample /= static_cast<float>(numChannels);

        // Apply distortion chain
        processedSample = processWavefolding (processedSample);
        processedSample = processAsymmetricClipping (processedSample);
        processedSample = processBitCrushing (processedSample);

        // Apply mode blend (0 = full wobble, 1 = full forge)
        float blendFactor = modeBlend;
        processedSample *= blendFactor;

        // Write to all output channels
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            float inputSample = inputBlock.getSample (channel, sample);
            float outputSample = inputSample * (1.0f - blendFactor) + processedSample;
            outputBlock.setSample (channel, sample, outputSample);
        }
    }
}

//==============================================================================
void DistortionForge::setWavefoldAmount (float amount)
{
    wavefoldAmount = juce::jlimit (0.0f, 1.0f, amount);
}

void DistortionForge::setClipAmount (float amount)
{
    clipAmount = juce::jlimit (0.0f, 1.0f, amount);
}

void DistortionForge::setBitCrushAmount (float amount)
{
    bitCrushAmount = juce::jlimit (0.0f, 1.0f, amount);
}

void DistortionForge::setFormantFreq (float freqHz)
{
    formantFreq = juce::jlimit (100.0f, 2000.0f, freqHz);
    updateFormantFilter();
}

void DistortionForge::setHammerMode (bool enabled)
{
    hammerMode = enabled;
}

void DistortionForge::setModeBlend (float blend)
{
    modeBlend = juce::jlimit (0.0f, 1.0f, blend);
}

//==============================================================================
void DistortionForge::setKeyTrackFrequency (float frequency)
{
    keyTrackFrequency = std::max (20.0f, frequency);  // Minimum 20 Hz
}

void DistortionForge::setKeyTrackAmount (float amount)
{
    keyTrackAmount = std::clamp (amount, 0.0f, 1.0f);
}

//==============================================================================
float DistortionForge::processWavefolding (float input)
{
    if (wavefoldAmount <= 0.0f)
        return input;

    float threshold = 0.5f + wavefoldAmount * 0.5f;
    float gain = 1.0f + wavefoldAmount * 2.0f;

    float amplified = input * gain;

    // Wavefolding algorithm
    float folded = 0.0f;
    if (std::abs (amplified) > threshold)
    {
        float excess = std::abs (amplified) - threshold;
        float foldedExcess = threshold - excess;
        folded = (amplified > 0) ? foldedExcess : -foldedExcess;
    }
    else
    {
        folded = amplified;
    }

    // Mix between dry and folded signal
    return input * (1.0f - wavefoldAmount) + folded * wavefoldAmount;
}

float DistortionForge::processAsymmetricClipping (float input)
{
    if (clipAmount <= 0.0f)
        return input;

    float thresholdPos = 0.7f + clipAmount * 0.3f;  // Positive threshold
    float thresholdNeg = -0.5f - clipAmount * 0.3f; // Negative threshold (more aggressive)

    float output = input;

    // Asymmetric clipping
    if (output > thresholdPos)
    {
        output = thresholdPos + (output - thresholdPos) * 0.3f; // Soft clip positive
    }
    else if (output < thresholdNeg)
    {
        output = thresholdNeg + (output - thresholdNeg) * 0.1f; // Hard clip negative
    }

    // Mix between dry and clipped signal
    return input * (1.0f - clipAmount) + output * clipAmount;
}

float DistortionForge::processBitCrushing (float input)
{
    if (bitCrushAmount <= 0.0f)
        return input;

    // Bit crushing reduces bit depth
    int bitDepth = static_cast<int>(16 - bitCrushAmount * 12); // 16 to 4 bits
    bitDepth = juce::jlimit (4, 16, bitDepth);

    float scale = static_cast<float>(1 << bitDepth);
    float crushed = std::round (input * scale) / scale;

    // Mix between dry and crushed signal
    return input * (1.0f - bitCrushAmount) + crushed * bitCrushAmount;
}

void DistortionForge::updateFormantFilter()
{
    // Calculate key-tracked formant frequency
    float keyTrackedFormantFreq = formantFreq;

    if (keyTrackAmount > 0.0f)
    {
        // Calculate pitch ratio relative to A4 (440 Hz)
        float pitchRatio = keyTrackFrequency / 440.0f;

        // Apply logarithmic scaling for more natural formant shifting
        // Higher notes = higher formant frequencies (more "brightness")
        float keyTrackedFreq = formantFreq * pitchRatio;

        // Blend between base formant frequency and key-tracked frequency
        keyTrackedFormantFreq = keyTrackedFreq * keyTrackAmount + formantFreq * (1.0f - keyTrackAmount);
    }

    // Smooth formant frequency changes
    float slewRate = 10.0f; // Hz per sample
    float freqDiff = keyTrackedFormantFreq - lastFormantFreq;

    if (std::abs (freqDiff) > slewRate)
    {
        currentFormantFreq = lastFormantFreq + (freqDiff > 0 ? slewRate : -slewRate);
    }
    else
    {
        currentFormantFreq = keyTrackedFormantFreq;
    }

    lastFormantFreq = currentFormantFreq;

    // Update filter coefficients
    auto formantCoeffs = juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, currentFormantFreq, 0.5f);
    *formantFilter.state = *formantCoeffs;
}
