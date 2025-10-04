#include "UniversalDistortionModule.h"

UniversalDistortionModule::UniversalDistortionModule()
{
    // Constructor
}

void UniversalDistortionModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    rodentInputGain.prepare(spec);
    rodentToneFilter.prepare(spec);
    rodentOutputGain.prepare(spec);

    screamerInputGain.prepare(spec);
    screamerMidBoostFilter.prepare(spec);
    screamerToneFilter.prepare(spec);
    screamerOutputGain.prepare(spec);

    reset();
}

void UniversalDistortionModule::reset()
{
    rodentInputGain.reset();
    rodentToneFilter.reset();
    rodentOutputGain.reset();

    screamerInputGain.reset();
    screamerMidBoostFilter.reset();
    screamerToneFilter.reset();
    screamerOutputGain.reset();

    fmPhase = 0.0f;
    updateFilters();
}

void UniversalDistortionModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    switch (currentModel)
    {
        case Model::Digital:
            processDigital(context);
            break;
        case Model::FM:
            processFM(context);
            break;
        case Model::Rodent:
            processRodent(context);
            break;
        case Model::Screamer:
            processScreamer(context);
            break;
    }
}

//==============================================================================
// --- Parameter Setters ---
void UniversalDistortionModule::setModel (Model newModel)
{
    if (currentModel != newModel)
    {
        currentModel = newModel;
        // Potentially reset state when changing models
        reset();
    }
}

void UniversalDistortionModule::setDigitalWavefold(float amount) { digitalWavefold = amount; }
void UniversalDistortionModule::setDigitalBitcrush(float amount) { digitalBitcrush = amount; }

void UniversalDistortionModule::setFmRatio(float ratio) 
{
    if (fmRatio != ratio) { fmRatio = ratio; }
}
void UniversalDistortionModule::setFmIndex(float index) { fmIndex = index; }

void UniversalDistortionModule::setRodentDrive(float drive)
{
    rodentInputGain.setGainDecibels(drive * 40.0f);
}

void UniversalDistortionModule::setRodentTone(float tone)
{
    if (rodentTone != tone) { rodentTone = tone; updateFilters(); }
}

void UniversalDistortionModule::setRodentLevel(float level)
{
    rodentOutputGain.setGainDecibels(juce::jmap(level, 0.0f, 1.0f, -20.0f, 0.0f));
}

void UniversalDistortionModule::setScreamerDrive(float drive)
{
    screamerInputGain.setGainDecibels(drive * 30.0f);
}

void UniversalDistortionModule::setScreamerTone(float tone)
{
    if (screamerTone != tone) { screamerTone = tone; updateFilters(); }
}

void UniversalDistortionModule::setScreamerLevel(float level)
{
    screamerOutputGain.setGainDecibels(juce::jmap(level, 0.0f, 1.0f, -18.0f, 0.0f));
}

//==============================================================================
// --- Internal Processing Functions ---

void UniversalDistortionModule::processDigital(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& block = context.getOutputBlock();
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* samples = block.getChannelPointer(ch);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            float s = samples[i];
            // Wavefolding
            if (digitalWavefold > 0.0f)
                s = std::sin(s * (1.0f + digitalWavefold * 5.0f));
            // Bitcrushing
            if (digitalBitcrush > 0.0f)
            {
                float totalLevels = std::pow(2.0f, juce::jmap(digitalBitcrush, 16.0f, 2.0f));
                s = std::round(s * totalLevels) / totalLevels;
            }
            samples[i] = s;
        }
    }
}

void UniversalDistortionModule::processFM(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (keyTracker == nullptr) return;

    auto currentFreq = keyTracker->getCurrentFrequency();
    auto targetFreq = juce::jlimit(20.0, 20000.0, currentFreq * fmRatio);
    fmPhaseDelta = (targetFreq * juce::MathConstants<float>::twoPi) / static_cast<float>(sampleRate);

    auto& block = context.getOutputBlock();
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* samples = block.getChannelPointer(ch);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            const float modulatedPhase = fmPhase + (samples[i] * fmIndex);
            samples[i] = std::sin(modulatedPhase);
            fmPhase += fmPhaseDelta;
        }
    }
    fmPhase = std::fmod(fmPhase, juce::MathConstants<float>::twoPi);
}

void UniversalDistortionModule::processRodent(const juce::dsp::ProcessContextReplacing<float>& context)
{
    rodentInputGain.process(context);
    auto& block = context.getOutputBlock();
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* samples = block.getChannelPointer(ch);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
            samples[i] = juce::jlimit(-1.0f, 1.0f, samples[i]);
    }
    rodentToneFilter.process(context);
    rodentOutputGain.process(context);
}

void UniversalDistortionModule::processScreamer(const juce::dsp::ProcessContextReplacing<float>& context)
{
    screamerMidBoostFilter.process(context);
    screamerInputGain.process(context);
    auto& block = context.getOutputBlock();
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto* samples = block.getChannelPointer(ch);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
            samples[i] = std::tanh(samples[i]);
    }
    screamerToneFilter.process(context);
    screamerOutputGain.process(context);
}

void UniversalDistortionModule::updateFilters()
{
    // Rodent Tone: A reverse low-pass filter.
    auto rodentCutoff = juce::jmap(rodentTone, 0.0f, 1.0f, 20000.0f, 500.0f);
    *rodentToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, rodentCutoff, 0.707f);

    // Screamer Mid-Boost: A high-pass filter to cut lows before clipping.
    *screamerMidBoostFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 720.0f);

    // Screamer Tone: A simple low-pass filter.
    auto screamerCutoff = juce::jmap(screamerTone, 0.0f, 1.0f, 15000.0f, 400.0f);
    *screamerToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, screamerCutoff, 0.707f);
}
