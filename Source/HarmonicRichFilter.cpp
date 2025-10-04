#include "HarmonicRichFilter.h"
#include <cmath>
#include <algorithm>

//==============================================================================
HarmonicRichFilter::HarmonicRichFilter()
{
    // Initialize oscillators for all three algorithms
    for (auto& osc : helicalOscillators)
        osc.initialise([] (float x) { return std::sin(x); });

    for (auto& osc : bloomModulators)
        osc.initialise([] (float x) { return std::sin(x); });

    for (auto& osc : helixOscillators)
        osc.initialise([] (float x) { return std::sin(x); });

    // Initialize LFOs
    veilLFO.initialise([] (float x) { return std::sin(x); });

    // Set initial frequencies for key tracking
    setCurrentFreq(440.0f);
}

HarmonicRichFilter::~HarmonicRichFilter() = default;

//==============================================================================
void HarmonicRichFilter::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Prepare all oscillators
    for (auto& osc : helicalOscillators)
        osc.prepare(spec);

    for (auto& osc : bloomModulators)
        osc.prepare(spec);

    for (auto& osc : helixOscillators)
        osc.prepare(spec);

    veilLFO.prepare(spec);

    // Prepare filters
    veilFilter.prepare(spec);
    helixFilter.prepare(spec);

    for (auto& filter : bloomFilters)
        filter.prepare(spec);

    for (auto& filter : allpassHelix)
        filter.prepare(spec);

    // Prepare utility classes
    veilEnvelope.prepare(sampleRate);
    performanceMonitor.prepare(sampleRate);

    // Set initial oscillator frequencies
    updateCoefficients();

    reset();
}

void HarmonicRichFilter::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto&& inputBlock = context.getInputBlock();
    auto&& outputBlock = context.getOutputBlock();
    auto numSamples = outputBlock.getNumSamples();

    // Performance monitoring
    performanceMonitor.update(numSamples);

    // Auto-Q clamping for stability
    clampQValue();

    // Process based on current filter shape
    if (mix > 0.0f)
    {
        for (size_t ch = 0; ch < outputBlock.getNumChannels(); ++ch)
        {
            auto* input = inputBlock.getChannelPointer(ch);
            auto* output = outputBlock.getChannelPointer(ch);

            switch (currentShape)
            {
                case FilterShape::HelicalSineVeil:
                    processHelicalSineVeil(input, output, numSamples);
                    break;
                case FilterShape::CascadeHarmonicBloom:
                    processCascadeHarmonicBloom(input, output, numSamples);
                    break;
                case FilterShape::SpectralSineHelix:
                    processSpectralSineHelix(input, output, numSamples);
                    break;
            }
        }
    }
}

void HarmonicRichFilter::reset()
{
    veilFilter.reset();
    helixFilter.reset();

    for (auto& filter : bloomFilters)
        filter.reset();

    for (auto& filter : allpassHelix)
        filter.reset();

    veilEnvelope.reset();
}

//==============================================================================
// Parameter Setters

void HarmonicRichFilter::setFilterShape(FilterShape shape)
{
    currentShape = shape;
    needsUpdate = true;
}

void HarmonicRichFilter::setCutoffFrequency(float freqHz)
{
    cutoffFreq = juce::jlimit(20.0f, 20000.0f, freqHz);
    needsUpdate = true;
}

void HarmonicRichFilter::setResonance(float res)
{
    resonance = juce::jlimit(0.1f, 2.0f, res);
    needsUpdate = true;
}

void HarmonicRichFilter::setDrive(float drv)
{
    drive = juce::jlimit(0.1f, 5.0f, drv);
}

void HarmonicRichFilter::setMix(float mx)
{
    mix = juce::jlimit(0.0f, 1.0f, mx);
}

void HarmonicRichFilter::setCurrentFreq(float freqHz)
{
    currentKeyFreq = juce::jlimit(20.0f, 20000.0f, freqHz);
    needsUpdate = true;
}

// Advanced parameter setters
void HarmonicRichFilter::setHelicalVeilDepth(float depth)
{
    helicalVeilDepth = juce::jlimit(0.0f, 1.0f, depth);
}

void HarmonicRichFilter::setBloomIntensity(float intensity)
{
    bloomIntensity = juce::jlimit(0.0f, 2.0f, intensity);
}

void HarmonicRichFilter::setHelixPhaseMod(float modAmount)
{
    helixPhaseMod = juce::jlimit(0.0f, 1.0f, modAmount);
}

