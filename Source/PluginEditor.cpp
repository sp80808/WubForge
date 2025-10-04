#include "PluginEditor.h"

//==============================================================================
WubForgeAudioProcessorEditor::WubForgeAudioProcessorEditor (WubForgeAudioProcessor& p)
    : MagicPluginEditor (&p, juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->DPI),
      audioProcessor (p)
{
    // Set up the editor with appropriate size constraints
    setSize (1200, 800);
    setResizable (true, true);
    resizeConstrainer.setMinimumSize (1000, 700);
    resizeConstrainer.setMaximumSize (1600, 1200);
    setConstrainer (&resizeConstrainer);

    // Configure spectrogram component
    addAndMakeVisible (spectrogramComponent);
    spectrogramComponent.setTimeWindow (2.0f);
    spectrogramComponent.setFreqRange (20.0f, 20000.0f);
    spectrogramComponent.setColourMap (0); // Viridis
    spectrogramComponent.setUpdateRateHz (30);
    spectrogramComponent.setEnabled (true);

    // Start timer for real-time updates (30 FPS)
    startTimerHz (30);

    // Set up the magic GUI state
    magicState.setGuiValueTree (binaryData, binaryDataSize);

    // Initialize layout
    setupLayout();
}

WubForgeAudioProcessorEditor::~WubForgeAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void WubForgeAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill background with dark theme
    g.fillAll (juce::Colour (30, 30, 30));

    // Draw title
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("WubForge - Spectral Bass Processor",
                getLocalBounds().removeFromTop (40).reduced (20, 0),
                juce::Justification::centred, true);

    // Draw version info
    g.setColour (juce::Colour (150, 150, 150));
    g.setFont (12.0f);
    g.drawText ("v1.0.0",
                getLocalBounds().getRight() - 100, 10, 80, 20,
                juce::Justification::centredRight, true);
}

void WubForgeAudioProcessorEditor::resized()
{
    setupLayout();
}

//==============================================================================
void WubForgeAudioProcessorEditor::setupLayout()
{
    auto bounds = getLocalBounds();
    auto topSection = bounds.removeFromTop (40); // Title area

    // Main content area
    auto contentArea = bounds.reduced (20);

    // Left side: Spectrogram (40% width)
    auto spectrogramArea = contentArea.removeFromLeft (contentArea.getWidth() * 0.4f);
    spectrogramComponent.setBounds (spectrogramArea);

    // Right side: Controls (60% width)
    auto controlsArea = contentArea;

    // Module controls section (top 60% of controls area)
    auto moduleControlsArea = controlsArea.removeFromTop (controlsArea.getHeight() * 0.6f);

    // Create module control sections
    auto moduleSlotHeight = moduleControlsArea.getHeight() / numModuleSlots;
    for (int i = 0; i < numModuleSlots; ++i)
    {
        auto slotArea = moduleControlsArea.removeFromTop (moduleSlotHeight);

        if (moduleControlComponents[i] == nullptr)
        {
            // Create a simple module control panel
            auto* modulePanel = new juce::GroupComponent();
            modulePanel->setText ("Module " + juce::String (i + 1));
            modulePanel->setBounds (slotArea.reduced (5));
            addAndMakeVisible (modulePanel);
            moduleControlComponents[i] = modulePanel;
        }
        else
        {
            moduleControlComponents[i]->setBounds (slotArea.reduced (5));
        }

        // Get the module from processor and update controls
        if (auto* module = audioProcessor.getModuleInSlot (i))
        {
            // Update module-specific controls based on type
            updateModuleControls();
        }
    }

    // Routing matrix section (bottom 20% of controls area)
    auto routingArea = controlsArea.removeFromTop (controlsArea.getHeight() * 0.5f);
    if (routingMatrixComponent == nullptr)
    {
        auto* routingMatrix = new juce::GroupComponent();
        routingMatrix->setText ("Routing Matrix");
        routingMatrix->setBounds (routingArea.reduced (5));
        addAndMakeVisible (routingMatrix);
        routingMatrixComponent = routingMatrix;
    }
    else
    {
        routingMatrixComponent->setBounds (routingArea.reduced (5));
    }

    // Preset management section (bottom 20% of controls area)
    auto presetArea = controlsArea;
    if (presetComponent == nullptr)
    {
        auto* presetPanel = new juce::GroupComponent();
        presetPanel->setText ("Presets");
        presetPanel->setBounds (presetArea.reduced (5));
        addAndMakeVisible (presetPanel);
        presetComponent = presetPanel;
    }
    else
    {
        presetComponent->setBounds (presetArea.reduced (5));
    }
}

void WubForgeAudioProcessorEditor::updateModuleControls()
{
    for (int i = 0; i < numModuleSlots; ++i)
    {
        if (auto* module = audioProcessor.getModuleInSlot (i))
        {
            // Update controls based on module type and parameters
            // This would be expanded based on specific module parameter interfaces
            juce::String moduleName = module->getName();
            if (auto* component = moduleControlComponents[i].getComponent())
            {
                if (auto* group = dynamic_cast<juce::GroupComponent*>(component))
                {
                    group->setText (moduleName);
                }
            }
        }
    }
}

void WubForgeAudioProcessorEditor::updateRoutingVisualization()
{
    // Update routing matrix visualization based on current routing state
    // This would visualize the signal flow between modules
    if (auto* component = routingMatrixComponent.getComponent())
    {
        // Update routing display
        component->repaint();
    }
}

void WubForgeAudioProcessorEditor::updateSpectrumData()
{
    // Get spectrum data from processor for visualization
    const int maxSize = 512;
    float spectrumData[maxSize];

    if (audioProcessor.getCurrentSpectrumData (spectrumData, maxSize))
    {
        spectrogramComponent.pushSpectrumData (spectrumData, maxSize,
                                               audioProcessor.getSampleRate());
    }
}

void WubForgeAudioProcessorEditor::timerCallback()
{
    // Update real-time visualization data
    updateSpectrumData();
    updateRoutingVisualization();

    // Trigger repaint for smooth animation
    if (spectrogramComponent.isVisible())
    {
        spectrogramComponent.repaint();
    }
}