#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UniversalDistortionModule.h"
#include "UniversalFilterModule.h"
#include "DistortionForge.h"
#include "MDASubSynthModuleDirect.h"
#include "SampleMorpher.h"
#include "FibonacciSpiralDistort.h"
#include "HarmonicRichFilter.h"
#include "WavetableFilterModule.h"
#include <juce_dsp/juce_dsp.h>

// Note: ChowEQModule requires ChowDSP library which may need separate installation
// #include "ChowEQModule.h"

juce::StringArray WubForgeAudioProcessor::getAvailableModules()
{
    return {
        "Universal Filter",
        "Universal Distortion",
        "Chow EQ",
        "MDA SubSynth",
        "Sample Morpher",
        "Fibonacci Spiral Distort",
        "Harmonic Rich Filter",
        "Wavetable Filter"
    };
}

std::unique_ptr<AudioModule> WubForgeAudioProcessor::createModuleFromName(const juce::String& name)
{
    if (name == "Universal Filter") return std::make_unique<UniversalFilterModule>();
    if (name == "Universal Distortion") return std::make_unique<UniversalDistortionModule>();
    // if (name == "Chow EQ") return std::make_unique<ChowEQModule>(); // Requires ChowDSP library
    if (name == "MDA SubSynth") return std::make_unique<MDASubSynthModuleDirect>();
    if (name == "Sample Morpher") return std::make_unique<SampleMorpher>();
    if (name == "Fibonacci Spiral Distort") return std::make_unique<FibonacciSpiralDistort>();
    if (name == "Harmonic Rich Filter") return std::make_unique<HarmonicRichFilter>();
    if (name == "Wavetable Filter") return std::make_unique<WavetableFilterModule>();

    return nullptr;
}

//==============================================================================
WubForgeAudioProcessor::WubForgeAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       valueTreeState (*this, nullptr, juce::Identifier("WubForge"), createParameterLayout())
{
   // Add parameter change listeners for Harmonic Rich Filter
   valueTreeState.addParameterListener("hrFilterShape", this);
   valueTreeState.addParameterListener("hrBloomDepth", this);
   valueTreeState.addParameterListener("hrLfoRate", this);
   valueTreeState.addParameterListener("hrVeilMix", this);
   valueTreeState.addParameterListener("hrAttackTime", this);
   valueTreeState.addParameterListener("hrReleaseTime", this);
   valueTreeState.addParameterListener("hrRichnessThreshold", this);

   // Initialize module slots using the factory
   moduleSlots[0] = createModuleFromName("Universal Filter");
   moduleSlots[1] = createModuleFromName("Universal Distortion");
   moduleSlots[2] = nullptr;
   moduleSlots[3] = nullptr;
   moduleSlots[4] = nullptr;

   // Provide global components to modules that need them
   for (auto& slot : moduleSlots)
   {
       if (slot != nullptr)
           slot->setKeyTracker (&keyTracker);
   }

   keyTracker.prepareToPlay (44100.0, 512);
}

WubForgeAudioProcessor::~WubForgeAudioProcessor()
{
}

//==============================================================================
void WubForgeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };

    // Prepare all modules
    for (auto& slot : moduleSlots)
    {
        if (slot != nullptr)
            slot->prepare (spec);
    }

    keyTracker.prepareToPlay (sampleRate, samplesPerBlock);

    // Prepare feedback
    feedbackBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
    feedbackBuffer.clear();

    // Prepare output DSP
    outputGain.prepare (spec);
    outputGain.setGainLinear (1.0f);
    dryWetMixer.prepare (spec);
}

void WubForgeAudioProcessor::releaseResources()
{
    // Plugin will reset here
}

void WubForgeAudioProcessor::reset()
{
    for (auto& slot : moduleSlots)
    {
        if (slot != nullptr)
            slot->reset();
    }

    keyTracker.reset();
    feedbackBuffer.clear();
    outputGain.reset();
    dryWetMixer.reset();
}

//==============================================================================

//==============================================================================
void WubForgeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels that don't have input
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    keyTracker.processMidi (midiMessages, buffer.getNumSamples());

    // Update parameters before processing
    updateDSPParameters();

    // Simple serial processing for alpha
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    // Process all slots in order
    for (auto& slot : moduleSlots)
    {
        if (slot != nullptr)
            slot->process (context);
    }

    // Apply final output processing
    outputGain.process (context);
}

//==============================================================================

//==============================================================================



//==============================================================================
AudioModule* WubForgeAudioProcessor::getModuleInSlot (int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(numModuleSlots))
        return moduleSlots[slotIndex].get();
    return nullptr;
}

//==============================================================================
bool WubForgeAudioProcessor::getCurrentSpectrumData(float* magnitudeBuffer, int maxSize) const
{
    // Placeholder - modules can implement this if they support spectrum data
    if (magnitudeBuffer && maxSize > 0)
    {
        std::fill_n(magnitudeBuffer, maxSize, 0.0f);
    }
    return false;
}