void HarmonicRichFilter::setEnvelopeSensitivity(float sensitivity)
{
    envelopeSensitivity = juce::jlimit(0.0f, 1.0f, sensitivity);
}

//==============================================================================
// Core Filter Algorithms

void HarmonicRichFilter::processHelicalSineVeil(const float* input, float* output, int numSamples)
{
    // Helical Sine Veil: 6 parallel sine oscillators with golden-ratio spacing
    // LP veil with sine LFO modulation and envelope follower

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = input[i];
        float envelopeValue = veilEnvelope.process(std::abs(sample)) * envelopeSensitivity;

        // Generate 6 harmonically related sine waves with golden ratio spacing
        float helicalSum = 0.0f;
        for (int osc = 0; osc < maxHelicalOscillators; ++osc)
        {
            float freq = currentKeyFreq * std::pow(goldenRatio, osc) * (1.0f + envelopeValue * 0.1f);
            helicalOscillators[osc].setFrequency(freq / sampleRate);

            float oscOutput = helicalOscillators[osc].processSample(0.0f) * helicalVeilDepth;
            helicalSum += oscOutput;
        }

        // Apply LP veil filter with sine LFO modulation
        float lfoValue = veilLFO.processSample(0.0f);
        float modulatedCutoff = cutoffFreq * (1.0f + lfoValue * 0.2f);
        veilFilter.setCutoffFrequency(modulatedCutoff / sampleRate);
        veilFilter.setResonance(resonance);

        float veiledSignal = veilFilter.processSample(helicalSum);

        // Mix with dry signal
        output[i] = input[i] * (1.0f - mix) + veiledSignal * mix * drive;
    }
}

void HarmonicRichFilter::processCascadeHarmonicBloom(const float* input, float* output, int numSamples)
{
    // Cascade Harmonic Bloom: 3 serial asymmetric LP filters with per-stage sine bloom
    // and cross-feedback for organic harmonic growth

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = input[i];

        // First stage - base filtering
        float stage1Freq = cutoffFreq * 0.5f;
        bloomFilters[0].setCutoffFrequency(stage1Freq / sampleRate);
        bloomFilters[0].setResonance(resonance * 0.8f);

        float stageOutput = bloomFilters[0].processSample(sample);

        // Second stage - harmonic bloom with sine modulation
        float bloomMod = bloomModulators[0].processSample(0.0f) * bloomIntensity;
        float stage2Freq = cutoffFreq * (1.0f + bloomMod * 0.3f);
        bloomFilters[1].setCutoffFrequency(stage2Freq / sampleRate);
        bloomFilters[1].setResonance(resonance * 1.2f);

        stageOutput = bloomFilters[1].processSample(stageOutput);

        // Third stage - final bloom with feedback
        float feedback = stageOutput * bloomFeedback[2] * 0.1f;
        float stage3Freq = cutoffFreq * 1.5f;
        bloomFilters[2].setCutoffFrequency(stage3Freq / sampleRate);
        bloomFilters[2].setResonance(resonance * 0.6f);

        stageOutput = bloomFilters[2].processSample(stageOutput + feedback);

        // Update cross-feedback for next sample
        bloomFeedback[0] = stageOutput * 0.05f;
        bloomFeedback[1] = bloomFeedback[0] * 0.8f;
        bloomFeedback[2] = bloomFeedback[1] * 0.6f;

        // Mix with dry signal
        output[i] = input[i] * (1.0f - mix) + stageOutput * mix * drive;
    }
}

void HarmonicRichFilter::processSpectralSineHelix(const float* input, float* output, int numSamples)
{
    // Spectral Sine Helix: 7 sines with Gaussian-shaved LP, all-pass helix with sine-mod phase
    // Creates spectral movement with helical phase relationships

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = input[i];

        // Generate 7 sine oscillators in helical frequency relationship
        float helixSum = 0.0f;
        for (int osc = 0; osc < maxHelixSines; ++osc)
        {
            // Gaussian frequency distribution around cutoff
            float gaussianOffset = std::exp(-0.5f * std::pow((osc - 3.0f) / 2.0f, 2.0f));
            float freq = cutoffFreq * (0.5f + gaussianOffset * 2.0f);

            helixOscillators[osc].setFrequency(freq / sampleRate);

            // Phase modulation for helical movement
            float phaseMod = std::sin(osc * goldenRatio * helixPhaseMod) * 0.1f;
            float oscOutput = helixOscillators[osc].processSample(phaseMod);

            helixSum += oscOutput * gaussianOffset;
        }

        // Apply Gaussian-shaved lowpass filter
        helixFilter.setCutoffFrequency(cutoffFreq / sampleRate);
        helixFilter.setResonance(resonance);

        float filteredSignal = helixFilter.processSample(helixSum);

        // All-pass helix for phase enrichment
        for (int ap = 0; ap < maxHelixSines; ++ap)
        {
            float apFreq = cutoffFreq * (0.8f + ap * 0.1f);
            // Simple all-pass approximation using IIR filter
            float g = 0.7f; // All-pass coefficient
            filteredSignal = g * (filteredSignal - allpassHelix[ap].getLastOutput()) + allpassHelix[ap].processSample(filteredSignal);
        }

        // Mix with dry signal
        output[i] = input[i] * (1.0f - mix) + filteredSignal * mix * drive;
    }
}

