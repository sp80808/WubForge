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

DistortionForge::~DistortionForge() = default;

//==============================================================================
void DistortionForge::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Prepare basic DSP components (using JUCE-based algorithms)

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
    this->driveDB = std::max(-20.0f, std::min(40.0f, driveDB));
    updateGainStaging();
}

void DistortionForge::setTone (float toneFreqHz)
{
    this->toneFreqHz = std::max(200.0f, std::min(8000.0f, toneFreqHz));
    updateFilters();
}

void DistortionForge::setMix (float wetMix)
{
    this->wetMix = std::max(0.0f, std::min(1.0f, wetMix));
    dryWetMixer.setWetMixProportion(wetMix);
}

void DistortionForge::setBias (float biasAmount)
{
    this->biasAmount = std::max(-1.0f, std::min(1.0f, biasAmount));
}

void DistortionForge::setBitDepth (float bitDepth)
{
    this->bitDepth = std::max(1.0f, std::min(16.0f, bitDepth));
}

void DistortionForge::setSampleRateReduction (float reduction)
{
    this->sampleRateReduction = std::max(0.01f, std::min(1.0f, reduction));
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

            // Apply simple hard clipping (-1 to 1)
            channelData[i] = std::max(-1.0f, std::min(1.0f, input * drive));
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

            // Apply polynomial soft clipping (arctangent approximation)
            float x = input * drive;
            channelData[i] = x / (1.0f + std::abs(x));  // Simple soft clip
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

            // Apply simple wavefolding using sine function
            float x = input * drive;
            // Fold the signal into sine waves at multiples of pi
            channelData[i] = std::sin(x);
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
