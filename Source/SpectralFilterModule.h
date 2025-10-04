#pragma once

#include "Module.h"
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
    A filter that operates in the frequency domain to create sharp spectral
    notches or resonant combs.
*/
class SpectralFilterModule : public FilterModule
{
public:
    enum class Mode { Notch, Comb };

    SpectralFilterModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Spectral Filter"; }

    //==============================================================================
    // Parameter Setters
    void setMode (Mode newMode);
    void setFrequency (float freqHz);
    void setBandwidth (float bwHz);

private:
    void pushSampleToFifo (float sample);
    void processFFT();

    // --- FFT Setup ---
    static constexpr int fftOrder = 11; // 2048
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    // --- Buffers ---
    std::array<float, fftSize> fftBuffer;
    std::array<float, fftSize * 2> workspace;
    std::array<float, fftSize> fifo;
    int fifoIndex = 0;

    juce::AudioBuffer<float> outputBuffer;
    int outputBufferPos = 0;

    // --- State & Parameters ---
    double sampleRate = 44100.0;
    Mode currentMode = Mode::Notch;
    float frequency = 1000.0f;
    float bandwidth = 100.0f;
};