#include "FormantTrackerModule.h"

//==============================================================================
FormantTrackerModule::FormantTrackerModule()
{
}

FormantTrackerModule::~FormantTrackerModule()
{
}

//==============================================================================
void FormantTrackerModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    formantTracker.prepareToPlay (spec.sampleRate, static_cast<int>(spec.maximumBlockSize));
}

void FormantTrackerModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    // Update current frequency from key tracker if available
    if (keyTracker != nullptr)
    {
        formantTracker.setCurrentFrequency (keyTracker->getCurrentFrequency());
    }
    
    // Create a non-const context for the formant tracker
    auto& block = const_cast<juce::dsp::AudioBlock<float>&>(context.getInputBlock());
    juce::dsp::ProcessContextReplacing<float> nonConstContext (block);
    formantTracker.process (nonConstContext);
}

void FormantTrackerModule::reset()
{
    formantTracker.reset();
}

//==============================================================================
void FormantTrackerModule::setKeyTrackAmount (float amount)
{
    formantTracker.setKeyTrackAmount (amount);
}

void FormantTrackerModule::setFormantGain (float gainDb)
{
    formantTracker.setFormantGain (gainDb);
}

void FormantTrackerModule::setFormantQ (float Q)
{
    formantTracker.setFormantQ (Q);
}

void FormantTrackerModule::setBaseFormants (const std::array<double, 3>& formants)
{
    formantTracker.setBaseFormants (formants);
}

void FormantTrackerModule::setBaseFrequency (double baseFreq)
{
    formantTracker.setBaseFrequency (baseFreq);
}

//==============================================================================
float FormantTrackerModule::getKeyTrackAmount() const
{
    return formantTracker.getKeyTrackAmount();
}

float FormantTrackerModule::getFormantGain() const
{
    return formantTracker.getFormantGain();
}

float FormantTrackerModule::getFormantQ() const
{
    return formantTracker.getFormantQ();
}

std::array<double, 3> FormantTrackerModule::getCurrentFormantFrequencies() const
{
    return formantTracker.getCurrentFormantFrequencies();
}
