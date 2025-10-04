#include "SampleMorpher.h"

//==============================================================================
SampleMorpher::SampleMorpher()
{
    // Initialize unique_ptr to nullptr
    sampleReader.reset();

    // Initialize grain processing buffers
    grainWindow.resize (fftSize);
    analysisBuffer.resize (fftSize * 2);  // For overlap-add
    synthesisBuffer.resize (fftSize * 2);

    // Create Hanning window for smooth grains
    for (int i = 0; i < fftSize; ++i)
    {
        float phase = (float)i / (float)(fftSize - 1);
        grainWindow[i] = 0.5f * (1.0f - std::cos (2.0f * juce::MathConstants<float>::pi * phase));
    }
}

SampleMorpher::~SampleMorpher()
{
    unloadSample();
}

//==============================================================================
void SampleMorpher::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;
    currentBlockSize = spec.maximumBlockSize;

    // Prepare envelope
    envelope.setSampleRate (spec.sampleRate);
    updateGrainParameters();

    // Reset processing state
    currentGrainPosition = 0.0f;
    writePosition = 0;

    // Clear buffers
    std::fill (analysisBuffer.begin(), analysisBuffer.end(), 0.0f);
    std::fill (synthesisBuffer.begin(), synthesisBuffer.end(), 0.0f);
}

void SampleMorpher::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto&& inputBlock = context.getInputBlock();
    auto&& outputBlock = context.getOutputBlock();
    auto numSamples = (int)inputBlock.getNumSamples();
    auto numChannels = (int)inputBlock.getNumChannels();

    // Process envelope for modulation
    envelope.setParameters (juce::ADSR::Parameters (attackTime, attackTime, 1.0f, releaseTime));

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* inputSamples = inputBlock.getChannelPointer (channel);
        auto* outputSamples = outputBlock.getChannelPointer (channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float inputSample = inputSamples[sample];

            // Trigger envelope on transients for modulation
            float envelopeValue = envelope.getNextSample();
            if (std::abs (inputSample) > 0.1f && !envelope.isActive())
                envelope.noteOn();

            // Process grain synthesis
            float morphedSample = inputSample;

            if (sampleLoaded && morphAmount > 0.0f)
            {
                // Get grain from sample
                float sampleGrain = 0.0f;

                // Simple granular playback for now - will enhance with FFT morphing
                if (sampleBuffer.getNumSamples() > 0)
                {
                    // Key-tracked grain rate for bass response
                    float grainRate = getKeyTrackedGrainRate();

                    // Advance grain position
                    currentGrainPosition += grainRate;
                    if (currentGrainPosition >= sampleBuffer.getNumSamples())
                        currentGrainPosition -= sampleBuffer.getNumSamples();

                    // Get sample at current position
                    int readPos = (int)currentGrainPosition;
                    if (readPos < sampleBuffer.getNumSamples())
                    {
                        sampleGrain = sampleBuffer.getSample (0, readPos);  // Use first channel
                    }

                    // Apply envelope modulation to position
                    applyEnvelopeModulation();
                }

                // Morph between input and sample grain
                morphedSample = (1.0f - morphAmount) * inputSample + morphAmount * sampleGrain;
            }

            outputSamples[sample] = morphedSample;

            // Auto-release envelope
            if (envelopeValue < 0.001f && envelope.isActive())
                envelope.noteOff();
        }
    }
}

void SampleMorpher::reset()
{
    currentGrainPosition = 0.0f;
    writePosition = 0;
    envelope.reset();

    std::fill (analysisBuffer.begin(), analysisBuffer.end(), 0.0f);
    std::fill (synthesisBuffer.begin(), synthesisBuffer.end(), 0.0f);
}

//==============================================================================
bool SampleMorpher::loadSample (const juce::File& file)
{
    unloadSample();

    // For now, return false as sample loading needs JUCE API compatibility fix
    // TODO: Implement proper sample loading with correct JUCE API
    juce::Logger::writeToLog ("SampleMorpher: Sample loading not yet implemented - needs JUCE API compatibility fix");

    return false;
}

void SampleMorpher::unloadSample()
{
    sampleReader.reset();
    sampleBuffer.clear();
    sampleLoaded = false;
    loadedSampleName = "";
}

//==============================================================================
void SampleMorpher::setMorphAmount (float amount)
{
    morphAmount = juce::jlimit (0.0f, 1.0f, amount);
}

void SampleMorpher::setGrainSize (int samples)
{
    grainSize = juce::jlimit (128, 2048, samples);
    updateGrainParameters();
}

void SampleMorpher::setGrainOverlap (float overlap)
{
    grainOverlap = juce::jlimit (0.0f, 0.75f, overlap);
    updateGrainParameters();
}

void SampleMorpher::setPositionMod (float modAmount)
{
    positionModAmount = juce::jlimit (0.0f, 1.0f, modAmount);
}

void SampleMorpher::setKeyTrackAmount (float amount)
{
    keyTrackAmount = juce::jlimit (0.0f, 1.0f, amount);
}

//==============================================================================
void SampleMorpher::setAttackTime (float seconds)
{
    attackTime = juce::jlimit (0.001f, 1.0f, seconds);
}

void SampleMorpher::setReleaseTime (float seconds)
{
    releaseTime = juce::jlimit (0.01f, 2.0f, seconds);
}

void SampleMorpher::setEnvelopeDepth (float depth)
{
    envelopeDepth = juce::jlimit (0.0f, 1.0f, depth);
}

//==============================================================================
float SampleMorpher::getSampleLengthSeconds() const
{
    if (sampleLoaded && sampleReader != nullptr)
        return (float)sampleReader->lengthInSamples / (float)currentSampleRate;
    return 0.0f;
}

//==============================================================================
void SampleMorpher::updateGrainParameters()
{
    // Calculate grain position increment based on key tracking
    float baseIncrement = (float)hopSize / (float)currentSampleRate;
    grainPositionIncrement = baseIncrement * getKeyTrackedGrainRate();
}

float SampleMorpher::getKeyTrackedGrainRate()
{
    if (keyTracker == nullptr)
        return 1.0f;

    float currentFreq = keyTracker->getCurrentFrequency();
    if (currentFreq <= 0.0f)
        return 1.0f;

    // Map frequency to grain rate (lower freq = slower grains for bass)
    float freqRatio = currentFreq / baseFrequency;
    return 1.0f / std::pow (freqRatio, keyTrackAmount * 0.5f);
}

void SampleMorpher::applyEnvelopeModulation()
{
    if (envelope.isActive())
    {
        float envValue = envelope.getNextSample();
        float modulation = 1.0f + (envValue * envelopeDepth * positionModAmount * 0.1f);
        grainPositionIncrement *= modulation;
    }
}
