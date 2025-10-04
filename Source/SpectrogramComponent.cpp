#include "SpectrogramComponent.h"
#include <cmath>

//==============================================================================
SpectrogramComponent::SpectrogramComponent()
{
    updateBuffers();
    startTimerHz(updateRateHz);
}

SpectrogramComponent::~SpectrogramComponent()
{
    stopTimer();
}

//==============================================================================
void SpectrogramComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Fill background
    g.fillAll(juce::Colours::black);

    if (!enabled || magnitudeHistory.empty() || magnitudeHistory[0].empty())
    {
        // Draw placeholder text
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(16.0f);
        g.drawText("Spectrogram - Analyzing Audio...",
                  bounds, juce::Justification::centred, false);
        return;
    }

    // Find maximum magnitude in history for normalization
    float maxMagnitude = 0.0f;
    for (const auto& freqSlice : magnitudeHistory)
        for (float mag : freqSlice)
            maxMagnitude = std::max(maxMagnitude, mag);

    if (maxMagnitude <= 0.0f)
        maxMagnitude = 1.0f; // Avoid division by zero

    // Draw spectrogram
    int width = bounds.getWidth();
    int height = bounds.getHeight();

    int freqBins = magnitudeHistory.size();
    int timeSlices = magnitudeHistory[0].size();

    if (timeSlices == 0 || freqBins == 0)
        return;

    // Calculate logarithmic frequency mapping
    auto freqToY = [this, height](float frequency) -> int {
        if (frequency <= minFrequency)
            return height - 1;

        // Logarithmic mapping from minFrequency to maxFrequency
        float logMin = std::log10(minFrequency);
        float logMax = std::log10(maxFrequency);
        float logFreq = std::log10(frequency);

        float normalizedY = 1.0f - (logFreq - logMin) / (logMax - logMin);
        return juce::jlimit(0, height - 1, (int)(normalizedY * height));
    };

    // Draw each column (time slice)
    for (int timeIndex = 0; timeIndex < timeSlices; ++timeIndex)
    {
        int x = timeIndex * width / timeSlices;

        for (int freqIndex = 0; freqIndex < freqBins; ++freqIndex)
        {
            float magnitude = magnitudeHistory[freqIndex][timeIndex];
            float freq = binToFreq(freqIndex);

            // Skip frequencies outside our range
            if (freq < minFrequency || freq > maxFrequency)
                continue;

            juce::Colour colour = getColourFromMagnitude(magnitude, maxMagnitude);

            int yStart = freqToY(freq);
            int yEnd = freqToY(binToFreq(freqIndex + 1));

            // Ensure minimum pixel height for visibility
            int pixelHeight = std::max(1, yEnd - yStart);
            g.setColour(colour);

            // Draw vertical line for this frequency bin
            g.fillRect(x, yStart, 1, pixelHeight);
        }
    }

    // Draw frequency labels
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(10.0f);

    // Draw octave markers
    for (int octave = 4; octave <= 14; ++octave) // 16Hz to 16kHz in octaves
    {
        float freq = std::pow(2.0f, octave) * 16.0f / 16.0f; // Start from 16Hz
        if (freq >= minFrequency && freq <= maxFrequency)
        {
            int y = freqToY(freq);
            g.drawLine(0, y, width, y, 1.0f);

            juce::String label;
            if (freq < 1000.0f)
                label = juce::String((int)freq) + "Hz";
            else
                label = juce::String(freq / 1000.0f, 1) + "k";

            g.drawText(label, juce::Rectangle<int>(0, y - 8, 40, 16),
                      juce::Justification::centredLeft, false);
        }
    }

    // Draw time markers
    float timePerSlice = timeWindow / timeSlices;
    g.setFont(9.0f);
    for (int marker = 0; marker <= 4; ++marker)
    {
        float time = marker * timeWindow / 4.0f;
        int x = marker * width / 4;

        g.drawLine(x, 0, x, 8, 1.0f);
        g.drawText(juce::String(time, 1) + "s", juce::Rectangle<int>(x - 15, height - 15, 30, 12),
                  juce::Justification::centred, false);
    }
}

