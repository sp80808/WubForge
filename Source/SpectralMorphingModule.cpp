#include "SpectralMorphingModule.h"
#include <cmath>
#include <algorithm>

//==============================================================================
SpectralMorphingModule::SpectralMorphingModule()
{
    // Initialize buffers
    inputBuffer.resize(fftSize, 0.0f);
    outputBuffer.resize(fftSize, 0.0f);
    frequencyDomain.resize(fftSize / 2 + 1, {0.0f, 0.0f});
    windowBuffer.resize(fftSize, 0.0f);

    // Initialize spectral snapshots
    for (auto& snapshot : spectralSnapshots) {
        snapshot.magnitude.resize(fftSize / 2 + 1, 1.0f);
        snapshot.phase.resize(fftSize / 2 + 1, 0.0f);
    }

    // Create Hann window for overlap-add
    for (int i = 0; i < fftSize; ++i) {
        windowBuffer[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize - 1)));
    }
}

void SpectralMorphingModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    blockSize = spec.maximumBlockSize;

    // Adjust FFT size based on sample rate for optimal performance
    if (sampleRate >= 88200) {
        windowSize = 4096;
    } else if (sampleRate >= 44100) {
        windowSize = 2048;
    } else {
        windowSize = 1024;
    }

    // Resize buffers if needed
    if (inputBuffer.size() != windowSize) {
        inputBuffer.resize(windowSize, 0.0f);
        outputBuffer.resize(windowSize, 0.0f);
        frequencyDomain.resize(windowSize / 2 + 1, {0.0f, 0.0f});
        windowBuffer.resize(windowSize, 0.0f);

        // Recreate window
        for (int i = 0; i < windowSize; ++i) {
            windowBuffer[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (windowSize - 1)));
        }

        // Resize snapshots
        for (auto& snapshot : spectralSnapshots) {
            snapshot.magnitude.resize(windowSize / 2 + 1, 1.0f);
            snapshot.phase.resize(windowSize / 2 + 1, 0.0f);
        }
    }
}

void SpectralMorphingModule::reset()
{
    std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
    std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
    bufferPosition = 0;
}

void SpectralMorphingModule::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = (int)inputBlock.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Accumulate input samples
        for (size_t ch = 0; ch < inputBlock.getNumChannels(); ++ch)
        {
            float inputSample = inputBlock.getSample(ch, sample);
            inputBuffer[bufferPosition] += inputSample; // Mix channels
        }

        bufferPosition++;

        // Process when buffer is full
        if (bufferPosition >= windowSize)
        {
            bufferPosition = 0;
            performSpectralProcessing();
        }

        // Output processed sample
        for (size_t ch = 0; ch < outputBlock.getNumChannels(); ++ch)
        {
            outputBlock.setSample(ch, sample, outputBuffer[bufferPosition]);
        }
    }
}

void SpectralMorphingModule::performSpectralProcessing()
{
    // Apply window and copy to frequency domain buffer
    for (int i = 0; i < windowSize; ++i)
    {
        frequencyDomain[i] = std::complex<float>(inputBuffer[i] * windowBuffer[i], 0.0f);
    }

    // FFT analysis (simplified - would use JUCE FFT in real implementation)
    performFFT();

    // Extract magnitude and phase
    std::vector<float> currentMagnitude(frequencyDomain.size());
    std::vector<float> currentPhase(frequencyDomain.size());

    for (size_t i = 0; i < frequencyDomain.size(); ++i)
    {
        currentMagnitude[i] = std::abs(frequencyDomain[i]);
        currentPhase[i] = std::arg(frequencyDomain[i]);
    }

    // Update spectral analysis
    currentCentroid = calculateSpectralCentroid(currentMagnitude);
    currentSpectralFlux = calculateSpectralFlux(currentMagnitude);

    // Apply spectral morphing
    updateSpectralMorphing();

    // Reconstruct frequency domain
    for (size_t i = 0; i < frequencyDomain.size(); ++i)
    {
        float magnitude = spectralSnapshots[activeSourceSlot].magnitude[i] * (1.0f - morphAmount) +
                         spectralSnapshots[activeTargetSlot].magnitude[i] * morphAmount;

        float phase = spectralSnapshots[activeSourceSlot].phase[i] * (1.0f - morphAmount) +
                     spectralSnapshots[activeTargetSlot].phase[i] * morphAmount;

        // Apply phase preservation
        phase = currentPhase[i] * phasePreservation + phase * (1.0f - phasePreservation);

        frequencyDomain[i] = std::polar(magnitude, phase);
    }

    // Apply spectral warping
    applySpectralWarping();

    // IFFT synthesis (simplified)
    performIFFT();

    // Apply window and overlap-add
    for (int i = 0; i < windowSize; ++i)
    {
        outputBuffer[i] = outputBuffer[i] * 0.5f + frequencyDomain[i].real() * windowBuffer[i] * 0.5f;
    }
}

