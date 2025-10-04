#pragma once

#include "Module.h"
#include "CombStack.h"
#include "DistortionForge.h"
#include "FormantTracker.h"

//==============================================================================
/**
    A wrapper to make the original CombStack class compatible with the new
    AudioModule interface.
*/
class CombStackModule : public FilterModule
{
public:
    CombStackModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        internalCombStack.prepareToPlay (spec.sampleRate, spec.maximumBlockSize);
        if (keyTracker != nullptr)
        {
            // Example of how a module can access the global key tracker
            // internalCombStack.setKeyTrackFrequency(keyTracker->getCurrentFrequency());
        }
    }

    void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        // Create a non-const context for the comb stack
        auto& block = const_cast<juce::dsp::AudioBlock<float>&>(context.getInputBlock());
        juce::dsp::ProcessContextReplacing<float> nonConstContext (block);
        internalCombStack.process (nonConstContext);
    }

    void reset() override
    {
        internalCombStack.reset();
    }

    const juce::String getName() const override { return "Comb Stack"; }

    // Expose internal object for parameter setting
    CombStack& getInternalProcessor() { return internalCombStack; }

private:
    CombStack internalCombStack;
};

//==============================================================================
/**
    A wrapper for the FormantTracker class.
*/
class FormantTrackerModule : public FilterModule
{
public:
    FormantTrackerModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        internalFormantTracker.prepareToPlay (spec.sampleRate, spec.maximumBlockSize);
    }

    void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        // The tracker needs the current frequency to function
        if (keyTracker != nullptr)
            internalFormantTracker.setCurrentFrequency (keyTracker->getCurrentFrequency());
        
        // Create a non-const context for the formant tracker
        auto& block = const_cast<juce::dsp::AudioBlock<float>&>(context.getInputBlock());
        juce::dsp::ProcessContextReplacing<float> nonConstContext (block);
        internalFormantTracker.process (nonConstContext);
    }

    void reset() override
    {
        internalFormantTracker.reset();
    }

    const juce::String getName() const override { return "Formant Tracker"; }

    FormantTracker& getInternalProcessor() { return internalFormantTracker; }

private:
    FormantTracker internalFormantTracker;
};

//==============================================================================
/**
    A wrapper for the DistortionForge class.
*/
class DistortionForgeModule : public DistortionModule
{
public:
    DistortionForgeModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        internalDistortionForge.prepareToPlay (spec.sampleRate, spec.maximumBlockSize);
    }

    void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        // Create a non-const context for the distortion forge
        auto& block = const_cast<juce::dsp::AudioBlock<float>&>(context.getInputBlock());
        juce::dsp::ProcessContextReplacing<float> nonConstContext (block);
        internalDistortionForge.process (nonConstContext);
    }

    void reset() override
    {
        internalDistortionForge.reset();
    }

    const juce::String getName() const override { return "Distortion Forge"; }

    DistortionForge& getInternalProcessor() { return internalDistortionForge; }

private:
    DistortionForge internalDistortionForge;
};
