#include "MDASubSynthModuleDirect.h"
#include <cmath>

//==============================================================================
MDASubSynthModuleDirect::MDASubSynthModuleDirect()
{
    updateParameters();
    updateFilterCoefficients();
}

//==============================================================================
void MDASubSynthModuleDirect::prepare(const juce::dsp::ProcessSpec& spec)
{
    _sampleRate = spec.sampleRate;
    reset();
    updateFilterCoefficients();
}

void MDASubSynthModuleDirect::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& buffer = context.getOutputBlock();
    const int numSamples = buffer.getNumSamples();

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Mono processing - average left and right channels
        float input = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            input += buffer.getSample(ch, sample);
        input *= 0.5f;

        // First two low-pass filter stages on input
        _filt1 = (_filto * _filt1) + (_filti * input);
        _filt2 = (_filto * _filt2) + (_filti * _filt1);

        float sub = 0.0f;

        if (_type != 3) // Distort, Divide, Invert modes
        {
            // Create harsh binary signal based on threshold
            if (_filt2 > _threshold)
                sub = 1.0f;
            else if (_filt2 < -_threshold)
                sub = -1.0f;
            else
                sub = 0.0f;

            // Octave divider (flip sign and phase on zero crossings)
            if (sub * _sign < 0.0f)
            {
                _sign = -_sign;
                if (_sign < 0.0f) _phase = -_phase;
            }

            // Mode-specific processing
            if (_type == 1) // Divide mode
            {
                sub = _phase * sub;
            }
            else if (_type == 2) // Invert mode
            {
                sub = _phase * _filt2 * 2.0f;
            }
        }
        else // Key Osc mode
        {
            // Envelope follows input level
            if (_filt2 > _threshold)
            {
                _env = 1.0f;
            }
            else
            {
                _env *= _decay;
            }

            // Generate sine wave oscillator
            sub = _env * std::sin(_oscPhase);
            _oscPhase = std::fmod(_oscPhase + _phaseInc, 6.283185307f);
        }

        // Final two low-pass filter stages on sub-bass signal
        _filt3 = (_filto * _filt3) + (_filti * sub);
        _filt4 = (_filto * _filt4) + (_filti * _filt3);

        // Mix dry/wet and write back to all channels
        float output = (input * _dry) + (_filt4 * _wet);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample(ch, sample, output);
    }

    // Prevent numerical underflow
    if (std::abs(_filt1) < 1.0e-10f) _filt1 = 0.0f;
    if (std::abs(_filt2) < 1.0e-10f) _filt2 = 0.0f;
    if (std::abs(_filt3) < 1.0e-10f) _filt3 = 0.0f;
    if (std::abs(_filt4) < 1.0e-10f) _filt4 = 0.0f;
}

void MDASubSynthModuleDirect::reset()
{
    _oscPhase = 0.0f;
    _env = 0.0f;
    _filt1 = _filt2 = _filt3 = _filt4 = 0.0f;
    _sign = 1.0f;
    _phase = 1.0f;
}

//==============================================================================
void MDASubSynthModuleDirect::setType(int type)
{
    _type = juce::jmax(0, juce::jmin(3, type));
    updateFilterCoefficients();
}

void MDASubSynthModuleDirect::setWetMix(float wet)
{
    _wet = juce::jmax(0.0f, juce::jmin(1.0f, wet));
}

void MDASubSynthModuleDirect::setDryMix(float dry)
{
    _dry = juce::jmax(0.0f, juce::jmin(1.0f, dry));
}

void MDASubSynthModuleDirect::setThreshold(float thresholdDB)
{
    float clampedDB = juce::jmax(-60.0f, juce::jmin(0.0f, thresholdDB));
    _threshold = juce::Decibels::decibelsToGain(clampedDB);
}

void MDASubSynthModuleDirect::setTune(float tune)
{
    _tune = juce::jmax(0.0f, juce::jmin(1.0f, tune));
    updateFilterCoefficients();
}

//==============================================================================
void MDASubSynthModuleDirect::updateParameters()
{
    // Initialize state
    _sign = 1.0f;
    _phase = 1.0f;
    updateFilterCoefficients();
}

void MDASubSynthModuleDirect::updateFilterCoefficients()
{
    // In Key Osc mode, use fixed frequency; otherwise scale with tune parameter
    _filti = (_type == 3) ? 0.018f : std::pow(10.0f, -3.0f + (2.0f * _tune));
    _filto = 1.0f - _filti;

    // Phase increment for Key Osc mode
    _phaseInc = 0.456159f * std::pow(10.0f, -2.5f + (1.5f * _tune));

    // Decay factor for Key Osc envelope
    float fParam6 = 0.65f; // Default release parameter
    _decay = 1.0f - std::pow(10.0f, -2.0f - (3.0f * fParam6));
}