void SpectralMorphingModule::performFFT()
{
    // Simplified FFT - in real implementation, use JUCE FFT class
    // This is a placeholder for the actual FFT computation
    int N = (int)frequencyDomain.size() * 2;

    // Basic DFT implementation (for demonstration)
    std::vector<std::complex<float>> temp(N);

    for (int k = 0; k < N; ++k)
    {
        temp[k] = std::complex<float>(0.0f, 0.0f);
        for (int n = 0; n < N; ++n)
        {
            float angle = -2.0f * M_PI * k * n / N;
            temp[k] += frequencyDomain[n] * std::complex<float>(std::cos(angle), std::sin(angle));
        }
    }

    // Copy back (in real implementation, this would be in-place)
    for (int i = 0; i < N / 2 + 1; ++i)
    {
        frequencyDomain[i] = temp[i];
    }
}

void SpectralMorphingModule::performIFFT()
{
    // Simplified IFFT - placeholder for actual implementation
    int N = (int)frequencyDomain.size() * 2;

    std::vector<std::complex<float>> temp(N);

    for (int n = 0; n < N; ++n)
    {
        temp[n] = std::complex<float>(0.0f, 0.0f);
        for (int k = 0; k < N; ++k)
        {
            float angle = 2.0f * M_PI * k * n / N;
            temp[n] += frequencyDomain[k] * std::complex<float>(std::cos(angle), std::sin(angle));
        }
        temp[n] /= N;
    }

    // Copy back to frequency domain buffer
    for (int i = 0; i < N; ++i)
    {
        if (i < (int)frequencyDomain.size())
            frequencyDomain[i] = temp[i];
    }
}

void SpectralMorphingModule::updateSpectralMorphing()
{
    // Smooth morph amount changes
    morphAmount = morphAmount + (targetMorphAmount - morphAmount) * morphSpeed;

    // Update snapshot analysis
    for (auto& snapshot : spectralSnapshots)
    {
        snapshot.centroid = calculateSpectralCentroid(snapshot.magnitude);
        snapshot.spectralFlux = calculateSpectralFlux(snapshot.magnitude);
    }
}

void SpectralMorphingModule::applySpectralWarping()
{
    if (std::abs(spectralWarping) < 0.01f) return;

    // Apply spectral warping to frequency domain
    for (size_t i = 1; i < frequencyDomain.size() - 1; ++i)
    {
        float normalizedFreq = (float)i / frequencyDomain.size();
        float warpFactor = 1.0f + spectralWarping * (1.0f - normalizedFreq);

        // Interpolate with neighboring bins
        size_t warpedIndex = (size_t)(i * warpFactor);
        if (warpedIndex < frequencyDomain.size())
        {
            frequencyDomain[i] = frequencyDomain[i] * (1.0f - spectralWarping) +
                               frequencyDomain[warpedIndex] * spectralWarping;
        }
    }
}

float SpectralMorphingModule::calculateSpectralCentroid(const std::vector<float>& magnitude)
{
    float numerator = 0.0f;
    float denominator = 0.0f;

    for (size_t i = 0; i < magnitude.size(); ++i)
    {
        float freq = (float)i * sampleRate / (2.0f * magnitude.size());
        numerator += freq * magnitude[i];
        denominator += magnitude[i];
    }

    return denominator > 0.0f ? numerator / denominator : 0.0f;
}

float SpectralMorphingModule::calculateSpectralFlux(const std::vector<float>& magnitude)
{
    // Simplified spectral flux calculation
    float flux = 0.0f;
    for (size_t i = 0; i < magnitude.size(); ++i)
    {
        float diff = magnitude[i] - spectralSnapshots[activeSourceSlot].magnitude[i];
        flux += std::max(0.0f, diff);
    }
    return flux;
}

//==============================================================================
// Parameter Setters

void SpectralMorphingModule::setMorphAmount(float amount)
{
    targetMorphAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void SpectralMorphingModule::setMorphSpeed(float speed)
{
    morphSpeed = juce::jlimit(0.01f, 1.0f, speed);
}

void SpectralMorphingModule::setSpectralWarping(float warp)
{
    spectralWarping = juce::jlimit(-2.0f, 2.0f, warp);
}

void SpectralMorphingModule::setPhasePreservation(float preserve)
{
    phasePreservation = juce::jlimit(0.0f, 1.0f, preserve);
}

void SpectralMorphingModule::captureSpectralSnapshot(int slot)
{
    if (slot < 0 || slot >= 4) return;

    // Copy current spectral data to snapshot
    spectralSnapshots[slot].magnitude = std::vector<float>(frequencyDomain.size());
    spectralSnapshots[slot].phase = std::vector<float>(frequencyDomain.size());

    for (size_t i = 0; i < frequencyDomain.size(); ++i)
    {
        spectralSnapshots[slot].magnitude[i] = std::abs(frequencyDomain[i]);
        spectralSnapshots[slot].phase[i] = std::arg(frequencyDomain[i]);
    }

    spectralSnapshots[slot].centroid = currentCentroid;
    spectralSnapshots[slot].spectralFlux = currentSpectralFlux;
}

void SpectralMorphingModule::setActiveSnapshots(int sourceSlot, int targetSlot)
{
    if (sourceSlot >= 0 && sourceSlot < 4 && targetSlot >= 0 && targetSlot < 4)
    {
        activeSourceSlot = sourceSlot;
        activeTargetSlot = targetSlot;
    }
}
