#pragma once

#include <juce_dsp/juce_dsp.h>
#include "KeyTracker.h"

// Defines the signal routing configuration for the module chain
enum class Routing
{
    Serial,
    Parallel,
    MidSide,
    Feedback
};

//==============================================================================
/**
    Abstract base class for a single audio processing module in the chain.
*/
class AudioModule
{
public:
    virtual ~AudioModule() = default;

    // Enum to identify module types
    enum class ModuleType { Filter, Distortion };

    // Pure virtual functions that must be implemented by derived modules
    virtual void prepare (const juce::dsp::ProcessSpec& spec) = 0;
    virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) = 0;
    virtual void reset() = 0;
    
    virtual const juce::String getName() const = 0;
    virtual ModuleType getType() const = 0;

    // Optional: for modules that need key tracking info
    virtual void setKeyTracker (KeyTracker* tracker) { keyTracker = tracker; }

protected:
    KeyTracker* keyTracker = nullptr;
};

//==============================================================================
/**
    Base class for filter-type modules.
*/
class FilterModule : public AudioModule
{
public:
    ModuleType getType() const override { return ModuleType::Filter; }
};

/**
    Base class for distortion-type modules.
*/
class DistortionModule : public AudioModule
{
public:
    ModuleType getType() const override { return ModuleType::Distortion; }
};
