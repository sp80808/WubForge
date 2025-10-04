#include "RatDistortionModule.h"

RatDistortionModule::RatDistortionModule()
{
    // Constructor
}

void RatDistortionModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    inputGain.prepare(spec);
    toneFilter.prepare(spec);
    outputGain.prepare(spec);

    reset();
}

void RatDistortionModule::reset()
{
    inputGain.reset();
    toneFilter.reset();
    outputGain.reset();
    updateToneFilter();
}

void RatDistortionModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    // 1. Apply input gain (Drive)
    inputGain.process(context);

    // 2. Apply hard-clipping distortion
    auto& block = context.getOutputBlock();
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* samples = block.getChannelPointer(channel);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            samples[i] = juce::jlimit(-1.0f, 1.0f, samples[i]);
        }
    }

    // 3. Apply tone-shaping filter
    toneFilter.process(context);

    // 4. Apply output gain (Level)
    outputGain.process(context);
}

void RatDistortionModule::setDrive (float newDrive)
{
    drive = newDrive;
    // Map 0-1 drive to a 0dB to +40dB gain range
    inputGain.setGainDecibels(drive * 40.0f);
}

void RatDistortionModule::setTone (float newTone)
{
    if (tone != newTone)
    {
        tone = newTone;
        updateToneFilter();
    }
}

void RatDistortionModule::setLevel (float newLevel)
{
    level = newLevel;
    // Map 0-1 level to a -20dB to 0dB gain range
    outputGain.setGainDecibels(juce::jmap(level, 0.0f, 1.0f, -20.0f, 0.0f));
}

void RatDistortionModule::updateToneFilter()
{
    // The RAT's tone knob is a reverse low-pass filter.
    // High tone value = brighter sound (higher cutoff).
    // We map the 0-1 tone parameter to a logarithmic frequency range.
    auto cutoffHz = juce::jmap(tone, 0.0f, 1.0f, 20000.0f, 500.0f);

    *toneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffHz, 0.707f);
}
