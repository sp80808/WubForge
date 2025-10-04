#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    WubForgeAudioProcessorEditor - Simple GUI for alpha testing

    Minimal implementation for testing the audio processing engine.
*/
class WubForgeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    WubForgeAudioProcessorEditor (WubForgeAudioProcessor&);
    ~WubForgeAudioProcessorEditor() override; // This is correct, it overrides juce::AudioProcessorEditor's destructor

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    WubForgeAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label infoLabel;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WubForgeAudioProcessorEditor)
};
