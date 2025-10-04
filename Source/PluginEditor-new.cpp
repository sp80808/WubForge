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
void WubForgeAudioProcessorEditor::setupModuleControls()
{
    // Add module controls to tabs

    // Fractal Filter Tab
    tabbedModules.addTab ("Filter", juce::Colours::darkgrey, &fractalGroup, false);

    // Spectral Filter Tab
    tabbedModules.addTab ("Spectral", juce::Colours::darkgrey, &spectralGroup, false);

    // Distortion Tab
    tabbedModules.addTab ("Distort", juce::Colours::darkgrey, &distortionGroup, false);

    // MDA SubSynth Tab - This is the big feature!
    tabbedModules.addTab ("MDA Sub", juce::Colours::darkorange, &subSynthGroup, false);

    // DistortionForge Tab
    tabbedModules.addTab ("Forge", juce::Colours::darkgrey, &forgeGroup, false);
}

void WubForgeAudioProcessorEditor::setupParameterBoxes()
{
    // Routing controls
    routingBox.addItem ("Serial", 1);
    routingBox.addItem ("Parallel", 2);
    routingBox.addItem ("Feedback", 3);
    routingBox.addItem ("Mid-Side", 4);
    routingBox.setSelectedId (1);

    // Fractal filter controls
    fractalType.addItem ("Low Pass", 1);
    fractalType.addItem ("High Pass", 2);
    fractalType.addItem ("Band Pass", 3);
    fractalType.setSelectedId (1);

    // Spectral filter controls
    spectralMode.addItem ("Notch", 1);
    spectralMode.addItem ("Comb", 2);
    spectralMode.setSelectedId (1);

    // Distortion model controls
    distortionModel.addItem ("Digital", 1);
    distortionModel.addItem ("FM", 2);
    distortionModel.addItem ("Rodent", 3);
    distortionModel.addItem ("Screamer", 4);
    distortionModel.setSelectedId (1);

    // MDA SubSynth type controls - The main feature!
    subSynthType.addItem ("Distort", 1);
    subSynthType.addItem ("Divide", 2);
    subSynthType.addItem ("Invert", 3);
    subSynthType.addItem ("Key Osc", 4);
    subSynthType.setSelectedId (1);

    // DistortionForge algorithm controls
    forgeAlgorithm.addItem ("Tanh", 1);
    forgeAlgorithm.addItem ("Hard Clip", 2);
    forgeAlgorithm.addItem ("Soft Clip", 3);
    forgeAlgorithm.addItem ("Wavefold", 4);
    forgeAlgorithm.addItem ("Bit Crush", 5);
    forgeAlgorithm.setSelectedId (1);

    // Setup slider ranges and styles
    setupSliderProperties();
}

void WubForgeAudioProcessorEditor::setupSliderProperties()
{
    // Fractal Filter controls
    fractalFreq.setRange (20.0, 20000.0, 1.0);
    fractalFreq.setSkewFactor (0.3);
    fractalQ.setRange (0.1, 18.0, 0.01);
    fractalDepth.setRange (1, 8, 1);

    // Spectral Filter controls
    spectralFreq.setRange (20.0, 20000.0, 1.0);
    spectralFreq.setSkewFactor (0.3);
    spectralBW.setRange (10.0, 5000.0, 1.0);
    spectralBW.setSkewFactor (0.4);

    // MDA SubSynth controls (the main bass feature!)
    subSynthWet.setRange (0.0, 1.0, 0.01);
    subSynthDry.setRange (0.0, 1.0, 0.01);
    subSynthThresh.setRange (-60.0, 0.0, 0.1);

    // DistortionForge controls
    forgeDrive.setRange (-20.0, 40.0, 0.1);
    forgeTone.setRange (200.0, 8000.0, 1.0);
    forgeTone.setSkewFactor (0.3);

    // Global output controls
    outputGain.setRange (-20.0, 12.0, 0.1);
    dryWetMix.setRange (0.0, 1.0, 0.01);

    // Feedback controls
    feedbackAmount.setRange (0.0, 0.99, 0.01);
    feedbackDamping.setRange (500.0, 15000.0, 1.0);
    feedbackDamping.setSkewFactor (0.4);
}

