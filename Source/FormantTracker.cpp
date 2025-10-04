#include "FormantTracker.h"

//==============================================================================
FormantTracker::FormantTracker()
{
    // Initialize smoothed values
    smoothedKeyTrack.reset (1.0);
    smoothedCurrentFreq.reset (100.0);
}

FormantTracker::~FormantTracker()
{
}

//==============================================================================
void FormantTracker::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare each formant filter
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 };
    
    for (auto& filter : formantFilters)
    {
        filter.prepare (spec);
        filter.reset();
    }
    
    // Initialize with default formant frequencies
    updateFormantCoefficients();
    
    // Set smoothing time constants
    smoothedKeyTrack.reset (sampleRate, 0.01); // 10ms ramp
    smoothedCurrentFreq.reset (sampleRate, 0.01);
}

void FormantTracker::reset()
{
    for (auto& filter : formantFilters)
        filter.reset();
    
    smoothedKeyTrack.reset (sampleRate, 0.01);
    smoothedCurrentFreq.reset (sampleRate, 0.01);
    needsUpdate = true;
}

//==============================================================================
void FormantTracker::process (juce::dsp::ProcessContextReplacing<float>& context)
{
    // Update smoothed parameters
    smoothedKeyTrack.setTargetValue (keyTrackAmount);
    smoothedCurrentFreq.setTargetValue (currentFrequency);
    
    // Check if we need to update coefficients
    if (needsUpdate || smoothedKeyTrack.isSmoothing() || smoothedCurrentFreq.isSmoothing())
    {
        updateFormantCoefficients();
        needsUpdate = false;
    }
    
    // Process through formant filters in series for cumulative resonance
    for (auto& filter : formantFilters)
    {
        filter.process (context);
    }
}

//==============================================================================
void FormantTracker::setKeyTrackAmount (float amount)
{
    keyTrackAmount = juce::jlimit (0.0f, 1.0f, amount);
    needsUpdate = true;
}

void FormantTracker::setFormantGain (float gainDb)
{
    formantGain = juce::jlimit (-20.0f, 20.0f, gainDb);
    needsUpdate = true;
}

void FormantTracker::setFormantQ (float Q)
{
    formantQ = juce::jlimit (0.1f, 20.0f, Q);
    needsUpdate = true;
}

void FormantTracker::setBaseFormants (const std::array<double, 3>& formants)
{
    baseFormants = formants;
    needsUpdate = true;
}

void FormantTracker::setBaseFrequency (double baseFreq)
{
    baseFrequency = juce::jlimit (20.0, 2000.0, baseFreq);
    needsUpdate = true;
}

void FormantTracker::setCurrentFrequency (double currentFreq)
{
    currentFrequency = juce::jlimit (20.0, 2000.0, currentFreq);
    needsUpdate = true;
}

//==============================================================================
void FormantTracker::updateFormantCoefficients()
{
    // Get current smoothed values
    double currentKeyTrack = smoothedKeyTrack.getCurrentValue();
    double currentFreq = smoothedCurrentFreq.getCurrentValue();
    
    // Calculate scale factor for key tracking
    // Blend between fixed (1.0) and tracked (currentFreq / baseFreq)
    double scaleFactor = currentKeyTrack * (currentFreq / baseFrequency) + (1.0 - currentKeyTrack);
    
    // Update each formant filter
    for (size_t i = 0; i < numFormants; ++i)
    {
        double trackedFormant = calculateTrackedFormant (baseFormants[i], scaleFactor);
        currentFormantFreqs[i] = trackedFormant;
        
        // Clamp to valid frequency range
        trackedFormant = juce::jlimit (50.0, sampleRate * 0.45, trackedFormant);
        
        // Create peak filter coefficients
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate, 
            trackedFormant, 
            formantQ, 
            juce::Decibels::decibelsToGain (formantGain)
        );
        
        // Update filter coefficients atomically
        formantFilters[i].coefficients = *coeffs;
    }
}

double FormantTracker::calculateTrackedFormant (double baseFormant, double scaleFactor)
{
    return baseFormant * scaleFactor;
}
