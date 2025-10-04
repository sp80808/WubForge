#include "PluginProcessor.h"
#include "UniversalFilterModule.h"
#include "UniversalDistortionModule.h"
#include "ChowEQModule.h"

// Foleys configuration
// FOLEYS_SET_SOURCE_PATH(__FILE__); // Temporarily commented out due to build issues

//==============================================================================
WubForgeAudioProcessor::WubForgeAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       valueTreeState (*this, nullptr, "Parameters", createParameterLayout()),
       presets (std::make_unique<Presets>())
{
    // Initialize module slots
    moduleSlots[0] = std::make_unique<FractalFilterModule>();
    moduleSlots[1] = std::make_unique<PluckModule>();
    moduleSlots[2] = std::make_unique<UniversalDistortionModule>();
    moduleSlots[3] = std::make_unique<MDASubSynthModuleDirect>(); // Classic MDA SubSynth bass enhancement
    moduleSlots[4] = std::make_unique<DistortionForge>(); // Professional distortion using chowdsp_utils

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

    // Trigger pluck modules on note on
    for (const auto metadata : midiMessages)
    {
        if (metadata.getMessage().isNoteOn())
        {
            for (auto& slot : moduleSlots)
            {
                if (auto* pluckModule = dynamic_cast<PluckModule*>(slot.get()))
                {
                    pluckModule->pluck();
                }
            }
        }
    }

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
            if (moduleSlots[4] != nullptr) moduleSlots[4]->process(contextB);

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
        {
            if (buffer.getNumChannels() != 2)
            {
                // Fallback to serial processing if not a stereo signal
                for (auto& slot : moduleSlots)
                    if (slot != nullptr) slot->process(context);
                break;
            }

            juce::AudioBuffer<float> midBuffer(1, buffer.getNumSamples());
            juce::AudioBuffer<float> sideBuffer(1, buffer.getNumSamples());
            buffer.getMidSide (midBuffer.getWritePointer(0), sideBuffer.getWritePointer(0), buffer.getNumSamples());

            // Process Mid channel through slots 0 & 1
            juce::dsp::AudioBlock<float> midBlock(midBuffer);
            juce::dsp::ProcessContextReplacing<float> midContext(midBlock);
            if (moduleSlots[0] != nullptr) moduleSlots[0]->process(midContext);
            if (moduleSlots[1] != nullptr) moduleSlots[1]->process(midContext);

            // Process Side channel through slots 2, 3 & 4
            juce::dsp::AudioBlock<float> sideBlock(sideBuffer);
            juce::dsp::ProcessContextReplacing<float> sideContext(sideBlock);
            if (moduleSlots[2] != nullptr) moduleSlots[2]->process(sideContext);
            if (moduleSlots[3] != nullptr) moduleSlots[3]->process(sideContext);
            if (moduleSlots[4] != nullptr) moduleSlots[4]->process(sideContext);

            // Recombine into original buffer
            juce::AudioBuffer<float>::setFromMidSide (&buffer, midBuffer.getReadPointer(0), sideBuffer.getReadPointer(0), buffer.getNumSamples());
            break;
        }
    }

    // Apply final output processing
    highPassFilter.process (context);
    outputGain.process (context);
}


const juce::String WubForgeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WobbleForgeAudioProcessor::acceptsMidi() const { return true; }
bool WobbleForgeAudioProcessor::producesMidi() const { return false; }
bool WobbleForgeAudioProcessor::isMidiEffect() const { return false; }
double WobbleForgeAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================
int WubForgeAudioProcessor::getNumPrograms() { return presets->getNumPresets(); }
int WubForgeAudioProcessor::getCurrentProgram() { return presets->getCurrentPresetIndex(); }
void WubForgeAudioProcessor::setCurrentProgram (int index) { presets->loadPreset (index, valueTreeState); }
const juce::String WubForgeAudioProcessor::getProgramName (int index) { return presets->getPresetName (index); }
void WubForgeAudioProcessor::changeProgramName (int index, const juce::String& newName) { presets->setPresetName (index, newName); }

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

juce::AudioProcessorEditor* WobbleForgeAudioProcessor::createEditor()
{
    return new WubForgeAudioProcessorEditor (*this);
}

//==============================================================================


