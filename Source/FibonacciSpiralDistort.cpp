#include "FibonacciSpiralDistort.h"
#include <cmath>

// Constructor
FibonacciSpiralDistort::FibonacciSpiralDistort()
{
    // Initialize Fibonacci ratios (n=5 to n=15)
    updateFibonacciRatios();

    // Initialize resonator bank
    updateResonatorBank();

    // Initialize distortion stages
    for (auto& stage : distortionStages) {
        stage.drive = 1.0f;
        stage.envelope = 0.0f;
        stage.attackCoeff = 0.999f;
        stage.releaseCoeff = 0.999f;
    }

    // Initialize veil filters (helical LP with key-scaled cutoff)
    for (auto& filter : veilFilters) {
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 800.0f);
        filter.filter.coefficients = filter.coefficients;
    }

    // Initialize resonator BP filters with self-oscillation
    for (auto& resonator : resonators) {
        resonator.bpCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(44100.0, 100.0f, 2.0f);
        resonator.bpFilter.coefficients = resonator.bpCoefficients;
        resonator.feedback = 0.9f;
        resonator.prevOutput = 0.0f;
    }

    // Initialize A-weighting filter for psychoacoustic processing (20Hz-20kHz bandpass)
    aWeightCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 1000.0f, 0.5f);
    aWeightFilter.coefficients = aWeightCoefficients;

    // Initialize block processing for RMS/A-weighted RMS calculation
    blockSize = 512;
    rmsBuffer.resize(blockSize, 0.0f);
    aWeightBuffer.resize(blockSize, 0.0f);
    blockPosition = 0;

    // Initialize Fib envelope follower for psycho-smoothed processing
    fibEnvelope = 0.0f;
    fibAlpha = 0.619f; // 13/21 Fibonacci ratio for smooth decay
}

// Destructor
FibonacciSpiralDistort::~FibonacciSpiralDistort() = default;

// AudioModule interface implementation
void FibonacciSpiralDistort::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Update all coefficients for new sample rate
    updateEnvelopeCoefficients();
    updateResonatorBank();
    updateVeilFilterCutoffs();

    // Reset DSP state
    reset();
}

void FibonacciSpiralDistort::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto inputBlock = context.getInputBlock();
    auto outputBlock = context.getOutputBlock();
    auto numSamples = (int)inputBlock.getNumSamples();
    auto numChannels = (int)inputBlock.getNumChannels();

    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel) {
        auto* input = inputBlock.getChannelPointer(channel);
        auto* output = outputBlock.getChannelPointer(channel);

        // Create working buffer for processing
        std::vector<float> workingBuffer(numSamples);
        std::copy(input, input + numSamples, workingBuffer.data());

        // Apply FSD algorithm stages
        processResonatorBank(input, workingBuffer.data(), numSamples);
        processFibonacciDistortion(workingBuffer.data(), numSamples);
        processSpiralVeilFilter(workingBuffer.data(), numSamples);

        // Mix with dry signal
        float wetMix = 0.8f; // TODO: Make this a parameter
        for (int sample = 0; sample < numSamples; ++sample) {
            output[sample] = input[sample] * (1.0f - wetMix) + workingBuffer[sample] * wetMix;
        }
    }
}

void FibonacciSpiralDistort::reset()
{
    // Reset resonator phases
    for (auto& resonator : resonators) {
        resonator.phase = 0.0f;
    }

    // Reset envelope followers
    envelopeFollower = 0.0f;

    for (auto& stage : distortionStages) {
        stage.envelope = 0.0f;
    }

    // Reset filters
    for (auto& filter : veilFilters) {
        filter.filter.reset();
    }
}

// Parameter setters
void FibonacciSpiralDistort::setDrive(float driveDB)
{
    // Convert dB to linear and use for overall drive scaling
    float linearDrive = juce::Decibels::decibelsToGain(driveDB);
    fibDrive = juce::jlimit(0.1f, 4.0f, linearDrive);
}

void FibonacciSpiralDistort::setTone(float toneFreqHz)
{
    // Use tone frequency to influence veil cutoff
    veilCutoff = juce::jlimit(200.0f, 5000.0f, toneFreqHz);
    updateVeilFilterCutoffs();
}

void FibonacciSpiralDistort::setMix(float wetMix)
{
    // Mix is handled in process() method
    // Could store for more complex mixing logic
}

void FibonacciSpiralDistort::setSpiralDepth(float depth)
{
    spiralDepth = juce::jlimit(0.0f, 1.0f, depth);
    updateResonatorBank();
}

void FibonacciSpiralDistort::setFibDrive(float fibDriveAmount)
{
    fibDrive = juce::jlimit(0.1f, 4.0f, fibDriveAmount);
    updateFibonacciRatios();
}

void FibonacciSpiralDistort::setBloomRate(float bloomRateSeconds)
{
    bloomRate = juce::jlimit(0.001f, 2.0f, bloomRateSeconds);
    updateEnvelopeCoefficients();
}

void FibonacciSpiralDistort::setVeilCutoff(float veilCutoffHz)
{
    veilCutoff = juce::jlimit(200.0f, 5000.0f, veilCutoffHz);
    updateVeilFilterCutoffs();
}

void FibonacciSpiralDistort::setResonance(float resonanceAmount)
{
    resonance = juce::jlimit(0.0f, 0.8f, resonanceAmount);
    updateResonatorBank();
}

void FibonacciSpiralDistort::setFibDepth(int fibNValue)
{
    fibN = juce::jlimit(5, 15, fibNValue);
    updateFibonacciRatios();
}

