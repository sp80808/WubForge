 # WubForge - Spectral Bass Processor

**WubForge** is a revolutionary spectral processing plugin that transforms bass sounds through real-time FFT morphing, fractal filtering, and intelligent key-tracking. Inspired by Filterverse's fractal filters and Serum 2's spectral innovations, WubForge delivers professional bass sculpting with <5ms latency for studio production and live performance.

## Revolutionary Spectral Processing

WubForge introduces **spectral morphing** - the ability to morph between spectral envelopes in real-time using XY control. This creates evolving, harmonically rich bass textures that respond musically to MIDI input while maintaining <5ms latency for professional production workflows.

### Core Technologies

- **Spectral Morphing Engine**: Real-time FFT-based morphing between source and target spectra
- **Fractal Filtering**: Self-similar filter banks with golden ratio scaling for natural harmonics
- **Key-Tracked Processing**: All spectral parameters respond to MIDI for musical consistency
- **Dual Filter Types**: Low-pass and band-pass fractal variants for different sonic characters
- **Modular Architecture**: 4-slot processing chain with serial/parallel routing options

Forget endless tweaking and muddy results. WubForge's intelligent, key-tracked engine ensures that even the most complex and chaotic processing stays locked to your music, delivering professional bass tones in seconds.

## Core Workflow

1.  **Modular Chain**: Drag and drop modules into a 4-slot effects chain. The first two slots are filter-focused, and the last two are distortion-focused.
2.  **Advanced Routing**: Instantly change the signal flow with the Routing Matrix. Choose between:
    -   **Serial**: A classic linear chain (A -> B -> C -> D).
    -   **Parallel**: Two separate chains (A->B and C->D) are processed independently and mixed together.
    -   **Mid-Side** (Planned): Process the mid and side signals of your audio separately.
    -   **Feedback** (Planned): Feed the output of the chain back into the input for resonant, Roar-style effects.
3.  **Intelligent Output**: A final post-processing stage automatically adds harmonic excitement, carves out mud with an adaptive EQ, and applies stereo widening to ensure your bass always cuts through the mix.

## Features

-   **Semi-Modular Engine**: 4-slot chain for loading a variety of filter and distortion modules.
-   **Advanced Routing**: Switch between Serial and Parallel signal flows instantly. (Mid-Side and Feedback coming soon!).
-   **Always-On Key Tracking**: All frequency-dependent parameters (filter cutoffs, delay times) automatically track the incoming MIDI or detected audio pitch.
-   **Innovative Filter Library**: A growing collection of classic and futuristic filters.
    -   **Implemented**: `Fractal Filter` - A unique, self-similar filter that creates rich harmonic textures through recursive processing.
    -   **Planned**: Classic Ladder, State Variable, Formant Bank, Spectral Notches, Karplus-Strong Pluck.
-   **Versatile Distortion Library**: A suite of algorithms for everything from subtle warmth to extreme destruction.
    -   **Implemented**: `DistortionForge` (Wavefolder, Clipper, Bitcrusher), `FormantTracker`.
    -   **Planned**: Dynamic Alkane, Ohmicide Swarm, FM Distort.
-   **"Magic Forge" Button** (Planned): Intelligently randomizes modules and their parameters while constraining them to musically-consonant ratios.
-   **Real-time Visual Feedback** (Planned): A full-screen spectrograph will show the harmonic evolution of your sound in real-time.

## Building from Source

### Prerequisites
- CMake 3.15+
- A C++17 compatible compiler
- JUCE (will be downloaded automatically by CMake)

### Build Instructions

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd WubForge
    ```

2.  **Configure with CMake:**
    ```bash
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```

3.  **Build the plugin:**
    ```bash
    cmake --build . --config Release
    ```

After a successful build, the VST3, AU, or other plugin formats will be located in the `build/WubForge_artefacts` directory.

## Development

The WubForge architecture is designed to be modular and extensible.

### Project Structure
```
Source/
├── PluginProcessor.h/cpp    # Main plugin class, audio processing logic.
├── PluginEditor.h/cpp       # GUI implementation.
|
├── Module.h                 # Defines the abstract AudioModule interface and Routing enum.
├── ModuleWrappers.h         # Contains wrappers for legacy DSP classes.
|
├── FractalFilter.h/cpp      # Implementation of the Fractal Filter module.
├── CombStack.h/cpp          # (Legacy) Comb filter implementation.
├── DistortionForge.h/cpp    # (Legacy) Distortion chain implementation.
├── FormantTracker.h/cpp     # (Legacy) Formant tracking filter.
|
├── KeyTracker.h/cpp         # MIDI key tracking.
└── Presets.h/cpp            # Preset management.
```

### Adding a New Module

1.  Create a new class that inherits from `FilterModule` or `DistortionModule` (defined in `Source/Module.h`).
2.  Implement the required virtual methods: `prepare`, `process`, `reset`, `getName`, and `getType`.
3.  Add any new `AudioParameter`s for your module to `PluginProcessor::createParameterLayout()`.
4.  In `PluginProcessor::updateDSPParameters()`, add a block to check for your new module type and call its parameter setters.
5.  (Optional) To test, instantiate your new module in one of the `moduleSlots` in the `PluginProcessor`'s constructor.

## License

This project is provided as-is for educational and development purposes.
