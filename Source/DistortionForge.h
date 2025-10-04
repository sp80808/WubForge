#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class DistortionForge
{
public:
    DistortionForge();
    ~DistortionForge();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    //==============================================================================
    // Parameter setters
    void setWavefoldAmount (float amount);
    void setClipAmount (float amount);
    void setBitCrushAmount (float amount);
    void setFormantFreq (float freqHz);
    void setHammerMode (bool enabled);
    void setModeBlend (float blend);

    //==============================================================================
    // Key tracking
    void setKeyTrackFrequency (float frequency);
    void setKeyTrackAmount (float amount);

    //==============================================================================
    // Getters for visualization
    float getCurrentFormantFreq() const { return currentFormantFreq; }
    bool getHammerMode() const { return hammerMode; }

private:
    //==============================================================================
    // DSP Components
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> formantFilter;
    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    // Parameters
    float wavefoldAmount = 0.3f;
    float clipAmount = 0.4f;
    float bitCrushAmount = 0.2f;
    float formantFreq = 400.0f;
    float currentFormantFreq = 400.0f;
    bool hammerMode = false;
    float modeBlend = 0.5f;

    // Key tracking parameters
    float keyTrackFrequency = 440.0f;  // Base frequency for key tracking
    float keyTrackAmount = 0.0f;       // Amount of key tracking (0 = none, 1 = full)

    // State
    double sampleRate = 44100.0;
    float lastFormantFreq = 400.0f;

    // Hammer mode state
    int hammerCounter = 0;
    static constexpr int hammerInterval = 100; // Randomize every 100 process calls

    //==============================================================================
    // DSP processing functions
    float processWavefolding (float input);
    float processAsymmetricClipping (float input);
    float processBitCrushing (float input);

    void updateFormantFilter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionForge)
};
