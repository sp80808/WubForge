#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ModuleWrappers.h"
#include "FractalFilter.h"
#include "FMDistortModule.h"
#include "RatDistortionModule.h"

//==============================================================================
WobbleForgeAudioProcessor::WobbleForgeAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       valueTreeState (*this, nullptr, "Parameters", createParameterLayout()),
       presets (std::make_unique<Presets>())
{
    // Initialize module slots - Spectral Morphing as core
    moduleSlots[0] = std::make_unique<FractalFilterModule>(); // Using the new Fractal Filter
    moduleSlots[1] = std::make_unique<FormantTrackerModule>();
    moduleSlots[2] = std::make_unique<RatDistortionModule>();
    moduleSlots[3] = nullptr; // Fourth slot is empty for now

    // Provide global components to modules that need them
    for (auto& slot : moduleSlots)
    {
        if (slot != nullptr)
            slot->setKeyTracker (&keyTracker);
    }
    
    keyTracker.prepareToPlay (44100.0, 512);
}

WobbleForgeAudioProcessor::~WobbleForgeAudioProcessor()
{
}

//==============================================================================
void WobbleForgeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2 };

    // Prepare all modules in their slots
    for (auto& slot : moduleSlots)
    {
        if (slot != nullptr)
            slot->prepare (spec);
    }
    
    keyTracker.prepareToPlay (sampleRate, samplesPerBlock);

    // Prepare feedback components
    feedbackBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
    feedbackBuffer.clear();
    feedbackDampingFilter.prepare(spec);
    feedbackDampingFilter.reset();

    // Prepare output processing
    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 20.0f);
    highPassFilter.prepare (spec);
    highPassFilter.reset();
    *highPassFilter.state = *hpCoeffs;

    outputGain.prepare (spec);
    outputGain.setRampDurationSeconds (0.1);
    outputGain.setGainLinear (1.0f);

    dryWetMixer.prepare (spec);
    dryWetMixer.setWetMixProportion (1.0f);

    updateDSPParameters();
}

void WobbleForgeAudioProcessor::releaseResources()
{
    // Plugin will reset here
}

void WobbleForgeAudioProcessor::reset()
{
    for (auto& slot : moduleSlots)
    {
        if (slot != nullptr)
            slot->reset();
    }
    
    keyTracker.reset();
    feedbackBuffer.clear();
    feedbackDampingFilter.reset();
    highPassFilter.reset();
    outputGain.reset();
    dryWetMixer.reset();
}

//==============================================================================
bool WobbleForgeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

//==============================================================================
void WobbleForgeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    keyTracker.processMidi (midiMessages, buffer.getNumSamples());
    updateDSPParameters();

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    // ### NEW MODULAR ROUTING LOGIC ###
    switch (currentRouting)
    {
        case Routing::Serial:
            for (auto& slot : moduleSlots)
            {
                if (slot != nullptr)
                    slot->process (context);
            }
            break;
        
        case Routing::Parallel:
        {
            juce::AudioBuffer<float> branchB_buffer;
            branchB_buffer.makeCopyOf(buffer);
            juce::dsp::AudioBlock<float> branchA_block(buffer);
            juce::dsp::AudioBlock<float> branchB_block(branchB_buffer);
            juce::dsp::ProcessContextReplacing<float> contextA(branchA_block);
            juce::dsp::ProcessContextReplacing<float> contextB(branchB_block);

            if (moduleSlots[0] != nullptr) moduleSlots[0]->process(contextA);
            if (moduleSlots[1] != nullptr) moduleSlots[1]->process(contextA);
            if (moduleSlots[2] != nullptr) moduleSlots[2]->process(contextB);
            if (moduleSlots[3] != nullptr) moduleSlots[3]->process(contextB);

            branchA_block.multiply(0.5f);
            branchB_block.multiply(0.5f);
            branchA_block.add(branchB_block);
            break;
        }

        case Routing::Feedback:
        {
            auto feedbackGain = valueTreeState.getRawParameterValue("feedbackAmount")->load();

            if (feedbackGain > 0.001f)
            {
                juce::dsp::AudioBlock<float> feedbackBlock (feedbackBuffer);
                juce::dsp::ProcessContextReplacing<float> feedbackContext (feedbackBlock);
                feedbackDampingFilter.process(feedbackContext);
                block.add(feedbackBlock, feedbackGain);
            }

            for (auto& slot : moduleSlots)
            {
                if (slot != nullptr)
                    slot->process(context);
            }

            feedbackBuffer.makeCopyOf(buffer);
            break;
        }

        case Routing::MidSide:
            // To be implemented
            break;
    }

    // Apply final output processing
    highPassFilter.process (context);
    outputGain.process (context);
}