//==============================================================================
// Utility Class Implementations

void HarmonicRichFilter::EnvelopeFollower::prepare(float sr)
{
    sampleRate = sr;
    setAttackTime(10.0f);  // 10ms default attack
    setReleaseTime(100.0f); // 100ms default release
}

void HarmonicRichFilter::EnvelopeFollower::reset()
{
    envelope = 0.0f;
}

float HarmonicRichFilter::EnvelopeFollower::process(float input)
{
    float rectified = std::abs(input);
    float coeff = rectified > envelope ? attackCoeff : releaseCoeff;
    envelope = (1.0f - coeff) * envelope + coeff * rectified;
    return envelope;
}

void HarmonicRichFilter::EnvelopeFollower::setAttackTime(float attackMs)
{
    float attackTime = attackMs * 0.001f; // Convert to seconds
    attackCoeff = std::exp(-1.0f / (sampleRate * attackTime));
}

void HarmonicRichFilter::EnvelopeFollower::setReleaseTime(float releaseMs)
{
    float releaseTime = releaseMs * 0.001f; // Convert to seconds
    releaseCoeff = std::exp(-1.0f / (sampleRate * releaseTime));
}

void HarmonicRichFilter::PerformanceMonitor::prepare(float sr)
{
    sampleRate = sr;
    lastUpdateTime = juce::Time::getMillisecondCounterHiRes();
}

void HarmonicRichFilter::PerformanceMonitor::update(int numSamples)
{
    samplesProcessed += numSamples;

    double currentTime = juce::Time::getMillisecondCounterHiRes();
    double timeDelta = (currentTime - lastUpdateTime) * 0.001; // Convert to seconds

    if (timeDelta >= updateInterval)
    {
        // Calculate CPU usage (simplified model)
        float expectedTime = numSamples / sampleRate;
        float actualTime = timeDelta;

        if (expectedTime > 0.0f)
        {
            cpuUsage = (actualTime / expectedTime) * 100.0f;
            cpuUsage = juce::jlimit(0.0f, 100.0f, cpuUsage);
        }

        lastUpdateTime = currentTime;
    }
}

//==============================================================================
// Helper Methods

void HarmonicRichFilter::updateCoefficients()
{
    if (!needsUpdate) return;

    // Update LFO frequency for veil modulation
    veilLFO.setFrequency(0.5f); // 0.5 Hz for gentle modulation

    // Update bloom modulator frequencies
    for (int i = 0; i < maxBloomStages; ++i)
    {
        float modFreq = 0.3f + i * 0.1f; // Staggered modulation rates
        bloomModulators[i].setFrequency(modFreq);
    }

    needsUpdate = false;
}

void HarmonicRichFilter::clampQValue()
{
    // Auto-clamp Q to prevent instability while maintaining musicality
    if (resonance < minQ) resonance = minQ;
    if (resonance > maxQ) resonance = maxQ;

    // Reduce Q if CPU usage is too high
    if (performanceMonitor.isOverloaded() && resonance > 1.0f)
    {
        resonance *= 0.95f; // Gradually reduce Q
    }
}

float HarmonicRichFilter::calculateGoldenRatioPhase(int oscillatorIndex)
{
    return std::fmod(oscillatorIndex * goldenRatio * juce::MathConstants<float>::twoPi, juce::MathConstants<float>::twoPi);
}

float HarmonicRichFilter::processWithSaturation(float input, float driveAmount)
{
    // Soft saturation curve for harmonic enhancement
    float saturated = input * driveAmount;
    return std::tanh(saturated) / std::tanh(1.0f) * (1.0f / driveAmount);
}