void FibonacciSpiralDistort::setMidiNote(float midiNoteValue)
{
    midiNote = juce::jlimit(0.0f, 127.0f, midiNoteValue);
    // Update frequency based on MIDI note (A4 = 69 = 440Hz)
    currentFrequency = 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
    updateResonatorBank();
}

void FibonacciSpiralDistort::setMorphAmount(float morphValue)
{
    morphAmount = juce::jlimit(0.0f, 1.0f, morphValue);
}

// Private methods implementation

void FibonacciSpiralDistort::updateFibonacciRatios()
{
    for (int n = 0; n < 16; ++n) {
        fibRatios[n] = fibonacciRatio(n + 5); // Start from n=5 for better ratios
    }
}

void FibonacciSpiralDistort::updateEnvelopeCoefficients()
{
    // Convert bloom rate to coefficient (time constant)
    float tau = 1.0f / (bloomRate * sampleRate);
    envAttackCoeff = std::exp(-1.0f / (0.01f * sampleRate));  // Fast attack
    envReleaseCoeff = std::exp(-1.0f / (bloomRate * sampleRate)); // Variable release

    // Update distortion stage coefficients
    for (auto& stage : distortionStages) {
        stage.attackCoeff = std::exp(-1.0f / (0.001f * sampleRate)); // Fast attack per stage
        stage.releaseCoeff = std::exp(-1.0f / (bloomRate * 0.25f * sampleRate)); // Faster per stage
    }
}

void FibonacciSpiralDistort::updateResonatorBank()
{
    // Update resonator frequencies based on current frequency and golden ratio
    for (int n = 0; n < MAX_RESONATORS; ++n) {
        resonators[n].frequency = currentFrequency * 3.0f * std::pow(phi, (float)n);
        resonators[n].amplitude = spiralDepth * 0.2f;
        resonators[n].feedback = resonance;
    }
}

void FibonacciSpiralDistort::updateVeilFilterCutoffs()
{
    for (int m = 0; m < VEIL_FILTERS; ++m) {
        veilFilters[m].cutoff = veilCutoff * std::pow(phi, (float)m);
        veilFilters[m].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, veilFilters[m].cutoff);
        veilFilters[m].filter.coefficients = veilFilters[m].coefficients;
    }
}

void FibonacciSpiralDistort::processResonatorBank(const float* input, float* output, int numSamples)
{
    // Generate resonator signals and mix with input
    for (int sample = 0; sample < numSamples; ++sample) {
        float resonatorSum = 0.0f;

        for (int n = 0; n < MAX_RESONATORS; ++n) {
            auto& res = resonators[n];

            // Generate sine wave
            float resonatorSignal = res.amplitude * std::sin(res.phase * juce::MathConstants<float>::twoPi);

            // Add some feedback from previous output
            if (sample > 0) {
                resonatorSignal += res.feedback * output[sample - 1] * 0.1f;
            }

            resonatorSum += resonatorSignal;

            // Update phase
            float phaseInc = res.frequency / sampleRate;
            res.phase += phaseInc;
            res.phase = std::fmod(res.phase, 1.0f);
        }

        // Mix resonator bank with input
        output[sample] = input[sample] + spiralDepth * resonatorSum * 0.3f;
    }
}

void FibonacciSpiralDistort::processFibonacciDistortion(float* buffer, int numSamples)
{
    for (int sample = 0; sample < numSamples; ++sample) {
        float inputSample = buffer[sample];

        // Update envelope follower
        float envInput = std::abs(inputSample);
        envelopeFollower = envInput + envAttackCoeff * (envelopeFollower - envInput);

        // Process through Fibonacci distortion cascade
        float processedSample = inputSample;

        for (int k = 0; k < DISTORTION_STAGES; ++k) {
            auto& stage = distortionStages[k];

            // Update stage envelope
            float stageEnvInput = std::abs(processedSample);
            stage.envelope = stageEnvInput + stage.attackCoeff * (stage.envelope - stageEnvInput);

            // Apply Fibonacci-ratio drive
            float drive = fibDrive * fibRatios[k % 16];
            float distorted = std::tanh(drive * stage.envelope * processedSample);

            processedSample = distorted;
        }

        buffer[sample] = processedSample;
    }
}

void FibonacciSpiralDistort::processSpiralVeilFilter(float* buffer, int numSamples)
{
    // Apply cascaded veil filters with key-scaled cutoff
    for (auto& filter : veilFilters) {
        // Update cutoff based on current frequency for key-scaled veil
        float keyScaledCutoff = filter.cutoff * std::pow(currentFrequency / 100.0f, 0.5f);
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, keyScaledCutoff);
        filter.filter.coefficients = filter.coefficients;

        juce::dsp::AudioBlock<float> block(buffer, 1, 0, numSamples);
        juce::dsp::ProcessContextReplacing<float> context(block);
        filter.filter.process(context);
    }
}

float FibonacciSpiralDistort::processEnvelopeFollower(float input)
{
    float absInput = std::abs(input);

    // Attack phase
    if (absInput > envelopeFollower) {
        envelopeFollower = absInput + envAttackCoeff * (envelopeFollower - absInput);
    }
    // Release phase
    else {
        envelopeFollower = absInput + envReleaseCoeff * (envelopeFollower - absInput);
    }

    return envelopeFollower;
}

float FibonacciSpiralDistort::fibonacciRatio(int n) const
{
    // Fibonacci sequence for ratio calculation
    static const std::array<int, 16> fibSequence = {
        5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765
    };

    if (n < 1) return 1.0f;
    if (n >= fibSequence.size()) return phi; // Converge to golden ratio

    return static_cast<float>(fibSequence[n]) / static_cast<float>(fibSequence[n - 1]);
}
