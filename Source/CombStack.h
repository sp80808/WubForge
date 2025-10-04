#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class CombStack
{
public:
    CombStack();
    ~CombStack();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    //==============================================================================
    // Parameter setters
    void setCombCount (int count);
    void setDelayTime (float delayMs);
    void setFeedback (float feedback);
    void setLfoRate (float rateHz);
    void setLfoDepth (float depth);
    void setModeBlend (float blend);

    //==============================================================================
    // Key tracking
    void setKeyTrackFrequency (float frequency);
    void setKeyTrackAmount (float amount);

    //==============================================================================
    // Getters for visualization
    int getCombCount() const { return combCount; }
    float getCurrentDelayTime() const { return currentDelayTime; }
    float getLfoPhase() const { return lfoPhase; }

private:
    //==============================================================================
    // DSP Components
    static constexpr int maxCombCount = 8;
    std::array<juce::dsp::DelayLine<float>, maxCombCount> delayLines;
    std::array<float, maxCombCount> delayTimes;
    std::array<float, maxCombCount> phaseOffsets;

    // LFO for modulation
    juce::dsp::Oscillator<float> lfo;
    float lfoPhase = 0.0f;
    float lfoPhaseIncrement = 0.0f;

    // Parameters
    int combCount = 6;
    float baseDelayTime = 1.0f;  // ms
    float currentDelayTime = 1.0f;
    float feedback = 0.7f;
    float lfoRate = 1.0f;
    float lfoDepth = 0.5f;
    float modeBlend = 0.5f;

    // Key tracking parameters
    float keyTrackFrequency = 440.0f;  // Base frequency for key tracking
    float keyTrackAmount = 0.0f;       // Amount of key tracking (0 = none, 1 = full)

    // State
    double sampleRate = 44100.0;
    float lastDelayTime = 1.0f;

    //==============================================================================
    void updateDelayTimes();
    void updateLfo();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CombStack)
};
