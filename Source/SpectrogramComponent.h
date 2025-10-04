#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

//==============================================================================
/**
    SpectrogramComponent - Real-time spectrgram visualization for WubForge

    Displays continuous spectral analysis of the audio signal:
    - Waterfall-style spectrogram showing frequency content over time
    - Logarithmic frequency scaling and magnitude color mapping
    - Configurable display settings (time window, frequency range)
    - Optimized for real-time performance with audio plugins
*/
class SpectrogramComponent : public juce::Component,
                           private juce::Timer
{
public:
    SpectrogramComponent();
    ~SpectrogramComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;

    //==============================================================================
    // Data input methods
    void pushSpectrumData(const float* magnitudeData, int size, double sampleRate);

    // Display configuration
    void setTimeWindow(float seconds);
    void setFreqRange(float minFreq, float maxFreq);
    void setColourMap(int mapType);

    // Performance settings
    void setUpdateRateHz(int rate);

    // Control
    void setEnabled(bool shouldBeEnabled);

private:
    //==============================================================================
    void timerCallback() override;
    void updateBuffers();

    //==============================================================================
    // Display parameters
    float timeWindow = 2.0f;      // Time window in seconds
    float minFrequency = 20.0f;   // Minimum frequency to display
    float maxFrequency = 20000.0f; // Maximum frequency to display
    int colourMap = 0;            // 0=Viridis, 1=Plasma, 2=Hot

    // Performance settings
    int updateRateHz = 30;        // Update rate for visualization
    bool enabled = true;

    // Data storage
    int spectrumSize = 512;       // FFT size
    int historyLength = 128;      // Number of time frames to store
    double currentSampleRate = 44100.0;

    // Spectral history buffer [frequency][time]
    std::vector<std::vector<float>> magnitudeHistory;

    //==============================================================================
    // Colour mapping
    juce::Colour getColourFromMagnitude(float magnitude, float maxMagnitude);

    // Helpers
    float freqToBin(float frequency) const;
    float binToFreq(int bin) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrogramComponent)
};
