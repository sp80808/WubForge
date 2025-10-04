#include "UniversalFilterModule.h"

UniversalFilterModule::UniversalFilterModule()
    : forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann),
      combLfo([](float x) { return std::sin(x); }) // Sine wave for comb LFO
{
    fractalFilterChain.resize(8);
}

void UniversalFilterModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Prepare all DSP components
    for (auto& f : fractalFilterChain) f.prepare(spec);
    spectralOutputBuffer.setSize(spec.numChannels, fftSize);
    pluckDelayLine.prepare(spec);
    pluckFilter.prepare(spec);
    for (auto& f : formantFilters) f.prepare(spec);
    for (auto& d : combDelayLines) d.prepare(spec);
    combLfo.prepare(spec);

    reset();
}

void UniversalFilterModule::reset()
{
    for (auto& f : fractalFilterChain) f.reset();
    fifo.fill(0.0f); fifoIndex = 0; spectralOutputBuffer.clear(); spectralOutputPos = 0;
    pluckDelayLine.reset(); pluckFilter.reset(); needsToPluck = true;
    for (auto& f : formantFilters) f.reset();
    for (auto& d : combDelayLines) d.reset();
    combLfo.reset();

    fractalNeedsUpdate = true;
    formantNeedsUpdate = true;
}

void UniversalFilterModule::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    switch (currentModel)
    {
        case Model::Fractal:   processFractal(context);   break;
        case Model::Spectral:  processSpectral(context);  break;
        case Model::Pluck:     processPluck(context);     break;
        case Model::Formant:   processFormant(context);   break;
        case Model::Comb:      processComb(context);      break;
    }
}

//==============================================================================
// --- Parameter Setters ---
void UniversalFilterModule::setModel(Model newModel) { if (currentModel != newModel) { currentModel = newModel; reset(); } }
void UniversalFilterModule::pluck() { needsToPluck = true; }

void UniversalFilterModule::setFractalType(int t) { if(fractalFilterType != t) { fractalFilterType = t; fractalNeedsUpdate = true; } }
void UniversalFilterModule::setFractalFreq(float f) { if(fractalBaseFrequency != f) { fractalBaseFrequency = f; fractalNeedsUpdate = true; } }
void UniversalFilterModule::setFractalQ(float q) { if(fractalQ != q) { fractalQ = q; fractalNeedsUpdate = true; } }
void UniversalFilterModule::setFractalDepth(int d) { if(fractalDepth != d) { fractalDepth = d; fractalNeedsUpdate = true; } }
void UniversalFilterModule::setFractalRatio(float r) { if(fractalRatio != r) { fractalRatio = r; fractalNeedsUpdate = true; } }

void UniversalFilterModule::setSpectralMode(int m) { spectralMode = m; }
void UniversalFilterModule::setSpectralFreq(float f) { spectralFrequency = f; }
void UniversalFilterModule::setSpectralBw(float bw) { spectralBandwidth = bw; }

void UniversalFilterModule::setPluckDecay(float d) { if(pluckDecay != d) { pluckDecay = d; updateFilters(); } }
void UniversalFilterModule::setPluckDamping(float d) { if(pluckDamping != d) { pluckDamping = d; updateFilters(); } }

void UniversalFilterModule::setFormantKeyTrack(float a) { if(formantKeyTrack != a) { formantKeyTrack = a; formantNeedsUpdate = true; } }
void UniversalFilterModule::setFormantGain(float g) { if(formantGain != g) { formantGain = g; formantNeedsUpdate = true; } }
void UniversalFilterModule::setFormantQ(float q) { if(formantQ != q) { formantQ = q; formantNeedsUpdate = true; } }
void UniversalFilterModule::setFormantBaseFreq(double f) { if(formantBaseFrequency != f) { formantBaseFrequency = f; formantNeedsUpdate = true; } }

void UniversalFilterModule::setCombCount(int c) { combCount = c; }
void UniversalFilterModule::setCombDelay(float d) { combDelay = d; }
void UniversalFilterModule::setCombFeedback(float fb) { combFeedback = fb; }
void UniversalFilterModule::setCombLfoRate(float r) { combLfoRate = r; }
void UniversalFilterModule::setCombLfoDepth(float d) { combLfoDepth = d; }

//==============================================================================
// --- Internal Processing & Helper Functions ---

void UniversalFilterModule::updateFilters()
{
    // Fractal
    if (fractalNeedsUpdate)
    {
        float currentFreq = fractalBaseFrequency;
        for (int i = 0; i < fractalDepth; ++i)
        {
            switch (fractalFilterType) {
                case 0: *fractalFilterChain[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, currentFreq, fractalQ); break;
                case 1: *fractalFilterChain[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, currentFreq, fractalQ); break;
                case 2: *fractalFilterChain[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, currentFreq, fractalQ); break;
            }
            currentFreq *= fractalRatio;
        }
        fractalNeedsUpdate = false;
    }

    // Pluck
    auto pluckCutoff = juce::jmap(pluckDecay, 0.0f, 1.0f, 8000.0f, 100.0f);
    auto pluckQ = juce::jmap(pluckDamping, 0.0f, 1.0f, 0.707f, 2.0f);
    *pluckFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, pluckCutoff, pluckQ);

    // Formant
    if (formantNeedsUpdate && keyTracker != nullptr)
    {
        double currentKeyFreq = keyTracker->getCurrentFrequency();
        double baseFreqRatio = currentKeyFreq / formantBaseFrequency;
        double tracking = juce::jlimit(0.0, 1.0, (double)formantKeyTrack);
        double scaleFactor = juce::jmap(tracking, 1.0, baseFreqRatio);

        for (int i = 0; i < numFormants; ++i)
        {
            double formantFreq = baseFormants[i] * scaleFactor;
            formantFreq = juce::jlimit(20.0, sampleRate / 2.1, formantFreq);
            *formantFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, (float)formantFreq, formantQ, juce::Decibels::decibelsToGain(formantGain));
        }
        formantNeedsUpdate = false;
    }
}

