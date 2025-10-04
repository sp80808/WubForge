ta#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "SpectrogramComponent.h"

//==============================================================================
class WobbleForgeAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    WobbleForgeAudioProcessorEditor (WobbleForgeAudioProcessor&);
    ~WobbleForgeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    WobbleForgeAudioProcessor& audioProcessor;

    // GUI Components
    juce::GroupComponent chainGroup;
    juce::GroupComponent moduleControlsGroup;
    juce::GroupComponent routingControlsGroup;
    juce::GroupComponent distortionGroup;
    juce::GroupComponent outputGroup;
    juce::GroupComponent modeGroup;

    // Chain controls
    juce::ComboBox routingSelector;
    std::array<juce::TextButton, 4> moduleSlotButtons;
    int selectedSlot = 0;

    // Fractal Filter controls
    juce::ComboBox fractalTypeSelector;
    juce::Slider fractalFreqSlider;
    juce::Slider fractalQSlider;
    juce::Slider fractalDepthSlider;
    juce::Slider fractalRatioSlider;

    // Comb Stack controls
    juce::Slider combCountSlider;
    juce::Slider combDelaySlider;
    juce::Slider combFeedbackSlider;
    juce::Slider lfoRateSlider;
    juce::Slider lfoDepthSlider;

    // Distortion Forge controls
    juce::Slider wavefoldSlider;
    juce::Slider clipSlider;
    juce::Slider bitCrushSlider;
    juce::Slider formantFreqSlider;

    // Distortion Forge controls
    juce::Slider wavefoldSlider;
    juce::Slider clipSlider;
    juce::Slider bitCrushSlider;
    juce::Slider formantFreqSlider;

    // Formant Tracker controls
    juce::Slider formantKeyTrackSlider;
    juce::Slider formantGainSlider;
    juce::Slider formantQSlider;
    juce::Slider formantBaseFreqSlider;

    // FM Distort controls
    juce::Slider fmRatioSlider;
    juce::Slider fmIndexSlider;

    // Rat Distortion controls
    juce::Slider ratDriveSlider;
    juce::Slider ratToneSlider;
    juce::Slider ratLevelSlider;

    // Routing controls
    juce::Slider feedbackAmountSlider;
    juce::Slider feedbackDampingSlider;

    // Output controls
    juce::Slider hpfSlider;
    juce::Slider outputGainSlider;
    juce::Slider dryWetSlider;

    // Mode controls
    juce::Slider wobbleModeSlider;
    juce::ToggleButton hammerModeButton;

    // XY Pad for mode morphing
    class XYPad : public juce::Component
    {
    public:
        XYPad (WobbleForgeAudioProcessor& proc);
        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseDrag (const juce::MouseEvent& event) override;

    private:
        WobbleForgeAudioProcessor& processor;
        float xValue = 0.5f;
        float yValue = 0.5f;
    };

    XYPad xyPad;

    // Spectrogram visualization
    SpectrogramComponent spectrogramComponent;

    // Labels
    juce::Label combCountLabel;
    juce::Label combDelayLabel;
    juce::Label combFeedbackLabel;
    juce::Label lfoRateLabel;
    juce::Label lfoDepthLabel;
    juce::Label wavefoldLabel;
    juce::Label clipLabel;
    juce::Label bitCrushLabel;
    juce::Label formantFreqLabel;
    juce::Label hpfLabel;
    juce::Label outputGainLabel;
    juce::Label dryWetLabel;
    juce::Label wobbleModeLabel;

    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboBoxAttachment> routingAttachment;

    // Fractal Filter attachments
    std::unique_ptr<ComboBoxAttachment> fractalTypeAttachment;
    std::unique_ptr<SliderAttachment> fractalFreqAttachment;
    std::unique_ptr<SliderAttachment> fractalQAttachment;
    std::unique_ptr<SliderAttachment> fractalDepthAttachment;
    std::unique_ptr<SliderAttachment> fractalRatioAttachment;

    std::unique_ptr<SliderAttachment> combCountAttachment;
    std::unique_ptr<SliderAttachment> combDelayAttachment;
    std::unique_ptr<SliderAttachment> combFeedbackAttachment;
    std::unique_ptr<SliderAttachment> lfoRateAttachment;
    std::unique_ptr<SliderAttachment> lfoDepthAttachment;

    // Distortion Forge attachments
    std::unique_ptr<SliderAttachment> wavefoldAttachment;
    std::unique_ptr<SliderAttachment> clipAttachment;
    std::unique_ptr<SliderAttachment> bitCrushAttachment;
    std::unique_ptr<SliderAttachment> formantFreqAttachment;

    // Formant Tracker attachments
    std::unique_ptr<SliderAttachment> formantKeyTrackAttachment;
    std::unique_ptr<SliderAttachment> formantGainAttachment;
    std::unique_ptr<SliderAttachment> formantQAttachment;
    std::unique_ptr<SliderAttachment> formantBaseFreqAttachment;

    // FM Distort attachments
    std::unique_ptr<SliderAttachment> fmRatioAttachment;
    std::unique_ptr<SliderAttachment> fmIndexAttachment;

    // Routing attachments
    std::unique_ptr<SliderAttachment> feedbackAmountAttachment;
    std::unique_ptr<SliderAttachment> feedbackDampingAttachment;

    std::unique_ptr<ButtonAttachment> hammerModeAttachment;

    //==============================================================================
    void setupSliders();
    void setupLabels();
    void setupAttachments();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WobbleForgeAudioProcessorEditor)
};
