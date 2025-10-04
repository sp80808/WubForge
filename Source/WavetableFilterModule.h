#pragma once

#include "Module.h"
#include "../JUCE/modules/juce_dsp/juce_dsp.h"
#include "../JUCE/modules/juce_audio_processors/juce_audio_processors.h"

/**
 * WavetableFilterModule - "Dying Wavetable" Filter Effect
 *
 * This module implements a sophisticated filter modulation system using:
 * 1. Resampled audio files as wavetable modulation sources
 * 2. Complex LFO with custom shapes for "dying" filter sweeps
 * 3. FM-style filter modulation using wavetable data
 * 4. Psychoacoustic processing with envelope following and A-weighting
 *
 * Perfect for creating evolving, organic filter movements that "die" and bloom
 * dynamically based on input signal characteristics.
 */
class WavetableFilterModule : public FilterModule
{
public:
    // Constructor
    WavetableFilterModule();

    // Destructor
    ~WavetableFilterModule() override;

    // AudioModule interface
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Wavetable Filter"; }
    ModuleType getType() const override { return ModuleType::Filter; }

    // Wavetable Management
    bool loadWavetableFromAudioFile(const juce::File& file);
    void unloadWavetable();
    bool isWavetableLoaded() const { return wavetableLoaded; }
    juce::String getWavetableName() const { return loadedWavetableName; }

    // Core Parameters
    void setCutoffFrequency(float frequencyHz);
    void setResonance(float resonanceAmount);
    void setFilterType(int type); // 0=LP, 1=HP, 2=BP, 3=Notch

    // Modulation Parameters
    void setLfoRate(float rateHz);
    void setLfoShape(int shape); // 0=Saw, 1=Triangle, 2=Sine, 3=Square, 4=Random
    void setLfoDepth(float depth); // 0.0-1.0: How much LFO affects cutoff

    // Wavetable Modulation Parameters
    void setWavetableModDepth(float depth); // 0.0-1.0: FM-style modulation amount
    void setWavetableRate(float rate); // 0.01-10.0: Wavetable playback speed
    void setWavetablePosition(float position); // 0.0-1.0: Manual position control

    // Envelope Following Parameters
    void setEnvelopeSensitivity(float sensitivity); // 0.0-1.0: How much input dynamics affect modulation
    void setEnvelopeAttack(float attackMs); // 0.1-1000ms: Envelope attack time
    void setEnvelopeRelease(float releaseMs); // 0.1-1000ms: Envelope release time

    // Mix Parameters
    void setWetMix(float wetMix); // 0.0-1.0: Effect mix amount

    // Getters for UI
    float getCurrentCutoff() const { return baseCutoff; }
    float getCurrentResonance() const { return resonance; }
    float getCurrentLfoPhase() const { return lfoPhase; }
    float getCurrentWavetablePosition() const { return wavetablePosition / (float)wavetableSize; }

private:
    // Wavetable System
    juce::AudioSampleBuffer wavetable;
    int wavetableSize = 2048;
    bool wavetableLoaded = false;
    juce::String loadedWavetableName;
    float wavetablePosition = 0.0f;
    float wavetableIncrement = 1.0f;

    // Core Filter
    juce::dsp::StateVariableFilter::Filter<float> filter;
    float baseCutoff = 1000.0f;
    float resonance = 0.5f;
    int filterType = 0; // 0=LP, 1=HP, 2=BP, 3=Notch

    // LFO System
    juce::dsp::Oscillator<float> lfo;
    float lfoFrequency = 1.0f;
    float lfoPhase = 0.0f;
    int lfoShape = 0; // 0=Saw, 1=Triangle, 2=Sine, 3=Square, 4=Random
    float lfoDepth = 0.3f;

    // Wavetable Modulation
    float wavetableModDepth = 0.2f;
    float wavetableRate = 1.0f;

    // Envelope Following
    juce::dsp::BallisticsFilter<float> envelopeFollower;
    float envelopeSensitivity = 0.5f;
    float envelopeAttackMs = 10.0f;
    float envelopeReleaseMs = 100.0f;

    // Mix
    float wetMix = 0.8f;

    // DSP State
    double sampleRate = 44100.0;

    // Internal Methods
    void updateLfoShape();
    void updateEnvelopeCoefficients();
    float getWavetableSample();
    float processLfoModulation();
    float processEnvelopeModulation();
    float processWavetableModulation();
    void createDefaultDigitalWavetable();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableFilterModule)
};
