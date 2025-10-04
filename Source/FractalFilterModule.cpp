#include "FractalFilterModule.h"

//==============================================================================
FractalFilterModule::FractalFilterModule()
{
}

FractalFilterModule::~FractalFilterModule()
{
}

//==============================================================================
void FractalFilterModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    fractalFilter.prepareToPlay (spec.sampleRate, spec.maximumBlockSize);
}

void FractalFilterModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    // Create a non-const copy for the fractal filter
    auto nonConstContext = context;
    fractalFilter.process (nonConstContext);
}

void FractalFilterModule::reset()
{
    fractalFilter.reset();
}

//==============================================================================
void FractalFilterModule::setCurrentFreq (double freq)
{
    fractalFilter.setCurrentFreq (freq);
}

void FractalFilterModule::setDepth (int depth)
{
    fractalFilter.setDepth (depth);
}

void FractalFilterModule::setScaleFactor (float scale)
{
    fractalFilter.setScaleFactor (scale);
}

void FractalFilterModule::setMix (float mix)
{
    fractalFilter.setMix (mix);
}

void FractalFilterModule::setBaseCutoff (float cutoffHz)
{
    fractalFilter.setBaseCutoff (cutoffHz);
}
