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
    enum class Model
    {
        Fractal,
        Spectral,
        Pluck,
        Formant,
        Comb
    };

    UniversalFilterModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Universal Filter"; }

    //==============================================================================
    // --- Parameter Setters ---
    void setModel (Model newModel);
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

    // --- Getters ---
    Model getModel() const { return currentModel; }

private:
    //==============================================================================
    void processFractal(const juce::dsp::ProcessContextReplacing<float>& context);
    void processSpectral(const juce::dsp::ProcessContextReplacing<float>& context);
    void processPluck(const juce::dsp::ProcessContextReplacing<float>& context);
    void processFormant(const juce::dsp::ProcessContextReplacing<float>& context);
    void processComb(const juce::dsp::ProcessContextReplacing<float>& context);

    void updateFilters();

    double sampleRate = 44100.0;
    Model currentModel = Model::Fractal;

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
};