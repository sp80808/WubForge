#include "WavetableFilterModule.h"
#include <cmath>

// Constructor
WavetableFilterModule::WavetableFilterModule()
{
    // Initialize wavetable buffer for modulation source
    wavetable.setSize(1, wavetableSize);
    wavetable.clear();

    // Initialize complex modulation LFO
    updateLfoShape();

    // Initialize envelope follower
    envelopeFollower.setAttackTime(1.0f);
    envelopeFollower.setReleaseTime(50.0f);

    // Set default parameters for complex harmonic behavior
    baseCutoff = 800.0f;
    resonance = 0.7f;
    lfoFrequency = 0.5f;
    wavetableModDepth = 0.5f;
    lfoDepth = 1.0f;
    wetMix = 0.9f;

    // Create default complex digital wavetable for robotic harmonics
    createDefaultDigitalWavetable();
}

// Destructor
WavetableFilterModule::~WavetableFilterModule() = default;

// AudioModule interface implementation
void WavetableFilterModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Prepare filter
    filter.prepare(spec);
    filter.parameters->setCutOffFrequency(sampleRate, baseCutoff, resonance);
    filter.parameters->type = juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass;

    // Prepare LFO
    lfo.prepare(spec);
    updateLfoShape();

    // Prepare envelope follower
    envelopeFollower.prepare(spec);
    updateEnvelopeCoefficients();

    // Reset state
    reset();
}

void WavetableFilterModule::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto inputBlock = context.getInputBlock();
    auto outputBlock = context.getOutputBlock();
    auto numSamples = (int)inputBlock.getNumSamples();
    auto numChannels = (int)inputBlock.getNumChannels();

    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel) {
        auto* input = inputBlock.getChannelPointer(channel);
        auto* output = outputBlock.getChannelPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample) {
            float inputSample = input[sample];

            // Process envelope follower for input dynamics
            float envelopeLevel = envelopeFollower.processSample(channel, inputSample * envelopeSensitivity);

            // Get complex modular modulation amounts (for evolving digital harmonics)
            float lfoMod = processLfoModulation();
            float wtMod = processWavetableModulation();
            float envMod = envelopeLevel * 2.0f - 1.0f; // Convert to bipolar modulation

            // **Complex Digital Harmonic Modulation Algorithm**
            // Create evolving robotic textures through multifaceted modulation

            // Layer 1: FM-style wavetable modulation (creates artificial harmonics)
            float fmHarmonic = wtMod * 0.8f + lfoMod * 0.2f;
            float harmonicCutoff = baseCutoff * (1.0f + fmHarmonic * 2.0f); // Aggressive modulation for digital sound

            // Layer 2: Envelope-driven spectral movement (creates evolving sweeps)
            float spectralMod = envMod * wtMod; // Interaction creates unpredictable movement
            float spectralCutoff = harmonicCutoff * (1.0f + spectralMod * 1.5f);

            // Layer 3: Phase-modulated distortion of filter frequency (digital artifacts)
            static float phaseAccumulator = 0.0f;
            phaseAccumulator += 0.1f * wavetableIncrement;
            float phaseMod = std::sin(phaseAccumulator * wtMod * 0.5f); // FM-like artifacts
            float finalCutoff = spectralCutoff * (1.0f + phaseMod * 0.3f);

            // Clamp to extreme ranges for more radical harmonic content
            float minCutoff = 20.0f;
            float maxCutoff = 18000.0f;
            finalCutoff = juce::jlimit(minCutoff, maxCutoff, finalCutoff);

            // Dynamic resonance based on modulation intensity (creates robotic emphasis)
            float dynamicRes = resonance + (fabsf(wtMod) * 0.4f);
            dynamicRes = juce::jlimit(0.1f, 1.0f, dynamicRes);

            // Set filter parameters with complex modulation
            filter.parameters->setCutOffFrequency(sampleRate, finalCutoff, dynamicRes);

            // Process through filter
            float filteredSample = filter.processSample(inputSample);

            // Mix with complex digital character
            // Add subtle digital artifacts through dry/wet manipulation
            float digitalArtifact = std::fmod(finalCutoff, 1000.0f) * 0.001f; // Frequency-based artifacts
            float processedWet = filteredSample * (1.0f + digitalArtifact * wtMod);

            output[sample] = inputSample * (1.0f - wetMix) + processedWet * wetMix;
        }
    }
}

void WavetableFilterModule::reset()
{
    filter.reset();
    lfo.reset();
    envelopeFollower.reset();

    wavetablePosition = 0.0f;
    lfoPhase = 0.0f;
}

