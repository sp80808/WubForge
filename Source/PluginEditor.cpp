#include "PluginEditor.h"

//==============================================================================
WubForgeAudioProcessorEditor::WubForgeAudioProcessorEditor (WubForgeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);

    // Simple text labels for alpha testing
    addAndMakeVisible (titleLabel);
    titleLabel.setText ("WUBFORGE ALPHA", juce::dontSendNotification);
    titleLabel.setFont (titleLabel.getFont().withHeight (24.0f));
    titleLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (infoLabel);
    infoLabel.setText ("Spectral Audio Effect\nAlpha Test Build", juce::dontSendNotification);
    infoLabel.setJustificationType (juce::Justification::centred);
}

WubForgeAudioProcessorEditor::~WubForgeAudioProcessorEditor() = default;

//==============================================================================
void WubForgeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (45, 48, 51));
}

void WubForgeAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    titleLabel.setBounds (bounds.removeFromTop (100).reduced (20));
    infoLabel.setBounds (bounds.reduced (20));
}
