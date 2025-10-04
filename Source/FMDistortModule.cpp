#include "FMDistortModule.h"

FMDistortModule::FMDistortModule()
{
    // Constructor
}

void FMDistortModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    phase = 0.0f;
    updatePhaseDelta();
}

void FMDistortModule::reset()
{
    phase = 0.0f;
}

void FMDistortModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();
    auto numChannels = outputBlock.getNumChannels();

    // Recalculate phase delta in case key tracking has changed
    updatePhaseDelta();

    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* inputSamples = inputBlock.getChannelPointer(channel);
        auto* outputSamples = outputBlock.getChannelPointer(channel);

        for (size_t i = 0; i < numSamples; ++i)
        {
            const float inputSample = inputSamples[i];
            
            // Modulate the phase with the input signal
            const float modulatedPhase = phase + (inputSample * modIndex);
            
            // Generate the output sample from our oscillator
            const float outputSample = std::sin (modulatedPhase);
            
            outputSamples[i] = outputSample;

            // Advance the main phase
            phase += phaseDelta;
        }
    }

    // Keep phase wrapped to prevent it from growing indefinitely
    phase = std::fmod(phase, juce::MathConstants<float>::twoPi);
}

void FMDistortModule::setRatio (float ratio)
{
    if (ratio != frequencyRatio)
    {
        frequencyRatio = ratio;
        updatePhaseDelta();
    }
}

void FMDistortModule::setModIndex (float index)
{
    modIndex = index;
}

void FMDistortModule::updatePhaseDelta()
{
    if (keyTracker != nullptr)
    {
        auto currentFreq = keyTracker->getCurrentFrequency();
        auto targetFreq = juce::jlimit(20.0, 20000.0, currentFreq * frequencyRatio);
        phaseDelta = (targetFreq * juce::MathConstants<float>::twoPi) / static_cast<float>(sampleRate);
    }
    else
    {
        phaseDelta = 0.0f;
    }
}