// Wavetable Management
bool WavetableFilterModule::loadWavetableFromAudioFile(const juce::File& file)
{
    // TODO: Fix JUCE API compatibility - AudioFormatManager not available in current JUCE version
    juce::Logger::writeToLog("WavetableFilterModule: AudioFormatManager not available - needs JUCE API compatibility fix");

    return false;
}

void WavetableFilterModule::unloadWavetable()
{
    wavetable.clear();
    wavetableLoaded = false;
    loadedWavetableName = "";
}

// Parameter setters
void WavetableFilterModule::setCutoffFrequency(float frequencyHz)
{
    baseCutoff = juce::jlimit(20.0f, 20000.0f, frequencyHz);
}

void WavetableFilterModule::setResonance(float resonanceAmount)
{
    resonance = juce::jlimit(0.0f, 1.0f, resonanceAmount);
}

void WavetableFilterModule::setFilterType(int type)
{
    filterType = juce::jlimit(0, 3, type);

    juce::dsp::StateVariableFilter::Parameters<float>::Type filterTypeEnum;

    switch (filterType) {
        case 0: filterTypeEnum = juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass; break;
        case 1: filterTypeEnum = juce::dsp::StateVariableFilter::Parameters<float>::Type::highPass; break;
        case 2: filterTypeEnum = juce::dsp::StateVariableFilter::Parameters<float>::Type::bandPass; break;
        case 3: filterTypeEnum = juce::dsp::StateVariableFilter::Parameters<float>::Type::bandPass; break;
        default: filterTypeEnum = juce::dsp::StateVariableFilter::Parameters<float>::Type::lowPass; break;
    }

    filter.parameters->type = filterTypeEnum;
}

void WavetableFilterModule::setLfoRate(float rateHz)
{
    lfoFrequency = juce::jlimit(0.01f, 20.0f, rateHz);
}

void WavetableFilterModule::setLfoShape(int shape)
{
    lfoShape = juce::jlimit(0, 4, shape);
    updateLfoShape();
}

void WavetableFilterModule::setLfoDepth(float depth)
{
    lfoDepth = juce::jlimit(0.0f, 1.0f, depth);
}

void WavetableFilterModule::setWavetableModDepth(float depth)
{
    wavetableModDepth = juce::jlimit(0.0f, 1.0f, depth);
}

void WavetableFilterModule::setWavetableRate(float rate)
{
    wavetableRate = juce::jlimit(0.01f, 10.0f, rate);
    wavetableIncrement = wavetableRate;
}

void WavetableFilterModule::setWavetablePosition(float position)
{
    wavetablePosition = position * (float)wavetableSize;
}

void WavetableFilterModule::setEnvelopeSensitivity(float sensitivity)
{
    envelopeSensitivity = juce::jlimit(0.0f, 1.0f, sensitivity);
}

void WavetableFilterModule::setEnvelopeAttack(float attackMs)
{
    envelopeAttackMs = juce::jlimit(0.1f, 1000.0f, attackMs);
    updateEnvelopeCoefficients();
}

void WavetableFilterModule::setEnvelopeRelease(float releaseMs)
{
    envelopeReleaseMs = juce::jlimit(0.1f, 1000.0f, releaseMs);
    updateEnvelopeCoefficients();
}

void WavetableFilterModule::setWetMix(float mix)
{
    wetMix = juce::jlimit(0.0f, 1.0f, mix);
}

// Private methods implementation

void WavetableFilterModule::updateLfoShape()
{
    switch (lfoShape) {
        case 0: // Saw
            lfo.initialise([](float x) { return 2.0f * (x / (2.0f * juce::MathConstants<float>::pi) - 0.5f); }, 128);
            break;
        case 1: // Triangle
            lfo.initialise([](float x) { return (2.0f / juce::MathConstants<float>::pi) * std::asin(std::sin(x)); }, 128);
            break;
        case 2: // Sine
            lfo.initialise([](float x) { return std::sin(x); }, 128);
            break;
        case 3: // Square
            lfo.initialise([](float x) { return std::sin(x) > 0.0f ? 1.0f : -1.0f; }, 128);
            break;
        case 4: // Random (approximated with noise-like function)
            lfo.initialise([](float x) { return 2.0f * (std::sin(x * 7.0f) * std::cos(x * 3.0f)) - 1.0f; }, 128);
            break;
        default:
            lfo.initialise([](float x) { return std::sin(x); }, 128);
            break;
    }
}

