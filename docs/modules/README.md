# WubForge Modules Documentation

This directory contains documentation for all audio processing modules in the WubForge plugin.

## Available Modules

### Core Modules
- **Universal Filter** - Fractal-based filtering with golden ratio harmonics
- **Universal Distortion** - Multi-algorithm distortion processing
- **Chow EQ** - Professional 3-band parametric EQ using chowdsp_utils
- **MDA SubSynth** - Classic sub-bass enhancement
- **Sample Morpher** - NEW: Serum 2-style granular sample-to-bass processor

## Module Categories

### Filter Modules
- Universal Filter
- Chow EQ

### Distortion Modules
- Universal Distortion
- MDA SubSynth

### Special Modules
- Sample Morpher (Filter type for modular chain compatibility)

## Sample Morpher Module

### Overview
The SampleMorpher is a Serum 2-inspired granular sample processor that transforms any audio sample into bass material through advanced granular synthesis techniques.

### Key Features
- **Drag-and-drop sample loading** (.wav files via AudioFormatManager)
- **512-sample granular synthesis** with windowed overlap-add
- **Key-tracked grain rate modulation** for responsive bass behavior
- **Envelope-controlled position/depth modulation** for dynamic expression
- **20-80% wet/dry morphing** with input signal blending
- **Real-time parameter adjustment** with smooth interpolation

### Technical Implementation
- **Granular Engine**: 512-sample grains with Hanning windowing
- **FFT Processing**: 1024-point FFT for spectral morphing (future enhancement)
- **Key Tracking**: MIDI-responsive grain rate adjustment
- **Envelope System**: ADSR-based modulation with transient triggering
- **Memory Efficient**: Optimized buffer management for real-time performance

### Parameters
- **Morph Amount** (0.0-1.0): Blend between input signal and sample grain
- **Grain Size** (128-2048): Size of individual audio grains
- **Grain Overlap** (0.0-0.75): Overlap percentage between grains
- **Position Modulation** (0.0-1.0): Envelope modulation depth
- **Key Track Amount** (0.0-1.0): MIDI key tracking sensitivity
- **Attack/Release Time** (0.001-2.0s): Envelope response timing

### Usage in Modular Chain
The SampleMorpher integrates seamlessly into WubForge's 5-slot modular architecture, functioning as a filter-type module that can be positioned anywhere in the processing chain.

### Performance Characteristics
- **CPU Usage**: <15% with typical granular settings
- **Latency**: Zero additional latency (sample-accurate processing)
- **Memory**: ~2MB per loaded sample (depending on length)
- **Real-time Safe**: Lock-free parameter updates

## Future Enhancements
- **FFT-based spectral morphing** for advanced timbral transformations
- **Multi-sample layering** for complex bass textures
- **Advanced windowing functions** (Kaiser, Blackman-Harris)
- **Grain cloud processing** for ambient bass design
- **Sample reverse/randomize** for experimental sound design
