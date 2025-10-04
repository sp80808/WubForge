#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WobbleForgeAudioProcessorEditor::WobbleForgeAudioProcessorEditor (WobbleForgeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), xyPad (p), spectrogramComponent ()
{
    setSize (800, 600);

    // ### Chain Controls ###
    chainGroup.setText ("Signal Chain");
    addAndMakeVisible (chainGroup);

    routingSelector.addItemList (juce::StringArray { "Serial", "Parallel", "Feedback" }, 1);
    addAndMakeVisible (routingSelector);

    for (int i = 0; i < moduleSlotButtons.size(); ++i)
    {
        auto& button = moduleSlotButtons[i];
        addAndMakeVisible(button);
        button.onClick = [this, i] { selectedSlot = i; };
    }

    // ### Module & Routing Controls ###
    moduleControlsGroup.setText ("Module Controls");
    addAndMakeVisible (moduleControlsGroup);
    routingControlsGroup.setText ("Routing Controls");
    addAndMakeVisible (routingControlsGroup);

    setupSliders();
    setupAttachments();

    // Add all sliders to the editor so they can be managed
    addAndMakeVisible (fractalTypeSelector);
    addAndMakeVisible (fractalFreqSlider);
    addAndMakeVisible (fractalQSlider);
    addAndMakeVisible (fractalDepthSlider);
    addAndMakeVisible (fractalRatioSlider);
    addAndMakeVisible (combCountSlider);
    addAndMakeVisible (combDelaySlider);
    addAndMakeVisible (combFeedbackSlider);
    addAndMakeVisible (lfoRateSlider);
    addAndMakeVisible (lfoDepthSlider);
    addAndMakeVisible (wavefoldSlider);
    addAndMakeVisible (clipSlider);
    addAndMakeVisible (bitCrushSlider);
    addAndMakeVisible (formantFreqSlider);
    addAndMakeVisible (formantKeyTrackSlider);
    addAndMakeVisible (formantGainSlider);
    addAndMakeVisible (formantQSlider);
    addAndMakeVisible (formantBaseFreqSlider);
    addAndMakeVisible (fmRatioSlider);
    addAndMakeVisible (fmIndexSlider);
    addAndMakeVisible (feedbackAmountSlider);
    addAndMakeVisible (feedbackDampingSlider);
    
    addAndMakeVisible (xyPad);
    addAndMakeVisible (spectrogramComponent);

    startTimerHz (5);
}

WobbleForgeAudioProcessorEditor::~WobbleForgeAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void WobbleForgeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("WubForge", getLocalBounds().removeFromTop (40).toNearestIntEdges(), juce::Justification::centred, true);
}

void WobbleForgeAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40);

    auto chainArea = area.removeFromTop(80);
    chainGroup.setBounds(chainArea);
    auto chainControls = chainGroup.getLocalBounds().reduced(10);
    routingSelector.setBounds(chainControls.removeFromTop(25));
    chainControls.removeFromTop(5);
    auto slotArea = chainControls;
    auto slotWidth = slotArea.getWidth() / 4;
    for(auto& button : moduleSlotButtons)
    {
        button.setBounds(slotArea.removeFromLeft(slotWidth).reduced(2));
    }

    auto leftArea = area.removeFromLeft(area.getWidth() / 2);
    auto moduleArea = leftArea.removeFromTop(leftArea.getHeight() * 0.7);
    moduleControlsGroup.setBounds(moduleArea.reduced(5));
    auto controlsArea = moduleControlsGroup.getLocalBounds().reduced(15);
    controlsArea.removeFromTop(10);

    auto sliderBounds1 = controlsArea.removeFromTop(30);
    fractalTypeSelector.setBounds(sliderBounds1);
    combCountSlider.setBounds(sliderBounds1);
    wavefoldSlider.setBounds(sliderBounds1);
    formantKeyTrackSlider.setBounds(sliderBounds1);
    fmRatioSlider.setBounds(sliderBounds1);

    controlsArea.removeFromTop(5);
    auto sliderBounds2 = controlsArea.removeFromTop(30);
    fractalFreqSlider.setBounds(sliderBounds2);
    combDelaySlider.setBounds(sliderBounds2);
    clipSlider.setBounds(sliderBounds2);
    formantGainSlider.setBounds(sliderBounds2);
    fmIndexSlider.setBounds(sliderBounds2);

    controlsArea.removeFromTop(5);
    auto sliderBounds3 = controlsArea.removeFromTop(30);
    fractalQSlider.setBounds(sliderBounds3);
    combFeedbackSlider.setBounds(sliderBounds3);
    bitCrushSlider.setBounds(sliderBounds3);
    formantQSlider.setBounds(sliderBounds3);

    controlsArea.removeFromTop(5);
    auto sliderBounds4 = controlsArea.removeFromTop(30);
    fractalDepthSlider.setBounds(sliderBounds4);
    lfoRateSlider.setBounds(sliderBounds4);
    formantFreqSlider.setBounds(sliderBounds4);
    formantBaseFreqSlider.setBounds(sliderBounds4);

    controlsArea.removeFromTop(5);
    auto sliderBounds5 = controlsArea.removeFromTop(30);
    fractalRatioSlider.setBounds(sliderBounds5);
    lfoDepthSlider.setBounds(sliderBounds5);

    routingControlsGroup.setBounds(leftArea.reduced(5));
    auto routingArea = routingControlsGroup.getLocalBounds().reduced(15);
    routingArea.removeFromTop(10);
    feedbackAmountSlider.setBounds(routingArea.removeFromTop(30));
    routingArea.removeFromTop(5);
    feedbackDampingSlider.setBounds(routingArea.removeFromTop(30));

    auto rightArea = area;
    xyPad.setBounds(rightArea.removeFromBottom(rightArea.getHeight() / 2).reduced(5));
    spectrogramComponent.setBounds(rightArea.reduced(5));
}