void WubForgeAudioProcessorEditor::setupControlLayouts()
{
    // Layout fractal controls
    auto fractalArea = fractalGroup.getLocalBounds().reduced (10);
    fractalType.setBounds (fractalArea.removeFromTop (30).reduced (0, 2));
    fractalFreq.setBounds (fractalArea.removeFromTop (30).reduced (0, 2));
    fractalQ.setBounds (fractalArea.removeFromTop (30).reduced (0, 2));
    fractalDepth.setBounds (fractalArea.removeFromTop (30).reduced (0, 2));

    // Layout spectral controls
    auto spectralArea = spectralGroup.getLocalBounds().reduced (10);
    spectralMode.setBounds (spectralArea.removeFromTop (30).reduced (0, 2));
    spectralFreq.setBounds (spectralArea.removeFromTop (30).reduced (0, 2));
    spectralBW.setBounds (spectralArea.removeFromTop (30).reduced (0, 2));

    // Layout distortion controls
    auto distortionArea = distortionGroup.getLocalBounds().reduced (10);
    distortionModel.setBounds (distortionArea);

    // Layout MDA SubSynth controls (the big feature!)
    auto subSynthArea = subSynthGroup.getLocalBounds().reduced (10);
    int controlHeight = 25;
    int labelHeight = 20;

    subSynthType.setBounds (subSynthArea.removeFromTop (controlHeight));
    subSynthArea.removeFromTop (5); // spacing

    auto wetArea = subSynthArea.removeFromTop (controlHeight + labelHeight);
    subSynthWetLabel.setBounds (wetArea.removeFromTop (labelHeight));
    subSynthWet.setBounds (wetArea);

    auto dryArea = subSynthArea.removeFromTop (controlHeight + labelHeight);
    subSynthDryLabel.setBounds (dryArea.removeFromTop (labelHeight));
    subSynthDry.setBounds (dryArea);

    auto threshArea = subSynthArea.removeFromTop (controlHeight + labelHeight);
    subSynthThreshLabel.setBounds (threshArea.removeFromTop (labelHeight));
    subSynthThresh.setBounds (threshArea);

    // Layout forge controls
    auto forgeArea = forgeGroup.getLocalBounds().reduced (10);
    forgeAlgorithm.setBounds (forgeArea.removeFromTop (30).reduced (0, 2));
    forgeDrive.setBounds (forgeArea.removeFromTop (30).reduced (0, 2));
    forgeTone.setBounds (forgeArea.removeFromTop (30).reduced (0, 2));

    // Layout global controls
    auto outputArea = outputGroup.getLocalBounds().reduced (10);
    outputGain.setBounds (outputArea.removeFromTop (35).reduced (0, 2));
    dryWetMix.setBounds (outputArea.removeFromTop (35).reduced (0, 2));

    auto feedbackArea = feedbackGroup.getLocalBounds().reduced (10);
    feedbackAmount.setBounds (feedbackArea.removeFromTop (35).reduced (0, 2));
    feedbackDamping.setBounds (feedbackArea.removeFromTop (35).reduced (0, 2));

    auto routingArea = routingGroup.getLocalBounds().reduced (10);
    routingLabel.setBounds (routingArea.removeFromTop (20));
    routingBox.setBounds (routingArea.removeFromTop (30));
}

void WubForgeAudioProcessorEditor::loadParameterValues()
{
    // Load current parameter values from processor
    if (auto* apvts = audioProcessor.getParameters())
    {
        // For now we load some defaults - in a full implementation,
        // this would load from the actual parameter state
        fractalType.setSelectedId (1);
        spectralMode.setSelectedId (1);
        distortionModel.setSelectedId (1);
        subSynthType.setSelectedId (1); // Distort mode as default for MDA SubSynth
        forgeAlgorithm.setSelectedId (1);

        fractalFreq.setValue (1000.0);
        fractalQ.setValue (1.0);
        fractalDepth.setValue (4);

        spectralFreq.setValue (1000.0);
        spectralBW.setValue (100.0);

        subSynthWet.setValue (0.3);     // Default wet mix for bass enhancement
        subSynthDry.setValue (1.0);     // Full dry signal
        subSynthThresh.setValue (-24.0); // -24dB threshold

        forgeDrive.setValue (0.0);
        forgeTone.setValue (2000.0);

        outputGain.setValue (0.0);
        dryWetMix.setValue (1.0);

        feedbackAmount.setValue (0.0);
        feedbackDamping.setValue (5000.0);

        routingBox.setSelectedId (1); // Serial routing
    }
}

void WubForgeAudioProcessorEditor::connectParametersToControls()
{
    // In a full implementation, this would connect each control to APVTS parameters
    // For now, we leave this as a stub for the distribution version
}

void WubForgeAudioProcessorEditor::updateUI()
{
    // Update UI state based on current parameters
    // This is called periodically to refresh any dynamic elements
}

void WubForgeAudioProcessorEditor::timerCallback()
{
    // Update real-time elements like spectrogram
    updateUI();
}
