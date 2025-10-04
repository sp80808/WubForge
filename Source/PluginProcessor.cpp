#include "PluginProcessor.h"
#include "UniversalDistortionModule.h"
#include "UniversalFilterModule.h"
#include "ChowEQModule.h"
#include "DistortionForge.h"
#include "MDASubSynthModuleDirect.h"
#include <juce_dsp/juce_dsp.h>

juce::StringArray WubForgeAudioProcessor::getAvailableModules()
{
    return {
        "Universal Filter",
        "Universal Distortion",
        "Chow EQ",
        "MDA SubSynth"
    };
}

std::unique_ptr<AudioModule> WubForgeAudioProcessor::createModuleFromName(const juce::String& name)
{
    if (name == "Universal Filter") return std::make_unique<UniversalFilterModule>();
    if (name == "Universal Distortion") return std::make_unique<UniversalDistortionModule>();
    if (name == "Chow EQ") return std::make_unique<ChowEQModule>();
    if (name == "MDA SubSynth") return std::make_unique<MDASubSynthModuleDirect>();

    return nullptr;
}

//==============================================================================
WubForgeAudioProcessor::WubForgeAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       valueTreeState (*this, nullptr, juce::Identifier("WubForge"), createParameterLayout())
{
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
    return new WubForgeAudioProcessorEditor (*this);
}
