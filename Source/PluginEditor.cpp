#include "PluginEditor.h"

//==============================================================================
WubForgeAudioProcessorEditor::WubForgeAudioProcessorEditor (WubForgeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1400, 900);
    setResizable (false, false); // Fixed size for now

    // Initialize UI components
    addAndMakeVisible (tabbedModules);
    addAndMakeVisible (centerPanel);
    addAndMakeVisible (rightPanel);

    // Center panel setup - Spectrogram section
    centerPanel.addAndMakeVisible (headerLabel);
    centerPanel.addAndMakeVisible (spectrogramComponent);

    // Configure spectrogram
    spectrogramComponent.setTimeWindow (2.0f);
    spectrogramComponent.setFreqRange (20.0f, 20000.0f);
    spectrogramComponent.setColourMap (0);
    spectrogramComponent.setUpdateRateHz (30);
    spectrogramComponent.setEnabled (true);

    // Style the header
    headerLabel.setFont (headerLabel.getFont().withHeight (18.0f));
    headerLabel.setJustificationType (juce::Justification::centred);
    headerLabel.setColour (juce::Label::textColourId, juce::Colours::white);

    // Setup parameter controls
    setupModuleControls();
    setupParameterBoxes();
    loadParameterValues();
    connectParametersToControls();

    // Start timer for real-time updates
    startTimerHz (30);

    updateUI();
}

WubForgeAudioProcessorEditor::~WubForgeAudioProcessorEditor()
{
    stopTimer();

    // Clean up parameter attachments (would be done by JUCE value tree state)
}

//==============================================================================
void WubForgeAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Professional dark theme background
    g.fillAll (juce::Colour (45, 48, 51)); // Twitter dark mode background

    // Subtle gradient header
    auto headerBounds = getLocalBounds().removeFromTop (50);
    g.setGradientFill (juce::ColourGradient (juce::Colour (30, 35, 40), headerBounds.getX(), headerBounds.getY(),
                                           juce::Colour (20, 25, 30), headerBounds.getX(), headerBounds.getBottom(), false));
    g.fillRect (headerBounds);

    // Brand logo/text
    g.setColour (juce::Colour (255, 100, 0)); // Orange accent
    g.setFont (28.0f);
    g.drawText ("WUBFORGE", headerBounds.withTrimmedLeft (20).withTrimmedTop (15), juce::Justification::centredLeft);

    // Version info
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.setFont (12.0f);
    g.drawText ("v1.0.0 - Spectral Bass Processor", headerBounds.withTrimmedRight (20).withTrimmedTop (30),
                juce::Justification::centredRight);
}

void WubForgeAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop (50); // Header

    auto contentBounds = bounds.reduced (20);

    // Three-column layout
    auto leftWidth = juce::roundToInt (contentBounds.getWidth() * 0.25f);
    auto centerWidth = juce::roundToInt (contentBounds.getWidth() * 0.45f);
    auto rightWidth = contentBounds.getWidth() - leftWidth - centerWidth;

    // Left panel - Module tabs
    tabbedModules.setBounds (contentBounds.removeFromLeft (leftWidth));

    // Center panel - Spectrogram
    centerPanel.setBounds (contentBounds.removeFromLeft (centerWidth));

    // Right panel - Global controls
    rightPanel.setBounds (contentBounds);

    // Layout internal components
    auto centerArea = centerPanel.getLocalBounds();
    headerLabel.setBounds (centerArea.removeFromTop (30));
    spectrogramComponent.setBounds (centerArea.reduced (5));

    // Layout right panel components
    auto rightArea = rightPanel.getLocalBounds();
    routingGroup.setBounds (rightArea.removeFromTop (120).reduced (5));
    outputGroup.setBounds (rightArea.removeFromTop (120).reduced (5));
    feedbackGroup.setBounds (rightArea.removeFromTop (120).reduced (5));

    // Layout controls within groups
    setupControlLayouts();
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
