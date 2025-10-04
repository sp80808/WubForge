#include "ModuleSlotComponent.h"

//==============================================================================
ModuleSlotComponent::ModuleSlotComponent(WubForgeAudioProcessor& p, int index)
    : processor(p), slotIndex(index)
{
    setModuleName("[Empty]");
}

ModuleSlotComponent::~ModuleSlotComponent()
{
}

void ModuleSlotComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().reduced(2);
    auto cornerSize = 8.0f;

    // Background
    g.setColour(juce::Colours::darkgrey.darker(0.5f));
    g.fillRoundedRectangle(area.toFloat(), cornerSize);

    // Border
    if (isSelected)
    {
        g.setColour(juce::Colours::cyan);
        g.drawRoundedRectangle(area.toFloat(), cornerSize, 2.0f);
    }
    else
    {
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRoundedRectangle(area.toFloat(), cornerSize, 1.0f);
    }

    // Text
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(moduleName, area, juce::Justification::centred, 1);
}

void ModuleSlotComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains...
}

void ModuleSlotComponent::mouseDown(const juce::MouseEvent& event)
{
    if (onClick)
    {
        onClick(slotIndex);
    }
}

void ModuleSlotComponent::setModuleName(const juce::String& newName)
{
    if (moduleName != newName)
    { 
        moduleName = newName;
        repaint();
    }
}

void ModuleSlotComponent::setSelected(bool isNowSelected)
{
    if (isSelected != isNowSelected)
    {
        isSelected = isNowSelected;
        repaint();
    }
}