void WavetableFilterModule::updateEnvelopeCoefficients()
{
    // Convert milliseconds to coefficient for 1-pole filter
    float attackTime = envelopeAttackMs / 1000.0f;
    float releaseTime = envelopeReleaseMs / 1000.0f;

    // These would need to be implemented in the BallisticsFilter or use a custom envelope follower
    // For now, we'll use simple coefficient calculation
    float attackCoeff = std::exp(-1.0f / (attackTime * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (releaseTime * sampleRate));

    // Note: BallisticsFilter may have its own methods for setting attack/release
    // This is a simplified implementation
}

float WavetableFilterModule::getWavetableSample()
{
    if (!wavetableLoaded) {
        return 0.0f; // Return silence if no wavetable loaded
    }

    // Get interpolated sample from wavetable
    float samplePos = wavetablePosition;
    int index1 = (int)samplePos;
    int index2 = (index1 + 1) % wavetableSize;
    float frac = samplePos - (float)index1;

    float sample1 = wavetable.getSample(0, index1);
    float sample2 = wavetable.getSample(0, index2);

    // Linear interpolation
    return sample1 + frac * (sample2 - sample1);
}

float WavetableFilterModule::processLfoModulation()
{
    if (lfoDepth <= 0.0f) {
        return 0.0f;
    }

    // Generate LFO sample
    float lfoSample = lfo.processSample(lfoFrequency);

    // Apply depth and return
    return lfoSample * lfoDepth;
}

float WavetableFilterModule::processEnvelopeModulation()
{
    // This method is now handled in the main process() method using the BallisticsFilter directly
    // The envelope follower processes the input signal and creates the envMod value there
    return 0.0f;
}

float WavetableFilterModule::processWavetableModulation()
{
    if (!wavetableLoaded || wavetableModDepth <= 0.0f) {
        return 0.0f;
    }

    // Get sample from wavetable
    float wtSample = getWavetableSample();

    // Advance wavetable position
    wavetablePosition += wavetableIncrement;
    if (wavetablePosition >= wavetableSize) {
        wavetablePosition -= wavetableSize;
    }

    // Apply depth and return
    return wtSample * wavetableModDepth;
}

// Private method: Create complex digital wavetable for robotic harmonics
void WavetableFilterModule::createDefaultDigitalWavetable()
{
    // Clear existing wavetable
    wavetable.clear();

    // Generate complex digital waveform that creates evolving robotic harmonics
    for (int i = 0; i < wavetableSize; ++i) {
        float phase = (float)i / (float)wavetableSize; // 0.0 to 1.0

        // **Complex Digital Harmonic Algorithm**
        // Create evolving robotic textures with multiple interacting harmonics

        // Primary harmonics (create metallic/digital character)
        float primaryHarmonic = std::sin(phase * juce::MathConstants<float>::twoPi) * 0.6f;
        float secondaryHarmonic = std::sin(phase * juce::MathConstants<float>::twoPi * 2.0f) * 0.4f;
        float tertiaryHarmonic = std::sin(phase * juce::MathConstants<float>::twoPi * 3.0f) * 0.3f;

        // Phase-modulated harmonics (create evolving digital sweeps)
        float phaseMod1 = std::sin(phase * juce::MathConstants<float>::twoPi * 7.0f) * 0.1f;
        float phaseMod2 = std::cos(phase * juce::MathConstants<float>::twoPi * 11.0f) * 0.08f;

        // Harmonic interaction (creates unpredictable robotic movement)
        float interaction1 = primaryHarmonic * secondaryHarmonic * 0.5f;
        float interaction2 = secondaryHarmonic * tertiaryHarmonic * 0.3f;

        // Pulsing envelope (creates rhythmic digital character)
        float envelope = 0.5f + 0.5f * std::sin(phase * juce::MathConstants<float>::twoPi * 8.0f);

        // Combine all elements for complex digital texture
        float digitalWavetable = primaryHarmonic +
                                secondaryHarmonic +
                                tertiaryHarmonic +
                                interaction1 +
                                interaction2 +
                                phaseMod1 +
                                phaseMod2;

        // Apply envelope and normalize
        digitalWavetable *= envelope;
        digitalWavetable = juce::jlimit(-1.0f, 1.0f, digitalWavetable);

        // Store in wavetable buffer
        wavetable.setSample(0, i, digitalWavetable * 0.3f); // Scale down for modulation intensity
    }

    // Mark as loaded
    wavetableLoaded = true;
    loadedWavetableName = "Complex Digital Harmonic";
}