void UniversalFilterModule::processFractal(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (fractalNeedsUpdate) updateFilters();
    for (int i = 0; i < fractalDepth; ++i) fractalFilterChain[i].process(context);
}

void UniversalFilterModule::processSpectral(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    for (size_t i = 0; i < inputBlock.getNumSamples(); ++i)
    {
        float inputSample = 0.0f;
        for (size_t ch = 0; ch < inputBlock.getNumChannels(); ++ch) inputSample += inputBlock.getSample(ch, i);
        inputSample /= static_cast<float>(inputBlock.getNumChannels());

        fifo[fifoIndex] = inputSample;
        if (++fifoIndex >= hopSize)
        {
            fifoIndex = 0;
            std::rotate(fifo.begin(), fifo.begin() + hopSize, fifo.end());
            std::copy(fifo.begin() + (fftSize - hopSize), fifo.end(), fftBuffer.begin());
            window.multiplyWithWindowingTable(fftBuffer.data(), fftSize);
            std::copy(fftBuffer.begin(), fftBuffer.end(), workspace.begin());
            forwardFFT.performRealOnlyForwardTransform(workspace.data());

            const float binWidth = (float)sampleRate / fftSize;
            for (int j = 0; j < fftSize / 2; ++j)
            {
                const float currentFreq = j * binWidth;
                if (spectralMode == 0) { // Notch
                    if (std::abs(currentFreq - spectralFrequency) < spectralBandwidth / 2.0f) {
                        workspace[j * 2] = workspace[j * 2 + 1] = 0.0f;
                    }
                } else { // Comb
                    const float harmonicRatio = currentFreq / spectralFrequency;
                    if (std::abs(harmonicRatio - std::round(harmonicRatio)) * spectralFrequency < spectralBandwidth / 2.0f) {
                        workspace[j * 2] *= 1.5f; workspace[j * 2 + 1] *= 1.5f;
                    } else {
                        workspace[j * 2] *= 0.5f; workspace[j * 2 + 1] *= 0.5f;
                    }
                }
            }
            forwardFFT.performRealOnlyInverseTransform(workspace.data());
            for (int j = 0; j < fftSize; ++j) spectralOutputBuffer.addSample(0, (spectralOutputPos + j) % fftSize, workspace[j]);
        }

        float outputSample = spectralOutputBuffer.getSample(0, spectralOutputPos);
        spectralOutputBuffer.setSample(0, spectralOutputPos, 0.0f);
        spectralOutputPos = (spectralOutputPos + 1) % fftSize;
        for (size_t ch = 0; ch < outputBlock.getNumChannels(); ++ch) outputBlock.setSample(ch, i, outputSample);
    }
}

void UniversalFilterModule::processPluck(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (needsToPluck) {
        if (keyTracker == nullptr) return;
        const float freq = juce::jlimit(20.0f, 20000.0f, (float)keyTracker->getCurrentFrequency());
        const int delaySamples = static_cast<int>(sampleRate / freq);
        pluckDelayLine.setDelay(delaySamples);
        for (int i = 0; i < delaySamples; ++i) pluckDelayLine.pushSample(0, juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f);
        needsToPluck = false;
    }

    auto& outputBlock = context.getOutputBlock();
    for (size_t i = 0; i < outputBlock.getNumSamples(); ++i)
    {
        float delayedSample = pluckDelayLine.popSample(0);
        float filteredSample = filter.processSample(delayedSample);
        pluckDelayLine.pushSample(0, filteredSample * 0.995f); // Damping
        for (size_t ch = 0; ch < outputBlock.getNumChannels(); ++ch) outputBlock.setSample(ch, i, delayedSample);
    }
}

void UniversalFilterModule::processFormant(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (formantNeedsUpdate) updateFilters();
    for (auto& f : formantFilters) f.process(context);
}

void UniversalFilterModule::processComb(const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (keyTracker == nullptr) return;
    combLfo.setFrequency(combLfoRate);
    float lfoSample = combLfo.processSample(0.0f) * combLfoDepth;

    float baseDelaySamples = (combDelay / 1000.0f) * (float)sampleRate;
    float keyTrackedFreq = (float)keyTracker->getCurrentFrequency();
    float keyTrackedDelay = (keyTrackedFreq > 0) ? (float)sampleRate / keyTrackedFreq : 0.0f;

    auto& block = context.getOutputBlock();
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        for (int i = 0; i < combCount; ++i)
        {
            float modulatedDelay = baseDelaySamples + lfoSample * 20.0f + keyTrackedDelay;
            combDelayLines[i].setDelay(modulatedDelay);
            float delayedSample = combDelayLines[i].popSample(ch);
            combDelayLines[i].pushSample(ch, block.getSample(ch, 0) + delayedSample * combFeedback);
            block.setSample(ch, 0, delayedSample);
        }
    }
}
