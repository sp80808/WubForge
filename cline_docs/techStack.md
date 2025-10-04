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
- **Open Source UI Libraries**: Currently researching JUCE-compatible visualization libraries
  - Target: Real-time spectrogram and spectrum analyzer components
  - Requirements: Open source, real-time capable, professional quality

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
