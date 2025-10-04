#include "SpectralMorphingModule.h"
#include <algorithm>
#include <cmath>

//==============================================================================
SpectralMorphingModule::SpectralMorphingModule()
{
    // Initialize with default parameters for 512-sample FFT
    updateFFTSize();
}

SpectralMorphingModule::~SpectralMorphingModule() = default;

//==============================================================================
void SpectralMorphingModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    updateFFTSize();
    initializeBuffers();
    reset();
}

void SpectralMorphingModule::reset()
{
    bufferPosition = 0;
    std::fill (inputBuffer.begin(), inputBuffer.end(), 0.0f);
    std::fill (outputBuffer.begin(), outputBuffer.end(), 0.0f);
    std::fill (spectralBuffer.begin(), spectralBuffer.end(), 0.0f);
    std::fill (magnitudeBuffer.begin(), magnitudeBuffer.end(), 0.0f);
    std::fill (phaseBuffer.begin(), phaseBuffer.end(), 0.0f);

    // Initialize source and target spectra
    sourceSpectrum.assign (fftSize / 2 + 1, 1.0f);
    targetSpectrum.assign (fftSize / 2 + 1, 1.0f);
    currentSpectrum.assign (fftSize / 2 + 1, 1.0f);
}

void SpectralMorphingModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();
    auto numChannels = outputBlock.getNumChannels();

    // Process each sample in the block
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Sum input channels for mono processing (bass optimization)
        float inputSample = 0.0f;
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            inputSample += inputBlock.getSample (ch, sample);
        }
        inputSample /= static_cast<float>(numChannels);

        // Add to input buffer for FFT processing
        inputBuffer[bufferPosition] = inputSample;

        // Process when we have enough samples for FFT
        if (bufferPosition >= hopSize - 1)
        {
            analyzeInputSpectrum();
            applySpectralMorphing();
            reconstructOutput();

            // Copy reconstructed output to all channels
            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                outputBlock.setSample (ch, sample, outputBuffer[bufferPosition]);
            }
        }
        else
        {
            // Pass through dry signal until we have enough for processing
            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                outputBlock.setSample (ch, sample, inputSample);
            }
        }

        bufferPosition = (bufferPosition + 1) % fftSize;
    }
}

//==============================================================================
void SpectralMorphingModule::setMorphPosition (float x, float y)
{
    morphX = std::max (0.0f, std::min (1.0f, x));
    morphY = std::max (0.0f, std::min (1.0f, y));
    updateMorphing();
}

void SpectralMorphingModule::setMorphTime (float timeMs)
{
    morphTime = std::max (1.0f, std::min (1000.0f, timeMs));
}

void SpectralMorphingModule::setSpectralRange (float range)
{
    spectralRange = std::max (0.0f, std::min (3.0f, range));
}

void SpectralMorphingModule::setWetDryMix (float mix)
{
    wetDryMix = std::max (0.0f, std::min (1.0f, mix));
}

void SpectralMorphingModule::setFFTSize (int size)
{
    fftSize = std::max (256, std::min (2048, size));
    updateFFTSize();
}

void SpectralMorphingModule::setOverlapFactor (float overlap)
{
    overlapFactor = std::max (0.25f, std::min (0.75f, overlap));
    hopSize = static_cast<int>(fftSize * (1.0f - overlapFactor));
}

void SpectralMorphingModule::setFormantPreservation (float amount)
{
    formantPreservation = std::max (0.0f, std::min (1.0f, amount));
}

void SpectralMorphingModule::setSpectralSmoothing (float smoothing)
{
    spectralSmoothing = std::max (0.0f, std::min (1.0f, smoothing));
}

void SpectralMorphingModule::setCurrentFreq (double freq)
{
    currentFreq = freq;
}

//==============================================================================
float SpectralMorphingModule::getCurrentLatency() const
{
    return static_cast<float>(fftSize) / static_cast<float>(sampleRate) * 1000.0f;
}

//==============================================================================
void SpectralMorphingModule::updateFFTSize()
{
    // Recreate FFT objects with new size
    fft = std::make_unique<juce::dsp::FFT> (static_cast<int>(std::log2 (fftSize)));
    ifft = std::make_unique<juce::dsp::FFT> (static_cast<int>(std::log2 (fftSize)));

    // Update hop size based on overlap
    hopSize = static_cast<int>(fftSize * (1.0f - overlapFactor));

    // Initialize all buffers
    initializeBuffers();
}

void SpectralMorphingModule::initializeBuffers()
{
    inputBuffer.resize (fftSize, 0.0f);
    outputBuffer.resize (fftSize, 0.0f);
    spectralBuffer.resize (fftSize * 2, 0.0f);  // Complex spectrum
    magnitudeBuffer.resize (fftSize / 2 + 1, 0.0f);
    phaseBuffer.resize (fftSize / 2 + 1, 0.0f);

    // Reinitialize morphing spectra
    sourceSpectrum.assign (fftSize / 2 + 1, 1.0f);
    targetSpectrum.assign (fftSize / 2 + 1, 1.0f);
    currentSpectrum.assign (fftSize / 2 + 1, 1.0f);

    // Update window buffer
    windowBuffer.resize (fftSize);
    juce::dsp::WindowingFunction<float>::fillWindowingTables (windowBuffer.data(), fftSize, juce::dsp::WindowingFunction<float>::hann, false);
}

