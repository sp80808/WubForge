#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class FormantTracker
{
public:
    FormantTracker();
    ~FormantTracker();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    void process (juce::dsp::ProcessContextReplacing<float>& context);

    //==============================================================================
    // Parameter setters
    void setKeyTrackAmount (float amount);
    void setFormantGain (float gainDb);
    void setFormantQ (float Q);
    void setBaseFormants (const std::array<double, 3>& formants);
    void setBaseFrequency (double baseFreq);
    void setCurrentFrequency (double currentFreq);

    //==============================================================================
    // Getters for visualization
    float getKeyTrackAmount() const { return keyTrackAmount; }
    float getFormantGain() const { return formantGain; }
    float getFormantQ() const { return formantQ; }
    std::array<double, 3> getCurrentFormantFrequencies() const { return currentFormantFreqs; }

private:
    //==============================================================================
    // DSP Components - 3 formant peak filters in series
    static constexpr int numFormants = 3;
    std::array<juce::dsp::IIR::Filter<float>, numFormants> formantFilters;
    
    // Base formant frequencies (neutral vowel)
    std::array<double, 3> baseFormants = {350.0, 1200.0, 2400.0};
    std::array<double, 3> currentFormantFreqs = {350.0, 1200.0, 2400.0};
    
    // Parameters
    float keyTrackAmount = 1.0f;  // 0 = fixed formants, 1 = full tracking
    float formantGain = 8.0f;    // dB boost for each formant
    float formantQ = 8.0f;        // Resonance sharpness
    double baseFrequency = 100.0; // Reference frequency for scaling
    double currentFrequency = 100.0; // Current input frequency from key tracker
    
    // State
    double sampleRate = 44100.0;
    bool needsUpdate = true;
    
    // Smoothing for parameter changes
    juce::SmoothedValue<double> smoothedKeyTrack;
    juce::SmoothedValue<double> smoothedCurrentFreq;
    
    //==============================================================================
    void updateFormantCoefficients();
    double calculateTrackedFormant (double baseFormant, double scaleFactor);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FormantTracker)
};
