# WubForge Audio Modules

WubForge is built around a modular architecture, allowing for flexible signal processing chains. This document details the available audio modules, their functionalities, and guidelines for extending the module system.

## Module Types

All audio modules in WubForge inherit from either `FilterModule` or `DistortionModule` (defined in `Source/Module.h`), which in turn inherit from `AudioModule`. These base classes provide a common interface for audio processing, parameter management, and module identification.

## Existing Modules

### SampleMorpher
- **Purpose**: Granular synthesis module for transforming samples into bass sounds, inspired by Serum 2. Supports drag-and-drop .wav files.
- **Key Features**: Real-time granular processing, sample manipulation, spectral shaping.
- **Location**: `Source/SampleMorpher.h/cpp`

### FractalFilter
- **Purpose**: A unique, self-similar filter that creates rich harmonic textures through recursive processing. Uses golden ratio scaling for natural harmonics.
- **Key Features**: Self-similar filter banks, golden ratio scaling, dual filter types (low-pass and band-pass).
- **Location**: `Source/FractalFilter.h/cpp`

### UniversalFilterModule
- **Purpose**: Professional filtering module with integration of ChowDSP utilities. Provides a versatile set of filtering options.
- **Key Features**: Professional EQ, various filter types, ChowDSP integration.
- **Location**: `Source/UniversalFilterModule.h/cpp`

### UniversalDistortionModule
- **Purpose**: A comprehensive distortion module that can switch between multiple classic and modern distortion models.
- **Key Features**: Digital (Wavefolder, Bitcrusher), FM, Rodent (ProCo RAT-style), Screamer (Tube Screamer-style) models.
- **Location**: `Source/UniversalDistortionModule.h/cpp`

### MDASubSynthModuleDirect
- **Purpose**: Classic sub bass enhancement module, likely based on MDA plugins.
- **Key Features**: Sub-harmonic generation, bass thickening.
- **Location**: `Source/MDASubSynthModuleDirect.h/cpp`

### Fibonacci Spiral Distort (FSD)
- **Purpose**: Novel hybrid filter-distortion module creating self-similar, consonant harmonics using golden ratio (φ ≈ 1.618) spacing and Fibonacci approximations.
- **Key Features**: φ-resonator bank, Fibonacci distortion cascade, spiral veil filter, real-time performance optimizations.
- **Location**: `Source/FibonacciSpiralDistort.h/cpp`

## Supporting Components (Not directly Audio Modules but crucial for DSP)

### KeyTracker
- **Purpose**: Detects and tracks incoming MIDI or audio pitch to ensure all frequency-dependent parameters respond musically.
- **Location**: `Source/KeyTracker.h/cpp`

### FormantTracker
- **Purpose**: Analyzes vocal formants for morphing effects.
- **Location**: `Source/FormantTracker.h/cpp`

### SpectrogramComponent
- **Purpose**: Provides real-time spectral visualization of the audio signal.
- **Location**: `Source/SpectrogramComponent.h/cpp`

## Adding a New Module

To add a new audio processing module to WubForge, follow these steps:

1.  **Create a new class**: Your new class must inherit from either `FilterModule` or `DistortionModule` (defined in `Source/Module.h`), depending on its primary function.
2.  **Implement required virtual methods**:
    *   `void prepare (const juce::dsp::ProcessSpec& spec)`: Initialize any DSP components and set the sample rate/block size.
    *   `void process (const juce::dsp::ProcessContextReplacing<float>& context)`: Implement the core audio processing logic.
    *   `void reset()`: Reset any internal states or filters.
    *   `const juce::String getName() const`: Return the display name of your module.
    *   `const juce::String getType() const`: Return a unique identifier string for your module type.
3.  **Add parameters**: If your module requires user-adjustable parameters, add them to the `PluginProcessor::createParameterLayout()` method. Ensure they are properly managed within the JUCE `AudioProcessorValueTreeState`.
4.  **Update DSP parameters**: In `PluginProcessor::updateDSPParameters()`, add a block to check for your new module type and call its parameter setters to update the module's internal state based on the current parameter values.
5.  **Integrate into the module factory**: Modify the module factory system (likely within `PluginProcessor`'s constructor or a dedicated factory class) to instantiate your new module when its type is selected.
6.  **(Optional) Create a GUI component**: If your module requires a custom graphical interface, create a corresponding `juce::Component` and integrate it with `PluginEditor`.
7.  **Test**: Thoroughly test your new module to ensure it functions correctly, is audio-thread safe, and meets performance targets.

## Legacy Modules

The following modules are considered legacy. While they may still exist in the codebase, new development should focus on using or extending the more modern `UniversalFilterModule` and `UniversalDistortionModule`, or creating new modules following the guidelines above.

*   **CombStack**: (Legacy) Comb filter implementation.
*   **DistortionForge**: (Legacy) Distortion chain implementation. Superseded by `UniversalDistortionModule`.
*   **FormantTracker**: (Legacy) Formant tracking filter. (Note: A `FormantTracker` is also listed under Supporting Components, indicating a potential overlap or evolution. Clarification needed if this legacy version is distinct from the supporting component.)