//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout WobbleForgeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // --- Global Parameters ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("routing", "Routing", juce::StringArray { "Serial", "Parallel", "Feedback" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("feedbackAmount", "Feedback Amount", juce::NormalisableRange<float>(0.0f, 0.99f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("feedbackDamping", "Feedback Damping", juce::NormalisableRange<float>(500.0f, 15000.0f, 1.0f, 0.4f), 5000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("keyTrackAmount", "Key Track Amount", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));

    // --- Slot 1: Fractal Filter ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("fractalType", "Fractal Type", juce::StringArray { "LP", "HP", "BP" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fractalFreq", "Fractal Freq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fractalQ", "Fractal Q", juce::NormalisableRange<float>(0.1f, 18.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterInt> ("fractalDepth", "Fractal Depth", 1, 8, 4));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fractalRatio", "Fractal Ratio", juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 0.5f));

    // --- Slot 2: Formant Tracker ---
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantKeyTrack", "Formant Key Track", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantGain", "Formant Gain", juce::NormalisableRange<float>(-6.0f, 18.0f, 0.1f), 8.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantQ", "Formant Q", juce::NormalisableRange<float>(1.0f, 15.0f, 0.1f), 8.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("formantBaseFreq", "Formant Base Freq", juce::NormalisableRange<float>(50.0f, 200.0f, 1.0f), 100.0f));

    // --- Slot 2: Spectral Filter ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("spectralMode", "Spectral Mode", juce::StringArray { "Notch", "Comb" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("spectralFreq", "Spectral Freq", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("spectralBw", "Spectral BW", juce::NormalisableRange<float>(10.0f, 5000.0f, 1.0f, 0.4f), 100.0f));

    // --- Slot 2: Pluck Module ---
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("pluckDecay", "Pluck Decay", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("pluckDamping", "Pluck Damping", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // --- Slot 3: Universal Distortion ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("distModel", "Distortion Model", juce::StringArray { "Digital", "FM", "Rodent", "Screamer" }, 0));
    // Digital
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distDigitalWavefold", "Wavefold", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distDigitalBitcrush", "Bitcrush", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    // FM
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distFmRatio", "FM Ratio", juce::NormalisableRange<float>(0.0f, 4.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distFmIndex", "FM Index", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 0.0f));
    // Rodent
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distRodentDrive", "Rodent Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distRodentTone", "Rodent Tone", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distRodentLevel", "Rodent Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));
    // Screamer
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distScreamerDrive", "Screamer Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distScreamerTone", "Screamer Tone", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("distScreamerLevel", "Screamer Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));

    // --- Slot 4: DistortionForge ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("forgeAlgorithm", "Forge Algorithm", juce::StringArray { "Tanh", "HardClip", "SoftClip", "Wavefold", "BitCrush" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("forgeDrive", "Forge Drive", juce::NormalisableRange<float>(-20.0f, 40.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("forgeTone", "Forge Tone", juce::NormalisableRange<float>(200.0f, 8000.0f, 1.0f, 0.3f), 2000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("forgeMix", "Forge Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("forgeBias", "Forge Bias", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("forgeBitDepth", "Forge Bit Depth", juce::NormalisableRange<float>(1.0f, 16.0f, 0.1f), 16.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("forgeSampleRateReduction", "Forge SR Reduction", juce::NormalisableRange<float>(0.01f, 1.0f, 0.01f), 1.0f));

    // --- Output Parameters ---
    params.push_back (std::make_unique<juce::AudioParameterBool> ("magicForgeTrigger", "Magic Forge", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("hpfCutoff", "HPF Cutoff", juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f), 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("outputGain", "Output Gain", juce::NormalisableRange<float>(-20.0f, 12.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("dryWet", "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

    return { params.begin(), params.end() };
}

void WobbleForgeAudioProcessor::updateDSPParameters()
{
    auto& params = valueTreeState;

    // Check for Magic Forge trigger
    if (auto* trigger = dynamic_cast<juce::AudioParameterBool*>(params.getParameter("magicForgeTrigger")))
    {
        if (trigger->get())
        {
            runMagicForge();
            trigger->setValueNotifyingHost(false);
        }
    }

    currentRouting = static_cast<Routing>(params.getRawParameterValue("routing")->load());

    auto dampingFreq = params.getRawParameterValue("feedbackDamping")->load();
    *feedbackDampingFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, dampingFreq);

    keyTracker.setKeyTrackAmount (params.getRawParameterValue ("keyTrackAmount")->load());

    if (auto* fractalModule = dynamic_cast<FractalFilterModule*>(moduleSlots[0].get()))
    {
        fractalModule->setType (*params.getRawParameterValue("fractalType"));
        fractalModule->setBaseFrequency (*params.getRawParameterValue("fractalFreq"));
        fractalModule->setQ (*params.getRawParameterValue("fractalQ"));
        fractalModule->setDepth (*params.getRawParameterValue("fractalDepth"));
        fractalModule->setRatio (*params.getRawParameterValue("fractalRatio"));
    }

    if (auto* spectralModule = dynamic_cast<SpectralFilterModule*>(moduleSlots[1].get()))
    {
        spectralModule->setMode(static_cast<SpectralFilterModule::Mode>(params.getRawParameterValue("spectralMode")->load()));
        spectralModule->setFrequency(params.getRawParameterValue("spectralFreq")->load());
        spectralModule->setBandwidth(params.getRawParameterValue("spectralBw")->load());
    }

    if (auto* pluckModule = dynamic_cast<PluckModule*>(moduleSlots[1].get()))
    {
        pluckModule->setDecay(params.getRawParameterValue("pluckDecay")->load());
        pluckModule->setDamping(params.getRawParameterValue("pluckDamping")->load());
    }

    if (auto* formantModule = dynamic_cast<FormantTrackerModule*>(moduleSlots[1].get()))
    {
        auto& proc = formantModule->getInternalProcessor();
        proc.setKeyTrackAmount (params.getRawParameterValue ("formantKeyTrack")->load());
        proc.setFormantGain (params.getRawParameterValue ("formantGain")->load());
        proc.setFormantQ (params.getRawParameterValue ("formantQ")->load());
        proc.setBaseFrequency (params.getRawParameterValue ("formantBaseFreq")->load());
    }

    if (auto* distModule = dynamic_cast<UniversalDistortionModule*>(moduleSlots[2].get()))
    {
        distModule->setModel(static_cast<UniversalDistortionModule::Model>(params.getRawParameterValue("distModel")->load()));
        distModule->setDigitalWavefold(params.getRawParameterValue("distDigitalWavefold")->load());
        distModule->setDigitalBitcrush(params.getRawParameterValue("distDigitalBitcrush")->load());
        distModule->setFmRatio(params.getRawParameterValue("distFmRatio")->load());
        distModule->setFmIndex(params.getRawParameterValue("distFmIndex")->load());
        distModule->setRodentDrive(params.getRawParameterValue("distRodentDrive")->load());
        distModule->setRodentTone(params.getRawParameterValue("distRodentTone")->load());
        distModule->setRodentLevel(params.getRawParameterValue("distRodentLevel")->load());
        distModule->setScreamerDrive(params.getRawParameterValue("distScreamerDrive")->load());
        distModule->setScreamerTone(params.getRawParameterValue("distScreamerTone")->load());
        distModule->setScreamerLevel(params.getRawParameterValue("distScreamerLevel")->load());
    }

    if (auto* forgeModule = dynamic_cast<DistortionForge*>(moduleSlots[4].get()))
    {
        forgeModule->setAlgorithm(static_cast<DistortionForge::Algorithm>(params.getRawParameterValue("forgeAlgorithm")->load()));
        forgeModule->setDrive(params.getRawParameterValue("forgeDrive")->load());
        forgeModule->setTone(params.getRawParameterValue("forgeTone")->load());
        forgeModule->setMix(params.getRawParameterValue("forgeMix")->load());
        forgeModule->setBias(params.getRawParameterValue("forgeBias")->load());
        forgeModule->setBitDepth(params.getRawParameterValue("forgeBitDepth")->load());
        forgeModule->setSampleRateReduction(params.getRawParameterValue("forgeSampleRateReduction")->load());
    }

    // Update output processing
    auto hpfCutoff = params.getRawParameterValue ("hpfCutoff")->load();
    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hpfCutoff);
    *highPassFilter.state = *hpCoeffs;

    auto gainDB = params.getRawParameterValue ("outputGain")->load();
    outputGain.setGainDecibels (gainDB);

    dryWetMixer.setWetMixProportion (params.getRawParameterValue ("dryWet")->load());
}

void WobbleForgeAudioProcessor::runMagicForge()
{
    auto& vts = valueTreeState;
    auto& random = juce::Random::getSystemRandom();

    // Randomize routing
    if (auto* p = vts.getParameter("routing"))
        p->setValueNotifyingHost(random.nextFloat()); // Randomly selects between Serial, Parallel, Feedback

    // Randomize feedback, but keep it low
    if (auto* p = vts.getParameter("feedbackAmount"))
        p->setValueNotifyingHost(random.nextFloat() * 0.4f);

    // Randomize fractal filter
    if (auto* p = vts.getParameter("fractalType")) p->setValueNotifyingHost(random.nextInt(3) / 2.0f);
    if (auto* p = vts.getParameter("fractalFreq")) p->setValueNotifyingHost(juce::jmap(random.nextFloat(), 0.3f, 1.0f)); // Skew towards higher freqs
    if (auto* p = vts.getParameter("fractalQ")) p->setValueNotifyingHost(random.nextFloat());
    if (auto* p = vts.getParameter("fractalDepth")) p->setValueNotifyingHost(random.nextInt(5) / 7.0f + (1.0f/7.0f));

    // Randomize spectral filter
    if (auto* p = vts.getParameter("spectralMode")) p->setValueNotifyingHost(random.nextInt(2) / 1.0f);
    if (auto* p = vts.getParameter("spectralFreq")) p->setValueNotifyingHost(random.nextFloat());
    if (auto* p = vts.getParameter("spectralBw")) p->setValueNotifyingHost(random.nextFloat() * 0.4f);

    // Randomize distortion
    if (auto* p = vts.getParameter("distModel")) p->setValueNotifyingHost(random.nextInt(4) / 3.0f);
    if (auto* p = vts.getParameter("distRodentDrive")) p->setValueNotifyingHost(random.nextFloat());
    if (auto* p = vts.getParameter("distScreamerDrive")) p->setValueNotifyingHost(random.nextFloat());
    if (auto* p = vts.getParameter("distFmIndex")) p->setValueNotifyingHost(random.nextFloat() * 0.5f);
    if (auto* p = vts.getParameter("distDigitalWavefold")) p->setValueNotifyingHost(random.nextFloat() * 0.7f);
    if (auto* p = vts.getParameter("distDigitalBitcrush")) p->setValueNotifyingHost(random.nextFloat() * 0.5f);
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
    return new WubForgeAudioProcessor();
}

