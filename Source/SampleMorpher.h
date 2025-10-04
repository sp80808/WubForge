#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include "Module.h"
#include "KeyTracker.h"

//==============================================================================
/**
    SampleMorpher - Serum 2-style granular sample-to-bass processor

    Features:
    - Drag-and-drop .wav sample loading via AudioFormatManager
    - 512-sample granular synthesis with windowed overlap-add
    - Key-tracked grain rate modulation for bass response
    - Envelope-controlled position/depth modulation
    - 20-80% wet/dry morphing with input signal
    - Real-time FFT morphing for "unreal" sample basses
*/
class SampleMorpher : public AudioModule
{
public:
    //==============================================================================
    SampleMorpher();
    ~SampleMorpher() override;

    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    //==============================================================================
    const juce::String getName() const override { return "Sample Morpher"; }
    ModuleType getType() const override { return ModuleType::Filter; }

    //==============================================================================
    // Sample Management
    bool loadSample (const juce::File& file);
    void unloadSample();
    bool isSampleLoaded() const { return sampleLoaded; }

    //==============================================================================
    // Morphing Parameters
    void setMorphAmount (float amount);      // 0.0 = pure input, 1.0 = pure sample
    void setGrainSize (int samples);         // 128-2048 samples
    void setGrainOverlap (float overlap);    // 0.0-0.75 (0% to 75% overlap)
    void setPositionMod (float modAmount);   // Envelope modulation depth
    void setKeyTrackAmount (float amount);   // How much grain rate follows MIDI

    //==============================================================================
    // Envelope Parameters
    void setAttackTime (float seconds);
    void setReleaseTime (float seconds);
    void setEnvelopeDepth (float depth);

    //==============================================================================
    // Getters for UI
    float getCurrentMorphAmount() const { return morphAmount; }
    float getCurrentGrainPosition() const { return currentGrainPosition; }
    float getSampleLengthSeconds() const;
    juce::String getSampleName() const { return loadedSampleName; }

private:
    //==============================================================================
    // Sample Loading and Management
    // std::unique_ptr<juce::AudioFormatReader> sampleReader; // TODO: Fix JUCE API compatibility
    juce::AudioBuffer<float> sampleBuffer;
    bool sampleLoaded = false;
    juce::String loadedSampleName;

    //==============================================================================
    // Granular Synthesis Engine
    static constexpr int fftSize = 512;  // Granular window size
    static constexpr int hopSize = 256;  // 50% overlap for smooth morphing

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::vector<float> grainWindow;
    std::vector<float> analysisBuffer;
    std::vector<float> synthesisBuffer;

    float currentGrainPosition = 0.0f;
    float grainPositionIncrement = 1.0f;
    int writePosition = 0;

    //==============================================================================
    // Key Tracking and MIDI Response
    KeyTracker* keyTracker = nullptr;
    float keyTrackAmount = 0.5f;
    float baseFrequency = 440.0f;

    //==============================================================================
    // Envelope System
    juce::ADSR envelope;
    float attackTime = 0.01f;    // 10ms attack for punch
    float releaseTime = 0.1f;    // 100ms release
    float envelopeDepth = 1.0f;

    //==============================================================================
    // Morphing Parameters
    float morphAmount = 0.5f;    // 50% mix by default
    int grainSize = 512;
    float grainOverlap = 0.5f;   // 50% overlap
    float positionModAmount = 0.3f;

    //==============================================================================
    // Processing State
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    //==============================================================================
    // Internal Methods
    void updateGrainParameters();
    void processGrain (float* output, const float* input, int numSamples);
    float getKeyTrackedGrainRate();
    void applyEnvelopeModulation();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleMorpher)
};
