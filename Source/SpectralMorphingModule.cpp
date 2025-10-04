#include "SpectralMorphingModule.h"
#include <cmath>
#include <algorithm>

//==============================================================================
SpectralMorphingModule::SpectralMorphingModule()
{
    // Create Hann window for overlap-add (shared across channels)
    windowBuffer.resize(fftSize);
    for (int i = 0; i < fftSize; ++i) {
        windowBuffer[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize - 1)));
    }
}

void SpectralMorphingModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    blockSize = spec.maximumBlockSize;
    numChannels = (int)spec.numChannels;

    // Adjust FFT size based on sample rate for optimal performance
    // (Keeping original logic for now, but windowSize should ideally be fftSize constant)
    if (sampleRate >= 88200) {
        windowSize = 4096;
    } else if (sampleRate >= 44100) {
        windowSize = 2048;
    } else {
        windowSize = 1024;
    }

    // Resize per-channel buffers and FFT objects
    inputBuffers.resize(numChannels);
    outputBuffers.resize(numChannels);
    frequencyDomains.resize(numChannels);
    forwardFFTs.clear(); // Clear existing FFTs
    forwardFFTs.reserve(numChannels);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        inputBuffers[ch].resize(windowSize, 0.0f);
        outputBuffers[ch].resize(windowSize, 0.0f);
        frequencyDomains[ch].resize(windowSize / 2 + 1, {0.0f, 0.0f});
        forwardFFTs.emplace_back(fftOrder); // Initialize JUCE FFT
    }

    // Resize snapshots for each channel
    for (auto& snapshot : spectralSnapshots)
    {
        snapshot.magnitude.resize(numChannels);
        snapshot.phase.resize(numChannels);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            snapshot.magnitude[ch].resize(windowSize / 2 + 1, 1.0f);
            snapshot.phase[ch].resize(windowSize / 2 + 1, 0.0f);
        }
    }
}

void SpectralMorphingModule::reset()
{
    for (int ch = 0; ch < numChannels; ++ch)
    {
        std::fill(inputBuffers[ch].begin(), inputBuffers[ch].end(), 0.0f);
        std::fill(outputBuffers[ch].begin(), outputBuffers[ch].end(), 0.0f);
    }
    bufferPosition = 0;
}

void SpectralMorphingModule::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = (int)inputBlock.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float inputSample = inputBlock.getSample(ch, sample);
            inputBuffers[ch][bufferPosition] = inputSample; // Store per channel
        }

        bufferPosition++;

        // Process when buffer is full
        if (bufferPosition >= windowSize)
        {
            bufferPosition = 0;
            performSpectralProcessing(); // This will now process all channels
        }

        // Output processed sample
        for (int ch = 0; ch < numChannels; ++ch)
        {
            outputBlock.setSample(ch, sample, outputBuffers[ch][bufferPosition]);
        }
    }
}