//==============================================================================
bool WobbleForgeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WobbleForgeAudioProcessor::createEditor()
{
    return new WobbleForgeAudioProcessorEditor (*this);
}

//==============================================================================
const juce::String WobbleForgeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WobbleForgeAudioProcessor::acceptsMidi() const { return true; }
bool WobbleForgeAudioProcessor::producesMidi() const { return false; }
bool WobbleForgeAudioProcessor::isMidiEffect() const { return false; }
double WobbleForgeAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================
int WobbleForgeAudioProcessor::getNumPrograms() { return presets->getNumPresets(); }
int WobbleForgeAudioProcessor::getCurrentProgram() { return presets->getCurrentPresetIndex(); }
void WobbleForgeAudioProcessor::setCurrentProgram (int index) { presets->loadPreset (index, valueTreeState); }
const juce::String WobbleForgeAudioProcessor::getProgramName (int index) { return presets->getPresetName (index); }
void WobbleForgeAudioProcessor::changeProgramName (int index, const juce::String& newName) { presets->setPresetName (index, newName); }

//==============================================================================
void WobbleForgeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void WobbleForgeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (valueTreeState.state.getType()))
            valueTreeState.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout WobbleForgeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // --- Fractal Filter Parameters ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("fractalType", "Fractal Type", juce::StringArray { "LP", "HP", "BP" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fractalFreq", "Fractal Freq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fractalQ", "Fractal Q", juce::NormalisableRange<float>(0.1f, 18.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterInt> ("fractalDepth", "Fractal Depth", 1, 8, 4));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fractalRatio", "Fractal Ratio", juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 0.5f));

    // --- Original Parameters (for other slots) ---
    // Comb Stack parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("combCount", "Comb Count", juce::NormalisableRange<float>(4.0f, 8.0f, 1.0f), 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("combDelay", "Comb Delay", juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("combFeedback", "Comb Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoRate", "LFO Rate", juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoDepth", "LFO Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // Distortion Forge parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("wavefoldAmount", "Wavefold Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("clipAmount", "Clip Amount", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("bitCrushAmount", "Bit Crush", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantFreq", "Formant Freq", juce::NormalisableRange<float>(100.0f, 2000.0f, 1.0f), 400.0f));

    // Key tracking
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("keyTrackAmount", "Key Track Amount", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));

    // Formant tracking parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantKeyTrack", "Formant Key Track", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantGain", "Formant Gain", juce::NormalisableRange<float>(-6.0f, 18.0f, 0.1f), 8.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantQ", "Formant Q", juce::NormalisableRange<float>(1.0f, 15.0f, 0.1f), 8.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantBaseFreq", "Formant Base Freq", juce::NormalisableRange<float>(50.0f, 200.0f, 1.0f), 100.0f));

    // Output parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("hpfCutoff", "HPF Cutoff", juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f), 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("outputGain", "Output Gain", juce::NormalisableRange<float>(-20.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("dryWet", "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    
    // Routing parameter
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("routing", "Routing", juce::StringArray { "Serial", "Parallel", "Mid-Side", "Feedback" }, 0));

    // Feedback parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("feedbackAmount", "Feedback Amount", juce::NormalisableRange<float>(0.0f, 0.99f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("feedbackDamping", "Feedback Damping", juce::NormalisableRange<float>(500.0f, 15000.0f, 1.0f, 0.4f), 5000.0f));

    // FM Distort parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fmRatio", "FM Ratio", juce::NormalisableRange<float>(0.0f, 4.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fmIndex", "FM Index", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 1.0f));

    // Rat Distortion parameters
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ratDrive", "Rat Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ratTone", "Rat Tone", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ratLevel", "Rat Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));

    return { params.begin(), params.end() };
}