//==============================================================================
void SpectrogramComponent::pushSpectrumData(const float* magnitudeData, int size, double sampleRate)
{
    if (!enabled || magnitudeData == nullptr || size <= 0)
        return;

    currentSampleRate = sampleRate;

    // Resize our history buffer if spectrum size changed
    if (spectrumSize != size)
    {
        spectrumSize = size;
        updateBuffers();
    }

    // Shift existing data to the left (older data)
    for (auto& freqSlice : magnitudeHistory)
    {
        if (!freqSlice.empty())
        {
            for (size_t i = 0; i < freqSlice.size() - 1; ++i)
                freqSlice[i] = freqSlice[i + 1];
        }
    }

    // Add new data to the end
    for (int i = 0; i < size; ++i)
    {
        if (i < (int)magnitudeHistory.size())
        {
            if (!magnitudeHistory[i].empty())
                magnitudeHistory[i].back() = magnitudeData[i];
        }
    }

    // Repaint will be handled by the timer callback
}

//==============================================================================
void SpectrogramComponent::setTimeWindow(float seconds)
{
    timeWindow = juce::jmax(0.1f, juce::jmin(10.0f, seconds));
    updateBuffers();
}

void SpectrogramComponent::setFreqRange(float minFreq, float maxFreq)
{
    minFrequency = juce::jmax(10.0f, minFreq);
    maxFrequency = juce::jmin(22000.0f, juce::jmax(minFrequency + 100.0f, maxFreq));
}

void SpectrogramComponent::setColourMap(int mapType)
{
    colourMap = juce::jlimit(0, 2, mapType);
}

void SpectrogramComponent::setUpdateRateHz(int rate)
{
    updateRateHz = juce::jlimit(5, 60, rate);
    restartTimer();
}

void SpectrogramComponent::setEnabled(bool shouldBeEnabled)
{
    enabled = shouldBeEnabled;
}

//==============================================================================
void SpectrogramComponent::timerCallback()
{
    repaint();
}

void SpectrogramComponent::updateBuffers()
{
    if (spectrumSize <= 0)
        return;

    // Calculate history length based on time window and update rate
    historyLength = juce::jmax(32, (int)(timeWindow * updateRateHz));

    // Resize the magnitude history buffer
    magnitudeHistory.resize(spectrumSize);

    for (auto& freqSlice : magnitudeHistory)
        freqSlice.resize(historyLength, 0.0f);
}

//==============================================================================
juce::Colour SpectrogramComponent::getColourFromMagnitude(float magnitude, float maxMagnitude)
{
    float normalizedMag = juce::jlimit(0.0f, 1.0f, magnitude / maxMagnitude);

    switch (colourMap)
    {
        case 0: // Viridis-like colormap (blue to yellow/green)
        default:
            return juce::Colour::fromHSV(0.66f - normalizedMag * 0.66f, 1.0f, normalizedMag * 0.8f + 0.2f, 1.0f);

        case 1: // Plasma-like colormap (purple to orange)
            return juce::Colour::fromHSV(0.8f - normalizedMag * 0.8f, 1.0f, normalizedMag * 0.7f + 0.3f, 1.0f);

        case 2: // Hot colormap (black to red to yellow to white)
            if (normalizedMag < 0.33f)
                return juce::Colour(normalizedMag * 3.0f, 0.0f, 0.0f);
            else if (normalizedMag < 0.66f)
                return juce::Colour(1.0f, (normalizedMag - 0.33f) * 3.0f, 0.0f);
            else
                return juce::Colour(1.0f, 1.0f, (normalizedMag - 0.66f) * 3.0f);
    }
}

//==============================================================================
float SpectrogramComponent::freqToBin(float frequency) const
{
    // Convert frequency to FFT bin index
    return frequency * spectrumSize / currentSampleRate;
}

float SpectrogramComponent::binToFreq(int bin) const
{
    // Convert FFT bin index to frequency
    return (float)bin * currentSampleRate / spectrumSize;
}