void SpectralMorphingModule::performSpectralProcessing()
{
    for (int ch = 0; ch < numChannels; ++ch)
    {
        // Apply window and copy to frequency domain buffer
        std::vector<float> fftInput(windowSize);
        for (int i = 0; i < windowSize; ++i)
        {
            fftInput[i] = inputBuffers[ch][i] * windowBuffer[i];
        }

        // FFT analysis using JUCE FFT
        forwardFFTs[ch].performRealOnlyForwardTransform(fftInput.data());

        // Copy to frequencyDomains for magnitude/phase extraction
        for (int i = 0; i < windowSize / 2 + 1; ++i)
        {
            frequencyDomains[ch][i] = std::complex<float>(fftInput[i * 2], fftInput[i * 2 + 1]);
        }

        // Extract magnitude and phase
        std::vector<float> currentMagnitude(frequencyDomains[ch].size());
        std::vector<float> currentPhase(frequencyDomains[ch].size());

        for (size_t i = 0; i < frequencyDomains[ch].size(); ++i)
        {
            currentMagnitude[i] = std::abs(frequencyDomains[ch][i]);
            currentPhase[i] = std::arg(frequencyDomains[ch][i]);
        }

        // Update spectral analysis (can be per-channel or averaged later)
        // For now, let's assume these are still calculated for a single channel or averaged
        currentCentroid = calculateSpectralCentroid(currentMagnitude);
        currentSpectralFlux = calculateSpectralFlux(currentMagnitude);

        // Apply spectral morphing
        updateSpectralMorphing(); // This method needs to be updated to handle per-channel snapshots

        // Reconstruct frequency domain
        for (size_t i = 0; i < frequencyDomains[ch].size(); ++i)
        {
            float magnitude = spectralSnapshots[activeSourceSlot].magnitude[ch][i] * (1.0f - morphAmount) +
                             spectralSnapshots[activeTargetSlot].magnitude[ch][i] * morphAmount;

            float phase = spectralSnapshots[activeSourceSlot].phase[ch][i] * (1.0f - morphAmount) +
                         spectralSnapshots[activeTargetSlot].phase[ch][i] * morphAmount;

            // Apply phase preservation
            phase = currentPhase[i] * phasePreservation + phase * (1.0f - phasePreservation);

            frequencyDomains[ch][i] = std::polar(magnitude, phase);
        }

        // Apply spectral warping
        applySpectralWarping(); // This method needs to be updated to handle per-channel frequency domains

        // IFFT synthesis using JUCE FFT
        std::vector<float> ifftOutput(windowSize);
        for (int i = 0; i < windowSize / 2 + 1; ++i)
        {
            ifftOutput[i * 2] = frequencyDomains[ch][i].real();
            ifftOutput[i * 2 + 1] = frequencyDomains[ch][i].imag();
        }
        forwardFFTs[ch].performRealOnlyInverseTransform(ifftOutput.data());

        // Apply window and overlap-add
        for (int i = 0; i < windowSize; ++i)
        {
            outputBuffers[ch][i] = outputBuffers[ch][i] * 0.5f + ifftOutput[i] * windowBuffer[i] * 0.5f;
        }
    }
}



void SpectralMorphingModule::updateSpectralMorphing()
{
    // Smooth morph amount changes
    morphAmount = morphAmount + (targetMorphAmount - morphAmount) * morphSpeed;

    // Update snapshot analysis (per-channel)
    for (auto& snapshot : spectralSnapshots)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            snapshot.centroid = calculateSpectralCentroid(snapshot.magnitude[ch]);
            snapshot.spectralFlux = calculateSpectralFlux(snapshot.magnitude[ch]);
        }
    }
}

void SpectralMorphingModule::applySpectralWarping()
{
    if (std::abs(spectralWarping) < 0.01f) return;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        // Apply spectral warping to frequency domain
        for (size_t i = 1; i < frequencyDomains[ch].size() - 1; ++i)
        {
            float normalizedFreq = (float)i / frequencyDomains[ch].size();
            float warpFactor = 1.0f + spectralWarping * (1.0f - normalizedFreq);

            // Interpolate with neighboring bins
            size_t warpedIndex = (size_t)(i * warpFactor);
            if (warpedIndex < frequencyDomains[ch].size())
            {
                frequencyDomains[ch][i] = frequencyDomains[ch][i] * (1.0f - spectralWarping) +
                                       frequencyDomains[ch][warpedIndex] * spectralWarping;
            }
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

    // Resize snapshot vectors for current number of channels
    spectralSnapshots[slot].magnitude.resize(numChannels);
    spectralSnapshots[slot].phase.resize(numChannels);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        // Copy current spectral data to snapshot for each channel
        spectralSnapshots[slot].magnitude[ch].resize(frequencyDomains[ch].size());
        spectralSnapshots[slot].phase[ch].resize(frequencyDomains[ch].size());

        for (size_t i = 0; i < frequencyDomains[ch].size(); ++i)
        {
            spectralSnapshots[slot].magnitude[ch][i] = std::abs(frequencyDomains[ch][i]);
            spectralSnapshots[slot].phase[ch][i] = std::arg(frequencyDomains[ch][i]);
        }
    }

    // Centroid and spectral flux might need to be per-channel or averaged,
    // keeping as is for now, assuming they are calculated from a single channel or averaged.
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
