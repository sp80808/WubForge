#include "FractalFilter.h"

FractalFilterModule::FractalFilterModule()
{
    filterChain.resize (maxDepth);
    coefficients.resize (maxDepth);
}

void FractalFilterModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    for (auto& filter : filterChain)
        filter.prepare(spec);

    needsUpdate = true;
}

void FractalFilterModule::reset()
{
    for (auto& filter : filterChain)
        filter.reset();
}

void FractalFilterModule::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    if (needsUpdate)
        updateCoefficients();

    for (int i = 0; i < depth; ++i)
    {
        filterChain[i].process(context);
    }
}

void FractalFilterModule::setType (int type)
{
    if (type != filterType)
    {
        filterType = type;
        needsUpdate = true;
    }
}

void FractalFilterModule::setBaseFrequency (float freqHz)
{
    if (freqHz != baseFrequency)
    {
        baseFrequency = freqHz;
        needsUpdate = true;
    }
}

void FractalFilterModule::setQ (float newQ)
{
    if (newQ != q)
    {
        q = newQ;
        needsUpdate = true;
    }
}

void FractalFilterModule::setDepth (int newDepth)
{
    // Clamp depth to the valid range
    newDepth = juce::jlimit (1, (int)maxDepth, newDepth);
    if (newDepth != depth)
    {
        depth = newDepth;
        needsUpdate = true; // Coefficients for active filters might not change, but the chain length does
    }
}

void FractalFilterModule::setRatio (float newRatio)
{
    if (newRatio != ratio)
    {
        ratio = newRatio;
        needsUpdate = true;
    }
}

void FractalFilterModule::updateCoefficients()
{
    float currentFreq = baseFrequency;

    for (int i = 0; i < depth; ++i)
    {
        switch (filterType)
        {
            case 0: // Low-pass
                *coefficients[i] = *Coefficients::makeLowPass (sampleRate, currentFreq, q);
                break;
            case 1: // High-pass
                *coefficients[i] = *Coefficients::makeHighPass (sampleRate, currentFreq, q);
                break;
            case 2: // Band-pass
                *coefficients[i] = *Coefficients::makeBandPass (sampleRate, currentFreq, q);
                break;
        }

        filterChain[i].coefficients = coefficients[i];
        
        // Apply the fractal ratio for the next iteration
        currentFreq *= ratio;
    }

    needsUpdate = false;
}