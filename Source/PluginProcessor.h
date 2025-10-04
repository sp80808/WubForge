#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <memory>
#include "Module.h"
#include "KeyTracker.h"
#include "Presets.h"
#include "HarmonicRichFilter.h"

//==============================================================================
class WubForgeAudioProcessor : public juce::AudioProcessor,
                               public juce::AudioProcessorValueTreeState::Listener
{
private:
    //==============================================================================
    // AudioProcessorValueTreeState::Listener required implementation
    void parameterChanged(const juce::String& parameterID, float newValue) override;

public:
    //==============================================================================
    // --- Factory Methods for Modules ---
    static juce::StringArray getAvailableModules();
    static std::unique_ptr<AudioModule> createModuleFromName(const juce::String& name);

    //==============================================================================
    WubForgeAudioProcessor();
    ~WubForgeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    // Modular System Access
    AudioModule* getModuleInSlot (int slotIndex) const;

    //==============================================================================
    // JUCE AudioProcessor Required Overrides
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Visualization Data Access
    bool getCurrentSpectrumData(float* magnitudeBuffer, int maxSize) const;

private:
    //==============================================================================
    // Modular DSP Components
    static constexpr int numModuleSlots = 5;
    std::array<std::unique_ptr<AudioModule>, numModuleSlots> moduleSlots;
    Routing currentRouting = Routing::Serial;

    // Global components
    KeyTracker keyTracker;

    // Feedback components
    juce::AudioBuffer<float> feedbackBuffer;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> feedbackDampingFilter;

    // Output processing
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPassFilter;
    juce::dsp::Gain<float> outputGain;
    juce::dsp::DryWetMixer<float> dryWetMixer;

    //==============================================================================
    void runMagicForge();



    // Parameter management
    std::unique_ptr<Presets> presets;
    juce::AudioProcessorValueTreeState valueTreeState;

    // State
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateDSPParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WubForgeAudioProcessor)
};