void SpectralMorphingModule::analyzeInputSpectrum()
{
    // Copy input buffer to spectral buffer for FFT
    for (int i = 0; i < fftSize; ++i)
    {
        spectralBuffer[i * 2] = inputBuffer[i] * windowBuffer[i];     // Real part
        spectralBuffer[i * 2 + 1] = 0.0f;                             // Imaginary part
    }

    // Perform FFT
    fft->performRealOnlyForwardTransform (spectralBuffer.data(), true);

    // Extract magnitude and phase
    for (int i = 0; i <= fftSize / 2; ++i)
    {
        float real = spectralBuffer[i * 2];
        float imag = spectralBuffer[i * 2 + 1];

        magnitudeBuffer[i] = std::sqrt (real * real + imag * imag);
        phaseBuffer[i] = std::atan2 (imag, real);
    }

    // Update source spectrum for morphing (simple averaging for now)
    for (size_t i = 0; i < magnitudeBuffer.size(); ++i)
    {
        sourceSpectrum[i] = magnitudeBuffer[i];
    }
}

void SpectralMorphingModule::applySpectralMorphing()
{
    // Apply morphing between source and target spectra
    for (size_t i = 0; i < currentSpectrum.size(); ++i)
    {
        // Linear interpolation between source and target
        float morphedMagnitude = sourceSpectrum[i] * (1.0f - morphX) + targetSpectrum[i] * morphX;

        // Apply spectral range filtering (simplified)
        float rangeFactor = 1.0f;
        if (spectralRange > 0.0f)
        {
            float normalizedFreq = static_cast<float>(i) / static_cast<float>(currentSpectrum.size());
            // Simple range filtering - can be enhanced
            if (spectralRange < 1.0f) // High frequencies
                rangeFactor = 1.0f - normalizedFreq;
            else if (spectralRange < 2.0f) // Mid frequencies
                rangeFactor = 1.0f - std::abs (normalizedFreq - 0.5f) * 2.0f;
            else // Low frequencies
                rangeFactor = normalizedFreq;
        }

        // Apply formant preservation (simplified)
        float formantFactor = 1.0f;
        if (formantPreservation < 1.0f)
        {
            // Preserve formants by reducing morphing in formant regions
            float normalizedFreq = static_cast<float>(i) / static_cast<float>(currentSpectrum.size());
            float formantRegion = std::exp (-std::pow (normalizedFreq - 0.1f, 2) / 0.01f);
            formantFactor = 1.0f - (formantPreservation * formantRegion);
        }

        currentSpectrum[i] = morphedMagnitude * rangeFactor * formantFactor;
    }

    // Apply spectral smoothing if enabled
    if (spectralSmoothing > 0.0f)
    {
        std::vector<float> smoothedSpectrum = currentSpectrum;
        for (size_t i = 1; i < currentSpectrum.size() - 1; ++i)
        {
            smoothedSpectrum[i] = currentSpectrum[i] * (1.0f - spectralSmoothing) +
                                 (currentSpectrum[i - 1] + currentSpectrum[i + 1]) * 0.5f * spectralSmoothing;
        }
        currentSpectrum = smoothedSpectrum;
    }
}

void SpectralMorphingModule::reconstructOutput()
{
    // Reconstruct complex spectrum from magnitude and phase
    for (int i = 0; i <= fftSize / 2; ++i)
    {
        float magnitude = currentSpectrum[i];
        float phase = phaseBuffer[i];  // Preserve original phase for natural sound

        spectralBuffer[i * 2] = magnitude * std::cos (phase);     // Real part
        spectralBuffer[i * 2 + 1] = magnitude * std::sin (phase); // Imaginary part
    }

    // Perform inverse FFT
    ifft->performRealOnlyInverseTransform (spectralBuffer.data());

    // Remove windowing and copy to output buffer
    for (int i = 0; i < fftSize; ++i)
    {
        outputBuffer[i] = spectralBuffer[i * 2] / windowBuffer[i];
    }
}

void SpectralMorphingModule::updateMorphing()
{
    // Update target spectrum based on morph position
    // For now, create a simple target spectrum based on morph position
    for (size_t i = 0; i < targetSpectrum.size(); ++i)
    {
        float normalizedFreq = static_cast<float>(i) / static_cast<float>(targetSpectrum.size());

        // Create different spectral shapes based on morph position
        float baseShape = 1.0f / (1.0f + normalizedFreq * 2.0f);  // Low-pass like

        // Modify based on morph position
        if (morphY < 0.5f)
        {
            // Brighter sound (more high frequencies)
            baseShape *= (1.0f + morphY * 2.0f);
        }
        else
        {
            // Darker sound (less high frequencies)
            baseShape *= (1.0f - (morphY - 0.5f) * 2.0f);
        }

        targetSpectrum[i] = baseShape;
    }
}
