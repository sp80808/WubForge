#pragma once

//#include <foleys_gui_magic/foleys_gui_magic.h> // Temporarily commented out due to build issues
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <memory>
#include "Module.h"
#include "KeyTracker.h"
#include "Presets.h"

//==============================================================================
class WubForgeAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    WubForgeAudioProcessor();
    ~WubForgeAudioProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
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

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    //==============================================================================
    // Modular System Access
    AudioModule* getModuleInSlot (int slotIndex) const;

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
    juce::AudioProcessorValueTreeState valueTreeState;
    std::unique_ptr<Presets> presets;

    // State
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateDSPParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WubForgeAudioProcessor)
};