void WobbleForgeAudioProcessor::updateDSPParameters()
{
    auto& params = valueTreeState;
    
    currentRouting = static_cast<Routing>(params.getRawParameterValue("routing")->load());

    auto dampingFreq = params.getRawParameterValue("feedbackDamping")->load();
    *feedbackDampingFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, dampingFreq);

    keyTracker.setKeyTrackAmount (params.getRawParameterValue ("keyTrackAmount")->load());

    // Pass key tracking data to DSP components
    float currentKeyTrackFreq = keyTracker.getCurrentFrequency();
    float currentKeyTrackAmount = keyTracker.getKeyTrackAmount();

    // Update modules based on their type and position in the chain
    if (auto* fractalModule = dynamic_cast<FractalFilterModule*>(moduleSlots[0].get()))
    {
        fractalModule->setType (*params.getRawParameterValue("fractalType"));
        fractalModule->setBaseFrequency (*params.getRawParameterValue("fractalFreq"));
        fractalModule->setQ (*params.getRawParameterValue("fractalQ"));
        fractalModule->setDepth (*params.getRawParameterValue("fractalDepth"));
        fractalModule->setRatio (*params.getRawParameterValue("fractalRatio"));
    }

    if (auto* formantModule = dynamic_cast<FormantTrackerModule*>(moduleSlots[1].get()))
    {
        auto& proc = formantModule->getInternalProcessor();
        proc.setKeyTrackAmount (params.getRawParameterValue ("formantKeyTrack")->load());
        proc.setFormantGain (params.getRawParameterValue ("formantGain")->load());
        proc.setFormantQ (params.getRawParameterValue ("formantQ")->load());
        proc.setBaseFrequency (params.getRawParameterValue ("formantBaseFreq")->load());
    }

    if (auto* distortionModule = dynamic_cast<DistortionForgeModule*>(moduleSlots[2].get()))
    {
        auto& proc = distortionModule->getInternalProcessor();
        proc.setWavefoldAmount (params.getRawParameterValue ("wavefoldAmount")->load());
        proc.setClipAmount (params.getRawParameterValue ("clipAmount")->load());
        proc.setBitCrushAmount (params.getRawParameterValue ("bitCrushAmount")->load());
        proc.setFormantFreq (params.getRawParameterValue ("formantFreq")->load());
    }

    if (auto* fmModule = dynamic_cast<FMDistortModule*>(moduleSlots[2].get()))
    {
        fmModule->setRatio(params.getRawParameterValue("fmRatio")->load());
        fmModule->setModIndex(params.getRawParameterValue("fmIndex")->load());
    }

    if (auto* ratModule = dynamic_cast<RatDistortionModule*>(moduleSlots[2].get()))
    {
        ratModule->setDrive(params.getRawParameterValue("ratDrive")->load());
        ratModule->setTone(params.getRawParameterValue("ratTone")->load());
        ratModule->setLevel(params.getRawParameterValue("ratLevel")->load());
    }

    // Update output processing
    auto hpfCutoff = params.getRawParameterValue ("hpfCutoff")->load();
    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hpfCutoff);
    *highPassFilter.state = *hpCoeffs;

    auto gainDB = params.getRawParameterValue ("outputGain")->load();
    outputGain.setGainDecibels (gainDB);

    dryWetMixer.setWetMixProportion (params.getRawParameterValue ("dryWet")->load());
}

//==============================================================================
AudioModule* WobbleForgeAudioProcessor::getModuleInSlot (int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < numModuleSlots)
        return moduleSlots[slotIndex].get();
    return nullptr;
}

//==============================================================================
bool WobbleForgeAudioProcessor::getCurrentSpectrumData(float* magnitudeBuffer, int maxSize) const
{
    if (magnitudeBuffer == nullptr || maxSize <= 0)
        return false;

    // Look for a SpectralMorphingModule in any slot
    for (const auto& module : moduleSlots)
    {
        if (module != nullptr)
        {
            if (auto* spectralModule = dynamic_cast<const SpectralMorphingModule*>(module.get()))
            {
                // Get spectrum data from the module
                // For now, we'll copy from the module's magnitude buffer
                // This assumes the module has a method to get current spectral data
                const auto& moduleMagnitude = spectralModule->getCurrentMagnitudeBuffer();

                int copySize = std::min(maxSize, static_cast<int>(moduleMagnitude.size()));
                if (copySize > 0)
                {
                    std::copy_n(moduleMagnitude.begin(), copySize, magnitudeBuffer);

                    // Clear remaining buffer if any
                    std::fill_n(magnitudeBuffer + copySize, maxSize - copySize, 0.0f);
                    return true;
                }
                break; // Use first available spectral module
            }
        }
    }

    return false;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WobbleForgeAudioProcessor();
}
