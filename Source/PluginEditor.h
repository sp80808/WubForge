#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "SpectrogramComponent.h"

//==============================================================================
/**
    WubForgeAudioProcessorEditor - Professional UI per Design Roadmap

    Features:
    - Modular chain visualization
    - XY pad for spectral morphing control
    - Routing matrix interface
    - Magic Forge randomization
    - Preset management system
    - Real-time spectrogram display
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
    juce::StretchableLayoutManager layoutManager;

    // Header section
    juce::Component header;
    juce::Label titleLabel { {}, "WUBFORGE" };
    juce::Label subtitleLabel { {}, "Professional Spectral Bass Processor" };
    juce::Label infoLabel { {}, "Spectral Audio Effect\nAlpha Test Build" };

    // Left panel - Module chain and routing
    juce::Component leftPanel;
    juce::GroupComponent moduleChainGroup { {}, "Processing Chain" };
    juce::GroupComponent routingGroup { {}, "Signal Routing" };
    juce::ComboBox routingCombo { "routing" };

    // Center panel - Spectrogram and XY pad
    juce::Component centerPanel;
    juce::GroupComponent spectrogramGroup { {}, "Real-time Spectral Analysis" };
    SpectrogramComponent spectrogramComponent;
    juce::GroupComponent xyPadGroup { {}, "Spectral Morphing Control" };
    juce::Component xyPad;

    // Right panel - Global controls and presets
    juce::Component rightPanel;
    juce::TextButton magicForgeButton { "💫 MAGIC FORGE" };
    juce::GroupComponent presetGroup { {}, "Presets" };
    juce::ComboBox presetCombo { "presets" };
    juce::TextButton savePresetButton { "💾" };
    juce::TextButton loadPresetButton { "📂" };

    //==============================================================================
    // UI Methods
    void setupLayout();
    void setupColors();
    void connectControls();
    void updateModuleVisualization();
    void handleMagicForge();

    // XY Pad control
    float xyPadValueX = 0.5f;
    float xyPadValueY = 0.5f;
    void drawXYPad (juce::Graphics& g, juce::Rectangle<float> area);
    void mouseDownXY (const juce::MouseEvent& event);
    void mouseDragXY (const juce::MouseEvent& event);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WubForgeAudioProcessorEditor)
};