//==============================================================================
juce::AudioProcessorEditor* WubForgeAudioProcessor::createEditor()
{
    return new WubForgeAudioProcessorEditor (*this); // This line is causing the error
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout WubForgeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Harmonic Rich Filter Parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "hrFilterShape",
        "Harmonic Rich Filter Shape",
        juce::StringArray{ "Helical Sine Veil", "Cascade Harmonic Bloom", "Spectral Sine Helix", "Blend Mode" },
        0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hrBloomDepth",
        "Harmonic Rich Bloom Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hrLfoRate",
        "Harmonic Rich LFO Rate",
        juce::NormalisableRange<float>(0.001f, 20.0f, 0.001f, 0.3f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hrVeilMix",
        "Harmonic Rich Veil Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hrAttackTime",
        "Harmonic Rich Attack Time",
        juce::NormalisableRange<float>(0.1f, 1000.0f, 0.1f, 0.3f),
        10.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hrReleaseTime",
        "Harmonic Rich Release Time",
        juce::NormalisableRange<float>(0.1f, 1000.0f, 0.1f, 0.3f),
        100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hrRichnessThreshold",
        "Harmonic Rich Richness Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -24.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
void WubForgeAudioProcessor::updateDSPParameters()
{
    // Update Harmonic Rich Filter parameters if module exists
    for (auto& slot : moduleSlots)
    {
        if (slot != nullptr && slot->getName() == "Harmonic Rich Filter")
        {
            auto* hrFilter = static_cast<HarmonicRichFilter*>(slot.get());

            // Update filter shape
            auto shapeParam = valueTreeState.getRawParameterValue("hrFilterShape");
            if (shapeParam != nullptr)
            {
                HarmonicRichFilter::FilterShape shape;
                switch (static_cast<int>(*shapeParam))
                {
                    case 0: shape = HarmonicRichFilter::FilterShape::HelicalSineVeil; break;
                    case 1: shape = HarmonicRichFilter::FilterShape::CascadeHarmonicBloom; break;
                    case 2: shape = HarmonicRichFilter::FilterShape::SpectralSineHelix; break;
                    default: shape = HarmonicRichFilter::FilterShape::HelicalSineVeil; break;
                }
                hrFilter->setFilterShape(shape);
            }

            // Update bloom depth
            auto bloomDepthParam = valueTreeState.getRawParameterValue("hrBloomDepth");
            if (bloomDepthParam != nullptr)
            {
                hrFilter->setHelicalVeilDepth(*bloomDepthParam);
            }

            // Update LFO rate (using envelope sensitivity as proxy for now)
            auto lfoRateParam = valueTreeState.getRawParameterValue("hrLfoRate");
            if (lfoRateParam != nullptr)
            {
                hrFilter->setEnvelopeSensitivity(*lfoRateParam * 0.05f); // Scale to 0.0-1.0 range
            }

            // Update veil mix (using mix parameter)
            auto veilMixParam = valueTreeState.getRawParameterValue("hrVeilMix");
            if (veilMixParam != nullptr)
            {
                hrFilter->setMix(*veilMixParam);
            }

            // Update attack time
            auto attackTimeParam = valueTreeState.getRawParameterValue("hrAttackTime");
            if (attackTimeParam != nullptr)
            {
                // Note: HarmonicRichFilter expects attack time in milliseconds
                // We'll need to modify the filter to accept this parameter
            }

            // Update release time
            auto releaseTimeParam = valueTreeState.getRawParameterValue("hrReleaseTime");
            if (releaseTimeParam != nullptr)
            {
                // Note: HarmonicRichFilter expects release time in milliseconds
                // We'll need to modify the filter to accept this parameter
            }

            // Update richness threshold
            auto richnessThresholdParam = valueTreeState.getRawParameterValue("hrRichnessThreshold");
            if (richnessThresholdParam != nullptr)
            {
                // Note: HarmonicRichFilter doesn't currently have this parameter
                // We'll need to add it to the filter interface
            }
        }
    }
}

//==============================================================================
void WubForgeAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Trigger parameter update when any Harmonic Rich Filter parameter changes
    if (parameterID.startsWith("hr"))
    {
        updateDSPParameters();
    }
}

//==============================================================================
// Required JUCE AudioProcessor implementations
//==============================================================================

const juce::String WubForgeAudioProcessor::getName() const
{
    return juce::String ("WubForge");
}

bool WubForgeAudioProcessor::acceptsMidi() const
{
    return true;
}

bool WubForgeAudioProcessor::producesMidi() const
{
    return false;
}

bool WubForgeAudioProcessor::isMidiEffect() const
{
    return false;
}

double WubForgeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WubForgeAudioProcessor::getNumPrograms()
{
    if (presets)
        return presets->getNumPresets();
    return 1;
}

int WubForgeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WubForgeAudioProcessor::setCurrentProgram(int index)
{
    // For now, we don't actually change presets but just track the index
    // presets->setPresetName(index, "Current"); // Placeholder
}

const juce::String WubForgeAudioProcessor::getProgramName(int index)
{
    if (presets && index >= 0 && index < presets->getNumPresets())
        return presets->getPresetName(index);
    return "Default";
}

void WubForgeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    if (presets && index >= 0 && index < presets->getNumPresets())
        presets->setPresetName(index, newName);
}

bool WubForgeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono->stereo, stereo->stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void WubForgeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WubForgeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(valueTreeState.state.getType()))
            valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}
