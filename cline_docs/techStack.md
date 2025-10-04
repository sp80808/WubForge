# WubForge Technical Stack

## Primary Framework
- **JUCE**: Cross-platform C++ framework for audio applications and plugins
  - Version: Latest (managed via CMake submodule)
  - Focus: VST3/AU plugin development, GUI components, DSP utilities

## Build System
- **CMake**: Cross-platform build configuration
  - Minimum version: 3.15+
  - Configuration: Release builds with JUCE auto-download

## Programming Language
- **C++17**: Core development language
  - Standard: C++17 compatible compiler required
  - Focus: High-performance audio processing

## Core DSP Libraries
- **JUCE DSP Module**: Built-in FFT, filtering, and audio utilities
- **Eigen** (planned): High-performance linear algebra library for spectral processing
- **Custom FFT Implementation**: Real-time spectral morphing engine

## Audio Plugin Formats
- **VST3**: Primary target format
- **AU**: macOS AudioUnit support
- Support for other formats through JUCE

## UI Framework
- **JUCE Graphics**: Core UI rendering and components
- **SpectrogramComponent**: Custom real-time spectrum visualization (Pure JUCE)
  - Real-time waterfall-style spectrogram with logarithmic frequency scaling
  - Multiple color maps (Viridis, Plasma, Hot), 30fps performance
  - Professional audio plugin standards

## Advanced DSP Libraries

### chowdsp_utils (Added - 304⭐️ 🟢 Active)
**Purpose**: Large collection of high-quality DSP utilities for professional audio processing
**Modules Integrated**:
- **chowdsp_eq**: Professional parametric EQ with multiple bands (low/mid/high shelf + peak)
- **chowdsp_filters**: Advanced filter collection (Butterworth, Chebyshev, Elliptic)
- **chowdsp_waveshapers**: Professional distortion algorithms (West Coast folder, Wave Multiplier)
- **chowdsp_dsp_utils**: Utility classes for convolution, resampling, ring buffers

**Benefits for WubForge**:
- High-quality EQ processing (replaces basic filters)
- Additional waveforms distortion algorithms
- Professional-grade DSP implementations
- Extends processing capabilities beyond basic audio effects

### ChowEQModule (New)
**Created**: Professional 3-band parametric EQ using chowdsp_utils
- Low shelf filter (200Hz), Mid peak filter (variable 200-8000Hz), High shelf filter (5kHz)
- Individual gain, Q, and frequency controls
- Integrated into WubForge's modular architecture as Slot 4

## Development Tools
- **CMake**: Build configuration and dependency management
- **Git**: Version control with submodules for JUCE
- **CLion/CLion-based IDEs**: C++ development environment

## Testing and Quality Assurance
- **JUCE UnitTestRunner**: Built-in testing framework (planned)
- **Manual testing**: Audio quality and performance validation

## Performance Requirements
- **Latency**: <5ms end-to-end processing latency
- **Real-time**: 60fps minimum for GUI updates
- **CPU**: Optimized for real-time spectral processing

## Future Technology Considerations
- **UI Libraries**: Research and integrate open source visualization components
- **Hardware Acceleration**: Potential GPU acceleration for FFT operations
- **Machine Learning**: Future AI-assisted parameter optimization
