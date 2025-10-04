#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    A UI component that represents a single slot in the module chain.
*/
class ModuleSlotComponent  : public juce::Component
{
public:
    ModuleSlotComponent(WubForgeAudioProcessor& p, int index);
    ~ModuleSlotComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& event) override;

    //==============================================================================
    /** Updates the name displayed on the component. */
    void setModuleName (const juce::String& newName);

    /** Sets whether the component should be drawn as selected. */
    void setSelected (bool isNowSelected);

    // A simple callback to inform the parent component of clicks.
    std::function<void(int)> onClick;

private:
    WubForgeAudioProcessor& processor;
    const int slotIndex;

    juce::String moduleName;
    bool isSelected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleSlotComponent)
};