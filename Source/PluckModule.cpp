#include "PluckModule.h"

PluckModule::PluckModule()
{
    // Constructor
}

void PluckModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    delayLine.prepare(spec);
    filter.prepare(spec);
    reset();
}

void PluckModule::reset()
{
    delayLine.reset();
    filter.reset();
    needsToPluck = true;
}

void PluckModule::pluck()
{
    if (keyTracker == nullptr) return;

    const float freq = juce::jlimit(20.0f, 20000.0f, (float)keyTracker->getCurrentFrequency());
    const int delaySamples = static_cast<int>(sampleRate / freq);

    delayLine.setDelay(delaySamples);

    // Fill the delay line with white noise
    for (int i = 0; i < delaySamples; ++i)
    {
        delayLine.pushSample(0, juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f);
    }
    needsToPluck = false;
}

void PluckModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (needsToPluck)
        pluck();

    auto& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();
    auto numChannels = outputBlock.getNumChannels();

    for (size_t i = 0; i < numSamples; ++i)
    {
        // Get the next sample from the delay line
        float delayedSample = delayLine.popSample(0);

        // Process it through the filter
        float filteredSample = filter.processSample(delayedSample);

        // Feed it back into the delay line (with some damping)
        delayLine.pushSample(0, filteredSample * 0.99f); // Small damping factor for stability

        // Write the output
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            outputBlock.setSample(ch, i, delayedSample);
        }
    }
}

void PluckModule::setDecay (float newDecay)
{
    if (decay != newDecay)
    {
        decay = newDecay;
        updateFilter();
    }
}

void PluckModule::setDamping (float newDamping)
{
    if (damping != newDamping)
    {
        damping = newDamping;
        updateFilter();
    }
}

void PluckModule::updateFilter()
{
    // Decay controls the low-pass filter cutoff. Higher decay = darker sound = lower cutoff
    auto cutoff = juce::jmap(decay, 0.0f, 1.0f, 8000.0f, 100.0f);
    // Damping controls the Q
    auto q = juce::jmap(damping, 0.0f, 1.0f, 0.707f, 2.0f);

    *filter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoff, q);
}
