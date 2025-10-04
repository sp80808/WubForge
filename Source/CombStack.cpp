#include "CombStack.h"

//==============================================================================
CombStack::CombStack()
{
    // Initialize phase offsets for each comb filter
    for (int i = 0; i < maxCombCount; ++i)
    {
        phaseOffsets[i] = static_cast<float>(i) / static_cast<float>(maxCombCount) * juce::MathConstants<float>::twoPi;
    }
}

CombStack::~CombStack()
{
}

//==============================================================================
void CombStack::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 };

    // Prepare delay lines
    for (auto& delayLine : delayLines)
    {
        delayLine.prepare (spec);
        delayLine.setMaximumDelayInSamples (static_cast<size_t>(sampleRate * 0.1f)); // Max 100ms
    }

    // Prepare LFO
    lfo.initialise ([] (float x) { return std::sin (x); });
    lfo.prepare (spec);
    lfo.setFrequency (lfoRate);

    reset();
    updateDelayTimes();
    updateLfo();
}

void CombStack::reset()
{
    for (auto& delayLine : delayLines)
    {
        delayLine.reset();
    }

    lfo.reset();
    lfoPhase = 0.0f;
    currentDelayTime = baseDelayTime;
    lastDelayTime = baseDelayTime;
}

//==============================================================================
void CombStack::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();
    auto numChannels = outputBlock.getNumChannels();

    // Clear output block initially
    outputBlock.clear();

    // Update LFO phase
    lfoPhaseIncrement = lfoRate * juce::MathConstants<float>::twoPi / static_cast<float>(sampleRate);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Update LFO
        lfoPhase += lfoPhaseIncrement;
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        float lfoValue = std::sin (lfoPhase) * lfoDepth;

        // Calculate base delay time with key tracking
        float keyTrackedDelay = baseDelayTime;

        // Apply key tracking if enabled
        if (keyTrackAmount > 0.0f)
        {
            // Calculate pitch ratio relative to A4 (440 Hz)
            float pitchRatio = keyTrackFrequency / 440.0f;

            // Apply logarithmic scaling for more natural delay scaling
            // Higher notes = shorter delay (like in the guide example)
            float keyTrackedDelayTime = baseDelayTime / pitchRatio;

            // Blend between base delay and key-tracked delay
            keyTrackedDelay = keyTrackedDelayTime * keyTrackAmount + baseDelayTime * (1.0f - keyTrackAmount);
        }

        // Apply LFO modulation to the key-tracked delay
        float modulatedDelay = keyTrackedDelay * (1.0f + lfoValue);

        // Smooth delay time changes to avoid artifacts
        float delaySlew = 0.001f; // Adjust for desired slew rate
        if (std::abs (modulatedDelay - lastDelayTime) > delaySlew)
        {
            modulatedDelay = lastDelayTime + (modulatedDelay > lastDelayTime ? delaySlew : -delaySlew);
        }
        lastDelayTime = modulatedDelay;

        // Convert to samples
        float delaySamples = modulatedDelay * 0.001f * static_cast<float>(sampleRate);

        // Process each comb filter
        float combOutput = 0.0f;

        for (int comb = 0; comb < combCount; ++comb)
        {
            // Add phase offset for each comb
            float combPhase = lfoPhase + phaseOffsets[comb];
            if (combPhase >= juce::MathConstants<float>::twoPi)
                combPhase -= juce::MathConstants<float>::twoPi;

            float combLfoValue = std::sin (combPhase) * lfoDepth;
            float combDelay = delaySamples * (1.0f + combLfoValue * 0.1f); // Slight spread

            // Clamp delay to valid range
            combDelay = juce::jlimit (1.0f, static_cast<float>(sampleRate * 0.1f), combDelay);

            delayLines[comb].setDelay (combDelay);

            // Process this comb filter
            float inputSample = 0.0f;
            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                inputSample += inputBlock.getSample (channel, sample);
            }
            inputSample /= static_cast<float>(numChannels);

            float delayedSample = delayLines[comb].popSample (0);
            float outputSample = inputSample + delayedSample * feedback;

            delayLines[comb].pushSample (0, outputSample);

            combOutput += outputSample;
        }

        // Average the comb outputs
        combOutput /= static_cast<float>(combCount);

        // Apply mode blend (0 = full wobble, 1 = full forge)
        float blendFactor = modeBlend;
        combOutput *= (1.0f - blendFactor);

        // Write to all output channels
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            float inputSample = inputBlock.getSample (channel, sample);
            float outputSample = inputSample * (1.0f - blendFactor) + combOutput * blendFactor;
            outputBlock.setSample (channel, sample, outputSample);
        }
    }
}

//==============================================================================
void CombStack::setCombCount (int count)
{
    combCount = juce::jlimit (1, maxCombCount, count);
    updateDelayTimes();
}

void CombStack::setDelayTime (float delayMs)
{
    baseDelayTime = juce::jlimit (0.1f, 10.0f, delayMs);
    updateDelayTimes();
}

void CombStack::setFeedback (float feedbackAmount)
{
    feedback = juce::jlimit (0.0f, 0.95f, feedbackAmount);
}

void CombStack::setLfoRate (float rateHz)
{
    lfoRate = juce::jlimit (0.1f, 20.0f, rateHz);
    updateLfo();
}

void CombStack::setLfoDepth (float depth)
{
    lfoDepth = juce::jlimit (0.0f, 1.0f, depth);
}

void CombStack::setModeBlend (float blend)
{
    modeBlend = juce::jlimit (0.0f, 1.0f, blend);
}

//==============================================================================
void CombStack::setKeyTrackFrequency (float frequency)
{
    keyTrackFrequency = juce::jmax (20.0f, frequency);  // Minimum 20 Hz
}

void CombStack::setKeyTrackAmount (float amount)
{
    keyTrackAmount = juce::jlimit (0.0f, 1.0f, amount);
}

//==============================================================================
void CombStack::updateDelayTimes()
{
    // Update delay times for all comb filters
    for (int i = 0; i < maxCombCount; ++i)
    {
        delayTimes[i] = baseDelayTime * (1.0f + i * 0.1f); // Slight spread
    }
}

void CombStack::updateLfo()
{
    // LFO is updated in process() method for real-time changes
}
