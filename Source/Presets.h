#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
class Presets
{
public:
    Presets();
    ~Presets();

    //==============================================================================
    int getNumPresets() const { return presets.size(); }
    juce::String getPresetName (int index) const;
    int getCurrentPresetIndex() const { return currentPresetIndex; }

    //==============================================================================
    void loadPreset (int index, juce::AudioProcessorValueTreeState& valueTreeState);
    void savePreset (int index, const juce::String& name, juce::AudioProcessorValueTreeState& valueTreeState);
    void setPresetName (int index, const juce::String& newName);

    //==============================================================================
    void initializeDefaultPresets();

private:
    //==============================================================================
    struct PresetData
    {
        juce::String name;
        std::map<juce::String, float> parameterValues;
    };

    std::vector<PresetData> presets;
    int currentPresetIndex = 0;

    //==============================================================================
    void addPreset (const juce::String& name, const std::map<juce::String, float>& params);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Presets)
};
