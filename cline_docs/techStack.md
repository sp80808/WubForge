# WubForge Technology Stack

## Core Technologies
- **JUCE 7.x**: Cross-platform audio plugin framework
- **C++17**: Modern C++ standard for performance and features
- **CMake**: Build system for cross-platform compilation

## Audio Processing Libraries
- **ChowDSP Utils**: Professional DSP components (EQ, filters, dynamics)
- **JUCE DSP**: Built-in audio processing utilities (FFT, IIR, FIR)
- **Custom DSP**: Novel algorithms (Fibonacci Spiral Distort, Fractal Filters)

## Architecture Patterns
- **Modular Design**: 5-slot processing chain with factory pattern
- **MVP Architecture**: Model-View-Processor for GUI separation
- **Observer Pattern**: Parameter changes and real-time updates
- **RAII**: Resource management for audio buffers and states

## Development Tools
- **CLion/Cursor**: Primary IDE for development
- **Valgrind**: Memory leak detection and profiling
- **JUCE Projucer**: GUI layout and resource management
- **Git**: Version control with GitHub integration

## Audio Specifications
- **Sample Rates**: 44.1kHz - 192kHz support
- **Buffer Sizes**: 32 - 4096 samples
- **Latency**: Zero-latency design where possible
- **CPU Target**: <15% per plugin instance

## Plugin Formats
- **VST3**: Primary target format
- **AU**: macOS Audio Units support
- **AAX**: Pro Tools compatibility (planned)

## Testing Framework
- **JUCE Unit Tests**: Built-in testing framework
- **Audio Validation**: Spectral analysis for algorithm verification
- **Performance Profiling**: CPU and memory usage monitoring

## Novel DSP Algorithms
- **Fibonacci Spiral Distort**: Golden ratio harmonic processing
- **Fractal Filters**: Self-similar filtering algorithms
- **Sample Morphing**: Granular synthesis for bass transformation
- **Spectral Processing**: FFT-based effects and analysis

## Performance Optimizations
- **Lock-Free Parameters**: Thread-safe real-time parameter updates
- **Buffer Management**: Efficient audio buffer pooling
- **SIMD Operations**: Vectorized processing where beneficial
- **Memory Alignment**: Cache-friendly data structures
