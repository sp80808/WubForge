#include "SpectralFilterModule.h"

SpectralFilterModule::SpectralFilterModule()
    : forwardFFT (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    fifo.fill(0.0f);
    fftBuffer.fill(0.0f);
    workspace.fill(0.0f);
}

void SpectralFilterModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    outputBuffer.setSize(spec.numChannels, fftSize);
    reset();
}

void SpectralFilterModule::reset()
{
    fifo.fill(0.0f);
    fifoIndex = 0;
    outputBuffer.clear();
    outputBufferPos = 0;
}

void SpectralFilterModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = inputBlock.getNumSamples();
    auto numChannels = inputBlock.getNumChannels();

    for (size_t i = 0; i < numSamples; ++i)
    {
        // For simplicity, we process a mono signal by averaging the channels
        float inputSample = 0.0f;
        for (size_t ch = 0; ch < numChannels; ++ch)
            inputSample += inputBlock.getSample(ch, i);
        inputSample /= static_cast<float>(numChannels);

        pushSampleToFifo(inputSample);

        // Get the next processed sample from our output buffer
        float outputSample = outputBuffer.getSample(0, outputBufferPos);
        outputBufferPos = (outputBufferPos + 1) % fftSize;

        for (size_t ch = 0; ch < numChannels; ++ch)
            outputBlock.setSample(ch, i, outputSample);
    }
}

void SpectralFilterModule::pushSampleToFifo (float sample)
{
    fifo[fifoIndex] = sample;
    fifoIndex++;

    if (fifoIndex >= hopSize)
    {
        processFFT();
        fifoIndex = 0;
    }
}

void SpectralFilterModule::processFFT()
{
    // 1. Shift FIFO buffer
    std::rotate(fifo.begin(), fifo.begin() + hopSize, fifo.end());

    // 2. Copy latest samples to FFT buffer and apply window
    std::copy(fifo.begin() + (fftSize - hopSize), fifo.end(), fftBuffer.begin());
    window.multiplyWithWindowingTable(fftBuffer.data(), fftSize);

    // 3. Perform forward FFT
    std::copy(fftBuffer.begin(), fftBuffer.end(), workspace.begin());
    forwardFFT.performRealOnlyForwardTransform(workspace.data());

    // 4. Manipulate frequency bins
    const float binWidth = static_cast<float>(sampleRate) / fftSize;

    for (int i = 0; i < fftSize / 2; ++i)
    {
        const float currentFreq = i * binWidth;

        if (currentMode == Mode::Notch)
        {
            const float startFreq = frequency - (bandwidth / 2.0f);
            const float endFreq = frequency + (bandwidth / 2.0f);
            if (currentFreq >= startFreq && currentFreq <= endFreq)
            {
                workspace[i * 2] = 0.0f;     // Real part
                workspace[i * 2 + 1] = 0.0f; // Imaginary part
            }
        }
        else if (currentMode == Mode::Comb)
        {
            // For comb, treat 'frequency' as the fundamental and 'bandwidth' as the width of each tooth
            const float harmonicRatio = currentFreq / frequency;
            const float distanceFromHarmonic = std::abs(harmonicRatio - std::round(harmonicRatio));

            if (distanceFromHarmonic * frequency < bandwidth / 2.0f)
            {
                // This bin is part of a harmonic tooth, amplify it
                workspace[i * 2] *= 1.5f;
                workspace[i * 2 + 1] *= 1.5f;
            }
            else
            {
                // Attenuate non-harmonic bins
                workspace[i * 2] *= 0.5f;
                workspace[i * 2 + 1] *= 0.5f;
            }
        }
    }

    // 5. Perform inverse FFT
    forwardFFT.performRealOnlyInverseTransform(workspace.data());

    // 6. Overlap-add to output buffer
    for (int i = 0; i < fftSize; ++i)
    {
        int bufferPos = (outputBufferPos + i) % fftSize;
        outputBuffer.addSample(0, bufferPos, workspace[i]);
    }
}

//==============================================================================
// --- Parameter Setters ---
void SpectralFilterModule::setMode (Mode newMode) { currentMode = newMode; }
void SpectralFilterModule::setFrequency (float freqHz) { frequency = freqHz; }
void SpectralFilterModule::setBandwidth (float bwHz) { bandwidth = bwHz; }
