#include "ChowEQModule.h"

//==============================================================================
ChowEQModule::ChowEQModule()
{
    // Initialize EQ bands with default settings
    // Low band: Low shelf filter around 200Hz
    lowBand.setType(chowdsp::EQ::EQBand<float>::Type::LowShelf);
    lowBand.setFreq(200.0f);
    lowBand.setQ(0.707f);  // Butterworth Q
    lowBand.setGain(0.0f);

    // Mid band: Peak filter, frequency will be set dynamically
    midBand.setType(chowdsp::EQ::EQBand<float>::Type::Peak);
    midBand.setFreq(midFrequency);
    midBand.setQ(1.414f);  // Moderate Q
    midBand.setGain(0.0f);

    // High band: High shelf filter around 5kHz
    highBand.setType(chowdsp::EQ::EQBand<float>::Type::HighShelf);
    highBand.setFreq(5000.0f);
    highBand.setQ(0.707f);  // Butterworth Q
    highBand.setGain(0.0f);
}

//==============================================================================
void ChowEQModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;

    // Prepare all EQ bands
    lowBand.prepare(spec);
    midBand.prepare(spec);
    highBand.prepare(spec);

    // Reset to ensure clean state
    reset();
}

void ChowEQModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    // Process through each EQ band in series
    lowBand.process(context);
    midBand.process(context);
    highBand.process(context);
}

void ChowEQModule::reset()
{
    lowBand.reset();
    midBand.reset();
    highBand.reset();
}

//==============================================================================
void ChowEQModule::setMidFreq(float frequencyHz)
{
    midFrequency = std::max(200.0f, std::min(8000.0f, frequencyHz));
    midBand.setFreq(midFrequency);
    midBand.reset();  // Reset to apply new frequency
}