//==============================================================================
void WobbleForgeAudioProcessorEditor::updateControlVisibility()
{
    auto* selectedModule = audioProcessor.getModuleInSlot(selectedSlot);
    juce::String groupName = "[Slot " + juce::String(selectedSlot + 1) + "] Empty";

    if (selectedModule != nullptr)
        groupName = "[Slot " + juce::String(selectedSlot + 1) + "] " + selectedModule->getName();

    moduleControlsGroup.setText(groupName);

    bool isFractal = selectedModule != nullptr && dynamic_cast<FractalFilterModule*>(selectedModule) != nullptr;
    bool isComb = selectedModule != nullptr && dynamic_cast<CombStackModule*>(selectedModule) != nullptr;
    bool isDistortion = selectedModule != nullptr && dynamic_cast<DistortionForgeModule*>(selectedModule) != nullptr;
    bool isFormant = selectedModule != nullptr && dynamic_cast<FormantTrackerModule*>(selectedModule) != nullptr;
    bool isFM = selectedModule != nullptr && dynamic_cast<FMDistortModule*>(selectedModule) != nullptr;

    fractalTypeSelector.setVisible(isFractal);
    fractalFreqSlider.setVisible(isFractal);
    fractalQSlider.setVisible(isFractal);
    fractalDepthSlider.setVisible(isFractal);
    fractalRatioSlider.setVisible(isFractal);

    combCountSlider.setVisible(isComb);
    combDelaySlider.setVisible(isComb);
    combFeedbackSlider.setVisible(isComb);
    lfoRateSlider.setVisible(isComb);
    lfoDepthSlider.setVisible(isComb);

    wavefoldSlider.setVisible(isDistortion);
    clipSlider.setVisible(isDistortion);
    bitCrushSlider.setVisible(isDistortion);
    formantFreqSlider.setVisible(isDistortion);

    formantKeyTrackSlider.setVisible(isFormant);
    formantGainSlider.setVisible(isFormant);
    formantQSlider.setVisible(isFormant);
    formantBaseFreqSlider.setVisible(isFormant);

    fmRatioSlider.setVisible(isFM);
    fmIndexSlider.setVisible(isFM);
}

void WobbleForgeAudioProcessorEditor::updateSlotButtonAppearance()
{
    for (int i = 0; i < moduleSlotButtons.size(); ++i)
    {
        auto& button = moduleSlotButtons[i];
        juce::String newText = "[Empty]";
        if (auto* module = audioProcessor.getModuleInSlot(i))
            newText = module->getName();

        if (button.getButtonText() != newText)
            button.setButtonText(newText);

        auto& lf = getLookAndFeel();
        button.setColour(juce::TextButton::buttonColourId, i == selectedSlot ? lf.findColour(juce::ComboBox::backgroundColourId).darker() : lf.findColour(juce::TextButton::buttonColourId));
    }
}

void WobbleForgeAudioProcessorEditor::timerCallback()
{
    updateSlotButtonAppearance();
    updateControlVisibility();

    // Feed spectrum data to the spectrogram component
    constexpr int spectrumSize = 512;
    static float spectrumBuffer[spectrumSize];

    if (audioProcessor.getCurrentSpectrumData(spectrumBuffer, spectrumSize))
    {
        spectrogramComponent.pushSpectrumData(spectrumBuffer, spectrumSize, audioProcessor.getSampleRate());
    }

    xyPad.repaint();
}

