#pragma once

#include "Module.h"
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
    A universal filter module that can switch between multiple filter models,
    including spectral, physical modeling, and classic filter types.
*/
class UniversalFilterModule : public FilterModule
{
public:
    enum class FilterAlgorithm
    {
        // Primary Tone Shaping (LPF/HPF/BPF)
        LPF_Warm,           // Gentle slope, preserves punch
        LPF_Bright,         // Steep slope, maximum headroom
        HPF_Gentle,         // Doesn't punch holes in sub
        HPF_Punch,          // Preserves bass presence
        BPF_Emphasis,       // Focus specific frequency ranges
        BPF_Notch,          // Surgical frequency removal

        // Harmonic Enhancement
        Harmonic_Uptone,    // Add octave harmonics
        Sub_Bass_Pump,      // Isolate/enhance 20-60Hz
        Resonant_Formant,   // Emulate phoneme-like resonances
        Spectral_Tilt,      // Smooth frequency response shaping

        // Dynamic Processing
        Envelope_Track,     // Frequency follows amplitude
        Transient_Boost,    // Add harmonics on attacks
        Sidechain_Compress, // Duck via external trigger
        Level_Match,        // Gain normalize across keys

        // Complex Algorithms
        MDA_RezFilter,      // Envelope-modulated resonance
        Comb_Filter_Bank,   // Phased resonances
        All_Pass_Network,   // Phase manipulation
        Time_Variant        // Changing characteristics
    };

    //==============================================================================
    // Filter Stage Types (for pipeline organization)
    enum class FilterStage
    {
        Tone_Shaping,       // Primary LPF/HPF/BPF before harmonics
        Harmonic_Enhancement, // Complex processing for richness
        Final_Sculpting,    // Final polish after harmonics
        Sub_Isolation,      // 20-60Hz processing only
        Dynamic_Processing  // Input-following behaviors
    };

    UniversalFilterModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Universal Filter"; }

    //==============================================================================
    // --- Parameter Setters ---
    void setModel (FilterAlgorithm newModel);
    void pluck(); // Special trigger for Pluck model

    // Fractal
    void setFractalType(int type); void setFractalFreq(float freq); void setFractalQ(float q); void setFractalDepth(int depth); void setFractalRatio(float ratio);
    // Spectral
    void setSpectralMode(int mode); void setSpectralFreq(float freq); void setSpectralBw(float bw);
    // Pluck
    void setPluckDecay(float decay); void setPluckDamping(float damping);
    // Formant
    void setFormantKeyTrack(float amount); void setFormantGain(float gain); void setFormantQ(float q); void setFormantBaseFreq(double freq);
    // Comb
    void setCombCount(int count); void setCombDelay(float delay); void setCombFeedback(float fb); void setCombLfoRate(float rate); void setCombLfoDepth(float depth);
    // Shaper
    void setShaperCutoff(float cutoff); void setShaperResonance(float res); void setShaperDrive(float drive);

    // --- Getters ---
    FilterAlgorithm getModel() const { return currentModel; }

private:
    //==============================================================================
    // --- Internal Processing Functions ---
    void processFractal(const juce::dsp::ProcessContextReplacing<float>& context);
    void processSpectral(const juce::dsp::ProcessContextReplacing<float>& context);
    void processPluck(const juce::dsp::ProcessContextReplacing<float>& context);
    void processFormant(const juce::dsp::ProcessContextReplacing<float>& context);
    void processComb(const juce::dsp::ProcessContextReplacing<float>& context);
    void processShaper(const juce::dsp::ProcessContextReplacing<float>& context);

    void updateFilters();

    //==============================================================================
    // --- State & Parameters ---
    double sampleRate = 44100.0;
    FilterAlgorithm currentModel = FilterAlgorithm::LPF_Warm;

    // --- DSP Components & Parameters for All Models ---

    // Fractal
    std::vector<juce::dsp::IIR::Filter<float>> fractalFilterChain;
    int fractalFilterType = 0; float fractalBaseFrequency = 1000.0f; float fractalQ = 1.0f; int fractalDepth = 4; float fractalRatio = 0.5f;
    bool fractalNeedsUpdate = true;

    // Spectral
    static constexpr int fftOrder = 11, fftSize = 1 << fftOrder, hopSize = fftSize / 4;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    std::array<float, fftSize> fftBuffer, fifo;
    std::array<float, fftSize * 2> workspace;
    juce::AudioBuffer<float> spectralOutputBuffer;
    int fifoIndex = 0, spectralOutputPos = 0;
    int spectralMode = 0; float spectralFrequency = 1000.0f; float spectralBandwidth = 100.0f;

    // Pluck
    juce::dsp::DelayLine<float> pluckDelayLine { 44100 };
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> pluckFilter;
    bool needsToPluck = true; float pluckDecay = 0.5f; float pluckDamping = 0.5f;

    // Formant (from FormantTracker)
    static constexpr int numFormants = 3;
    std::array<juce::dsp::IIR::Filter<float>, numFormants> formantFilters;
    std::array<double, 3> baseFormants = {350.0, 1200.0, 2400.0}; 
    float formantKeyTrack = 1.0f; float formantGain = 8.0f; float formantQ = 8.0f; double formantBaseFrequency = 100.0;
    bool formantNeedsUpdate = true;

    // Comb (from CombStack)
    static constexpr int maxCombCount = 8;
    std::array<juce::dsp::DelayLine<float>, maxCombCount> combDelayLines;
    juce::dsp::Oscillator<float> combLfo;
    int combCount = 6; float combDelay = 1.0f; float combFeedback = 0.7f; float combLfoRate = 1.0f; float combLfoDepth = 0.5f;

    // Shaper
    float shaperCutoff = 1000.0f, shaperResonance = 0.5f, shaperDrive = 0.0f;
    float s1 = 0.0f, s2 = 0.0f; // State variables for manual SVF
};
