#include "Presets.h"

//==============================================================================
Presets::Presets()
{
    initializeDefaultPresets();
}

Presets::~Presets()
{
}

//==============================================================================
juce::String Presets::getPresetName (int index) const
{
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[index].name;
    return "Invalid Preset";
}

void Presets::loadPreset (int index, juce::AudioProcessorValueTreeState& valueTreeState)
{
    if (index >= 0 && index < static_cast<int>(presets.size()))
    {
        currentPresetIndex = index;
        const auto& preset = presets[index];

        for (const auto& param : preset.parameterValues)
        {
            auto* parameter = valueTreeState.getParameter (param.first);
            if (parameter != nullptr)
            {
                parameter->setValueNotifyingHost (param.second);
            }
        }
    }
}

void Presets::savePreset (int index, const juce::String& name, juce::AudioProcessorValueTreeState& valueTreeState)
{
    if (index >= 0 && index < static_cast<int>(presets.size()))
    {
        auto& preset = presets[index];
        preset.name = name;

        // Save current parameter values
        preset.parameterValues.clear();

        // Get all parameters from the value tree state
        for (auto* parameter : valueTreeState.getParameters())
        {
            if (auto* paramWithID = dynamic_cast<juce::RangedAudioParameter*>(parameter))
            {
                preset.parameterValues[paramWithID->getParameterID()] = paramWithID->getValue();
            }
        }
    }
}

void Presets::setPresetName (int index, const juce::String& newName)
{
    if (index >= 0 && index < static_cast<int>(presets.size()))
    {
        presets[index].name = newName;
    }
}

//==============================================================================
void Presets::initializeDefaultPresets()
{
    presets.clear();

    // Dubstep Wobble preset
    addPreset ("Dubstep Wobble",
    {
        {"combCount", 6.0f},
        {"combDelay", 2.0f},
        {"combFeedback", 0.8f},
        {"lfoRate", 0.5f},
        {"lfoDepth", 0.7f},
        {"wavefoldAmount", 0.2f},
        {"clipAmount", 0.3f},
        {"bitCrushAmount", 0.1f},
        {"formantFreq", 300.0f},
        {"keyTrackAmount", 1.0f},
        {"hpfCutoff", 40.0f},
        {"outputGain", 0.0f},
        {"dryWet", 1.0f},
        {"wobbleMode", 0.8f},
        {"hammerMode", 0.0f}
    });

    // Trap Forge preset
    addPreset ("Trap Forge",
    {
        {"combCount", 4.0f},
        {"combDelay", 1.5f},
        {"combFeedback", 0.6f},
        {"lfoRate", 2.0f},
        {"lfoDepth", 0.4f},
        {"wavefoldAmount", 0.5f},
        {"clipAmount", 0.6f},
        {"bitCrushAmount", 0.3f},
        {"formantFreq", 500.0f},
        {"keyTrackAmount", 1.2f},
        {"hpfCutoff", 60.0f},
        {"outputGain", -3.0f},
        {"dryWet", 1.0f},
        {"wobbleMode", 0.3f},
        {"hammerMode", 0.0f}
    });

    // Neurofunk preset
    addPreset ("Neurofunk",
    {
        {"combCount", 8.0f},
        {"combDelay", 1.0f},
        {"combFeedback", 0.9f},
        {"lfoRate", 1.5f},
        {"lfoDepth", 0.8f},
        {"wavefoldAmount", 0.4f},
        {"clipAmount", 0.5f},
        {"bitCrushAmount", 0.2f},
        {"formantFreq", 350.0f},
        {"keyTrackAmount", 1.5f},
        {"hpfCutoff", 50.0f},
        {"outputGain", -2.0f},
        {"dryWet", 1.0f},
        {"wobbleMode", 0.6f},
        {"hammerMode", 1.0f}
    });

    // Bass House preset
    addPreset ("Bass House",
    {
        {"combCount", 5.0f},
        {"combDelay", 3.0f},
        {"combFeedback", 0.7f},
        {"lfoRate", 0.8f},
        {"lfoDepth", 0.5f},
        {"wavefoldAmount", 0.3f},
        {"clipAmount", 0.4f},
        {"bitCrushAmount", 0.1f},
        {"formantFreq", 400.0f},
        {"keyTrackAmount", 0.8f},
        {"hpfCutoff", 35.0f},
        {"outputGain", 1.0f},
        {"dryWet", 1.0f},
        {"wobbleMode", 0.7f},
        {"hammerMode", 0.0f}
    });
}

void Presets::addPreset (const juce::String& name, const std::map<juce::String, float>& params)
{
    PresetData preset;
    preset.name = name;
    preset.parameterValues = params;
    presets.push_back (preset);
}
