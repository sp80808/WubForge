#pragma once

#include "Module.h"

//==============================================================================
/**
    A universal distortion module that can switch between multiple classic
    and modern distortion models.
*/
class UniversalDistortionModule : public DistortionModule
{
public:
    enum class Model
    {
        Digital,    // Wavefolder, Bitcrusher
        FM,         // FM Synthesis-based distortion
        Rodent,     // ProCo RAT-style hard clipping
        Screamer    // Tube Screamer-style soft clipping
    };

    UniversalDistortionModule();

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void process (const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    const juce::String getName() const override { return "Universal Distortion"; }

    //==============================================================================
    // --- Parameter Setters ---
    void setModel (Model newModel);

    // Digital Model
    void setDigitalWavefold (float amount);
    void setDigitalBitcrush (float amount);

    // FM Model
    void setFmRatio (float ratio);
    void setFmIndex (float index);

    // Rodent Model
    void setRodentDrive (float drive);
    void setRodentTone (float tone);
    void setRodentLevel (float level);

    // Screamer Model
    void setScreamerDrive (float drive);
    void setScreamerTone (float tone);
    void setScreamerLevel (float level);

    // --- Getters ---
    Model getModel() const { return currentModel; }

private:
    //==============================================================================
    // --- Internal Processing Functions ---
    void processDigital (const juce::dsp::ProcessContextReplacing<float>& context);
    void processFM (const juce::dsp::ProcessContextReplacing<float>& context);
    void processRodent (const juce::dsp::ProcessContextReplacing<float>& context);
    void processScreamer (const juce::dsp::ProcessContextReplacing<float>& context);

    void updateFilters();

    //==============================================================================
    // --- State & Parameters ---
    double sampleRate = 44100.0;
    Model currentModel = Model::Digital;

    // --- DSP Components for All Models ---

    // Digital
    float digitalWavefold = 0.0f;
    float digitalBitcrush = 0.0f;

    // FM
    float fmPhase = 0.0f;
    float fmPhaseDelta = 0.0f;
    float fmRatio = 1.0f;
    float fmIndex = 0.0f;

    // Rodent
    juce::dsp::Gain<float> rodentInputGain;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> rodentToneFilter;
    juce::dsp::Gain<float> rodentOutputGain;
    float rodentTone = 0.5f;

    // Screamer
    juce::dsp::Gain<float> screamerInputGain;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> screamerMidBoostFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> screamerToneFilter;
    juce::dsp::Gain<float> screamerOutputGain;
    float screamerTone = 0.5f;
};