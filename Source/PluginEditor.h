#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h> // Temporarily use standard JUCE editor
#include "PluginProcessor.h"
#include "SpectrogramComponent.h"

//==============================================================================
/**
    WubForgeAudioProcessorEditor - Main GUI for the WubForge spectral bass processor

    Features:
    - Real-time spectrogram visualization
    - 4-slot modular processing chain controls
    - Routing matrix visualization
    - Preset management interface
    - Professional audio plugin UI standards
*/
class WubForgeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    WubForgeAudioProcessorEditor (WubForgeAudioProcessor&);
    ~WubForgeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    WubForgeAudioProcessor& audioProcessor;

    // Main layout components
    juce::StretchableLayoutManager stretchableManager;
    juce::StretchableLayoutResizerBar resizerBar1;
    juce::StretchableLayoutResizerBar resizerBar2;

    // Left panel - Module controls
    juce::TabbedComponent tabbedModules { juce::TabbedButtonBar::Orientation::TabsAtTop };

    // Center panel - Spectrogram visualization
    juce::Component centerPanel;
    juce::Label headerLabel { {}, "WUBFORGE SPECTRAL BASS PROCESSOR" };
    SpectrogramComponent spectrogramComponent;

    // Right panel - Global controls and routing
    juce::Component rightPanel;
    juce::GroupComponent routingGroup { {}, "Routing & Output" };
    juce::ComboBox routingBox { "routing" };
    juce::Label routingLabel { {}, "Signal Flow:" };

    // Module parameter controls
    void setupModuleControls();

    // fractal filter controls (slot 0)
    juce::GroupComponent fractalGroup { {}, "Fractal Filter" };
    juce::ComboBox fractalType { "fractalType" };
    juce::Slider fractalFreq { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider fractalQ { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider fractalDepth { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    // spectral filter controls (slot 1)
    juce::GroupComponent spectralGroup { {}, "Spectral Filter" };
    juce::ComboBox spectralMode { "spectralMode" };
    juce::Slider spectralFreq { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider spectralBW { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    // distortion controls (slot 2)
    juce::GroupComponent distortionGroup { {}, "Universal Distortion" };
    juce::ComboBox distortionModel { "distModel" };

    // MDA SubSynth controls (slot 3)
    juce::GroupComponent subSynthGroup { {}, "MDA SubSynth" };
    juce::ComboBox subSynthType { "subBassType" };
    juce::Slider subSynthWet { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider subSynthDry { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider subSynthThresh { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Label subSynthTypeLabel { {}, "Mode:" };
    juce::Label subSynthWetLabel { {}, "Wet:" };
    juce::Label subSynthDryLabel { {}, "Dry:" };
    juce::Label subSynthThreshLabel { {}, "Threshold:" };

    // DistortionForge controls (slot 4)
    juce::GroupComponent forgeGroup { {}, "Distortion Forge" };
    juce::ComboBox forgeAlgorithm { "forgeAlgorithm" };
    juce::Slider forgeDrive { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider forgeTone { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    // Global output controls
    juce::GroupComponent outputGroup { {}, "Global Output" };
    juce::Slider outputGain { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider dryWetMix { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    // Feedback controls
    juce::GroupComponent feedbackGroup { {}, "Feedback" };
    juce::Slider feedbackAmount { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider feedbackDamping { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    //==============================================================================
    void loadParameterValues();
    void connectParametersToControls();
    void updateUI();
    void setupParameterBoxes();

    // Timer for UI updates
    void timerCallback() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WubForgeAudioProcessorEditor)
};
