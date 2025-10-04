#include "DistortionForge.h"

//==============================================================================
DistortionForge::DistortionForge()
{
    // Initialize DSP components
    inputGain.setRampDurationSeconds(0.05);   // Smooth parameter changes
    outputGain.setRampDurationSeconds(0.05);
    // Note: DryWetMixer doesn't have setRampDurationSeconds in this JUCE version

    // Set initial gain staging
    updateGainStaging();
}

//==============================================================================
void DistortionForge::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Prepare all DSP components
    hardClipper.prepare(spec.numChannels);
    tanhClipper.prepare(spec.numChannels);
    softClipper.prepare(spec.numChannels);
    wavefolder.prepare(spec.numChannels);

    // Prepare tone filter
    toneFilter.prepare(spec);

    // Prepare gain staging
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    dryWetMixer.prepare(spec);

    // Initialize bit crush buffer
    bitCrushBuffer.resize(spec.maximumBlockSize);

    // Reset to ensure clean state
    reset();

    // Update filters and gain staging with current parameters
    updateFilters();
    updateGainStaging();
}

void DistortionForge::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    // Apply input gain for drive control
    auto inputContext = context;
    inputGain.process(inputContext);

    // Route to appropriate distortion algorithm
    switch (currentAlgorithm)
    {
        case Algorithm::Tanh:
            processTanh(inputContext);
            break;
        case Algorithm::HardClip:
            processHardClip(inputContext);
            break;
        case Algorithm::SoftClip:
            processSoftClip(inputContext);
            break;
        case Algorithm::Wavefold:
            processWavefold(inputContext);
            break;
        case Algorithm::BitCrush:
            processBitCrush(inputContext);
            break;
    }

    // Apply tone filtering
    toneFilter.process(inputContext);

    // Apply output gain compensation
    outputGain.process(inputContext);

    // Apply dry/wet mixing
    dryWetMixer.pushDrySamples(context.getInputBlock());
    dryWetMixer.mixWetSamples(inputContext.getOutputBlock());
}

void DistortionForge::reset()
{
    hardClipper.reset();
    tanhClipper.reset();
    softClipper.reset();
    wavefolder.reset();
    toneFilter.reset();
    inputGain.reset();
    outputGain.reset();
    dryWetMixer.reset();

    bitCrushCounter = 0;
    bitCrushPhase = 0.0f;
}

//==============================================================================
// Parameter Setters
void DistortionForge::setAlgorithm (Algorithm algorithm)
{
    currentAlgorithm = algorithm;
}

void DistortionForge::setDrive (float driveDB)
{
    this->driveDB = juce::jlimit(-20.0f, 40.0f, driveDB);
    updateGainStaging();
}

void DistortionForge::setTone (float toneFreqHz)
{
    this->toneFreqHz = juce::jlimit(200.0f, 8000.0f, toneFreqHz);
    updateFilters();
}

void DistortionForge::setMix (float wetMix)
{
    this->wetMix = juce::jlimit(0.0f, 1.0f, wetMix);
    dryWetMixer.setWetMixProportion(wetMix);
}

void DistortionForge::setBias (float biasAmount)
{
    this->biasAmount = juce::jlimit(-1.0f, 1.0f, biasAmount);
}

void DistortionForge::setBitDepth (float bitDepth)
{
    this->bitDepth = juce::jlimit(1.0f, 16.0f, bitDepth);
}

void DistortionForge::setSampleRateReduction (float reduction)
{
    this->sampleRateReduction = juce::jlimit(0.01f, 1.0f, reduction);
}

//==============================================================================
// Distortion Algorithm Implementations

void DistortionForge::processTanh (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto block = context.getOutputBlock();

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* channelData = block.getChannelPointer(ch);

        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            // Apply DC bias for asymmetric distortion
            float input = channelData[i] + biasAmount;

            // Apply tanh distortion with drive scaling
            float drive = std::pow(10.0f, driveDB / 20.0f);
            channelData[i] = std::tanh(input * drive);
        }
    }
}

void DistortionForge::processHardClip (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto block = context.getOutputBlock();

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* channelData = block.getChannelPointer(ch);

        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            // Apply DC bias and drive scaling
            float input = channelData[i] + biasAmount;
            float drive = std::pow(10.0f, driveDB / 20.0f);

            // Apply hard clipping with ADAA
            channelData[i] = hardClipper.processSample(input * drive, ch);
        }
    }
}

void DistortionForge::processSoftClip (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto block = context.getOutputBlock();

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* channelData = block.getChannelPointer(ch);

        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            // Apply DC bias and drive scaling
            float input = channelData[i] + biasAmount;
            float drive = std::pow(10.0f, driveDB / 20.0f);

            // Apply soft clipping with ADAA
            channelData[i] = softClipper.processSample(input * drive, ch);
        }
    }
}

void DistortionForge::processWavefold (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto block = context.getOutputBlock();

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* channelData = block.getChannelPointer(ch);

        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            // Apply DC bias and drive scaling
            float input = channelData[i] + biasAmount;
            float drive = std::pow(10.0f, driveDB / 20.0f);

            // Apply wavefolding
            channelData[i] = wavefolder.processSample(input * drive, ch);
        }
    }
}

void DistortionForge::processBitCrush (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto block = context.getOutputBlock();
    auto numSamples = block.getNumSamples();

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* channelData = block.getChannelPointer(ch);

        for (size_t i = 0; i < numSamples; ++i)
        {
            // Sample rate reduction
            float reductionFactor = sampleRateReduction;
            if (reductionFactor < 1.0f)
            {
                bitCrushPhase += reductionFactor;

                if (bitCrushPhase >= 1.0f)
                {
                    bitCrushPhase -= 1.0f;
                    bitCrushBuffer[ch] = channelData[i];
                }

                channelData[i] = bitCrushBuffer[ch];
            }

            // Bit depth reduction
            if (bitDepth < 16.0f)
            {
                float steps = std::pow(2.0f, bitDepth) - 1.0f;
                float scale = 1.0f / steps;

                // Quantize to bit depth
                channelData[i] = std::round(channelData[i] / scale) * scale;
            }
        }
    }
}

//==============================================================================
// Internal Helper Functions

void DistortionForge::updateFilters()
{
    // Update tone filter coefficients
    auto toneCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, toneFreqHz);
    *toneFilter.state = *toneCoeffs;
}

void DistortionForge::updateGainStaging()
{
    // Calculate input gain from drive parameter
    float inputGainLinear = std::pow(10.0f, driveDB / 20.0f);
    inputGain.setGainLinear(inputGainLinear);

    // Calculate output gain compensation based on distortion algorithm
    float outputGainDB = 0.0f;

    switch (currentAlgorithm)
    {
        case Algorithm::Tanh:
            // Tanh naturally reduces output level, compensate accordingly
            outputGainDB = -driveDB * 0.3f;
            break;
        case Algorithm::HardClip:
            // Hard clipping can maintain or increase level
            outputGainDB = -driveDB * 0.1f;
            break;
        case Algorithm::SoftClip:
            // Soft clipping reduces output level
            outputGainDB = -driveDB * 0.2f;
            break;
        case Algorithm::Wavefold:
            // Wavefolding can significantly increase level
            outputGainDB = -driveDB * 0.4f;
            break;
        case Algorithm::BitCrush:
            // Bit crushing generally maintains level
            outputGainDB = 0.0f;
            break;
    }

    // Apply output gain compensation
    float outputGainLinear = std::pow(10.0f, outputGainDB / 20.0f);
    outputGain.setGainLinear(outputGainLinear);
}