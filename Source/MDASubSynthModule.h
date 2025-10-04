#pragma once

#include <JuceHeader.h>
#include "Module.h"
#include <memory>

// Include the MDA SubSynth processor
// Note: This assumes the MDA plugin headers are available
class MDASubSynthAudioProcessor;

class MDASubSynthModule : public AudioModule
{
public:
    MDASubSynthModule();
    ~MDASubSynthModule() override = default;

    //==============================================================================
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    //==============================================================================
    const juce::String getName() const override { return "MDA SubSynth"; }
    ModuleType getType() const override { return ModuleType::Effect; }

    //==============================================================================
    // SubSynth-specific controls
    void setType(int type);  // 0=Sine, 1=Square, 2=Bass
    void setWetMix(float wet);  // Amount of sub-bass addition
    void setDryMix(float dry);  // Amount of original signal reduction
    void setThreshold(float threshold);  // Trigger threshold
    void setDecay(float decay);  // Envelope decay for Key Osc mode

private:
    //==============================================================================
    // Internal MDA processor
    std::unique_ptr<MDASubSynthAudioProcessor> subSynthProcessor;

    // Current parameter values
    int currentType = 0;
    float currentWet = 0.5f;
    float currentDry = 1.0f;
    float currentThreshold = 0.5f;
    float currentDecay = 0.5f;

    //==============================================================================
    // Parameter mapping helpers
    void updateParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDASubSynthModule)
};
