# WubForge Codebase Summary

## Project Overview
WubForge is a JUCE-based VST3 audio plugin implementing spectral bass processing with features like fractal filtering, spectral morphing, and modular processing chains.

## Project Structure

### Source/ Directory
```
├── PluginProcessor.h/cpp      # Main plugin class, audio processing logic
├── PluginEditor.h/cpp         # GUI implementation and visualization
├── Module.h                   # Abstract interface for audio processing modules
├── ModuleWrappers.h           # Legacy DSP class adapters
├── KeyTracker.h/cpp           # MIDI key tracking system
├── Presets.h/cpp              # Preset management functionality
│
├── Spectral Processing Modules:
│   ├── FractalFilter.h/cpp          # Self-similar harmonic filter
│   ├── BandpassFractalFilter.h/cpp  # Bandpass variant of fractal filter
│   ├── SpectralMorphingModule.h/cpp # FFT-based spectral morphing
│   └── (Legacy components)
│
├── Filter Modules:
│   ├── (Fractal filters implemented)
│   └── (Additional filters planned)
│
└── Distortion Modules:
    ├── DistortionForge.h/cpp    # Multi-algorithm distortion
    ├── BitCrusher.h/cpp         # Bit depth reduction effect
    ├── FormantTracker.h/cpp     # Formant-based processing
    └── FMDistortModule.h/cpp    # FM distortion (legacy)
```

## Key Components

### Core Architecture
- **PluginProcessor**: Main audio processing hub managing the 4-slot modular chain
- **PluginEditor**: GUI controller implementing parameter controls and visualizations
- **Module Interface**: Abstract base class defining the contract for all processing modules

### DSP Engine
- **Real-time Processing**: <5ms latency spectral processing
- **Key Tracking**: Automatic parameter adjustment based on MIDI input
- **Modular Chain**: 4-slot processing pipeline with routing options
- **FFT-based Morphing**: Real-time spectral envelope manipulation

### Filter Library
- **Fractal Filters**: Golden ratio-based harmonic scaling
- **Formant Processing**: Intelligent vowel/consonant shaping
- **Legacy Filters**: Comb filtering, bandpass variants

## Current Development Status

### Implemented Features
- Spectral morphing engine with FFT processing
- Fractal filtering system (LPF and BPF variants)
- Key-tracking for frequency-dependent parameters
- Distortion module (DistortionForge) with multiple algorithms
- Bit-crushing and formant tracking modules
- Basic modular architecture (4-slot chain)

### Active Development
- **UI Enhancement**: Seeking open source visualization libraries for spectrograms
- **Modular System**: Expanding routing options (Serial/Parallel/Mid-Side)
- **Module Library**: Adding classic filter types and distortion algorithms

## External Dependencies
- **JUCE Framework**: Complete audio plugin development ecosystem
  - Location: JUCE/ submodule (automatically managed via CMake)
  - Modules used: juce_core, juce_audio_processors, juce_gui_basics, juce_dsp
  - Purpose: Plugin framework, GUI components, basic DSP utilities

## Data Flow
1. Audio input → PluginProcessor → Modular chain routing
2. Individual modules process audio with parameter modulation
3. KeyTracker adjusts frequency-dependent parameters based on MIDI
4. Processed audio → Output with post-processing (planned)

## Configuration Management
- **CMakeLists.txt**: Build configuration with JUCE integration
- **Git submodules**: JUCE dependency management
- **Preset system**: User configuration storage and recall

## Recent Significant Changes
- ✅ Implemented SpectralMorphingModule for real-time FFT morphing
- ✅ Added BandpassFractalFilter as complement to FractalFilter
- ✅ Enhanced key-tracking system for improved musical consistency
- ✅ Modular architecture expansion with better routing options
- 🔄 **Current**: Researching open source UI libraries for visualization

## Future Architecture Considerations
- Add GPU acceleration for FFT operations
- Implement machine learning for intelligent parameter optimization
- Expand module library with community-contributed algorithms
- Add hardware control surface integration

## Performance Characteristics
- **Latency**: Maintained <5ms for professional use
- **CPU**: Optimized spectral processing algorithms
- **Memory**: Efficient FFT buffer management
- **Threading**: Real-time safe processing with JUCE message thread