//==============================================================================
void WobbleForgeAudioProcessorEditor::setupSliders()
{
    auto setupSlider = [] (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 20);
    };

    setupSlider(fractalFreqSlider);
    setupSlider(fractalQSlider);
    setupSlider(fractalDepthSlider);
    setupSlider(fractalRatioSlider);
    setupSlider(combCountSlider);
    setupSlider(combDelaySlider);
    setupSlider(combFeedbackSlider);
    setupSlider(lfoRateSlider);
    setupSlider(lfoDepthSlider);
    setupSlider(wavefoldSlider);
    setupSlider(clipSlider);
    setupSlider(bitCrushSlider);
    setupSlider(formantFreqSlider);
    setupSlider(formantKeyTrackSlider);
    setupSlider(formantGainSlider);
    setupSlider(formantQSlider);
    setupSlider(formantBaseFreqSlider);
    setupSlider(fmRatioSlider);
    setupSlider(fmIndexSlider);
    setupSlider(feedbackAmountSlider);
    setupSlider(feedbackDampingSlider);
}

void WobbleForgeAudioProcessorEditor::setupLabels()
{
}

void WobbleForgeAudioProcessorEditor::setupAttachments()
{
    auto& vts = audioProcessor.getValueTreeState();

    routingAttachment = std::make_unique<ComboBoxAttachment> (vts, "routing", routingSelector);

    fractalTypeAttachment = std::make_unique<ComboBoxAttachment> (vts, "fractalType", fractalTypeSelector);
    fractalFreqAttachment = std::make_unique<SliderAttachment> (vts, "fractalFreq", fractalFreqSlider);
    fractalQAttachment = std::make_unique<SliderAttachment> (vts, "fractalQ", fractalQSlider);
    fractalDepthAttachment = std::make_unique<SliderAttachment> (vts, "fractalDepth", fractalDepthSlider);
    fractalRatioAttachment = std::make_unique<SliderAttachment> (vts, "fractalRatio", fractalRatioSlider);

    combCountAttachment = std::make_unique<SliderAttachment> (vts, "combCount", combCountSlider);
    combDelayAttachment = std::make_unique<SliderAttachment> (vts, "combDelay", combDelaySlider);
    combFeedbackAttachment = std::make_unique<SliderAttachment> (vts, "combFeedback", combFeedbackSlider);
    lfoRateAttachment = std::make_unique<SliderAttachment> (vts, "lfoRate", lfoRateSlider);
    lfoDepthAttachment = std::make_unique<SliderAttachment> (vts, "lfoDepth", lfoDepthSlider);

    wavefoldAttachment = std::make_unique<SliderAttachment> (vts, "wavefoldAmount", wavefoldSlider);
    clipAttachment = std::make_unique<SliderAttachment> (vts, "clipAmount", clipSlider);
    bitCrushAttachment = std::make_unique<SliderAttachment> (vts, "bitCrushAmount", bitCrushSlider);
    formantFreqAttachment = std::make_unique<SliderAttachment> (vts, "formantFreq", formantFreqSlider);

    formantKeyTrackAttachment = std::make_unique<SliderAttachment>(vts, "formantKeyTrack", formantKeyTrackSlider);
    formantGainAttachment = std::make_unique<SliderAttachment>(vts, "formantGain", formantGainSlider);
    formantQAttachment = std::make_unique<SliderAttachment>(vts, "formantQ", formantQSlider);
    formantBaseFreqAttachment = std::make_unique<SliderAttachment>(vts, "formantBaseFreq", formantBaseFreqSlider);

    fmRatioAttachment = std::make_unique<SliderAttachment>(vts, "fmRatio", fmRatioSlider);
    fmIndexAttachment = std::make_unique<SliderAttachment>(vts, "fmIndex", fmIndexSlider);

    feedbackAmountAttachment = std::make_unique<SliderAttachment>(vts, "feedbackAmount", feedbackAmountSlider);
    feedbackDampingAttachment = std::make_unique<SliderAttachment>(vts, "feedbackDamping", feedbackDampingSlider);
}

// XY Pad Implementation
WobbleForgeAudioProcessorEditor::XYPad::XYPad (WobbleForgeAudioProcessor& proc) : processor (proc) {}
void WobbleForgeAudioProcessorEditor::XYPad::paint (juce::Graphics& g) { g.fillAll(juce::Colours::black.withAlpha(0.5f)); g.setColour(juce::Colours::white); g.drawText("XY Pad (Future)", getLocalBounds(), juce::Justification::centred); }
void WobbleForgeAudioProcessorEditor::XYPad::mouseDown (const juce::MouseEvent& event) { }
void WobbleForgeAudioProcessorEditor::XYPad::mouseDrag (const juce::MouseEvent& event) { }

// Comb Visualization Implementation
WobbleForgeAudioProcessorEditor::CombVisualization::CombVisualization (WobbleForgeAudioProcessor& proc) : processor (proc) { }
void WobbleForgeAudioProcessorEditor::CombVisualization::paint (juce::Graphics& g) { g.fillAll(juce::Colours::black.withAlpha(0.5f)); g.setColour(juce::Colours::white); g.drawText("Visualization (Future)", getLocalBounds(), juce::Justification::centred); }
