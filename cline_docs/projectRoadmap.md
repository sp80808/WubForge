# WubForge Project Roadmap

## Overview
WubForge is a spectral bass processor VST plugin using JUCE framework. The project focuses on implementing innovative spectral processing techniques for bass sound manipulation.

## Core Goals
- [x] Implement basic spectral morphing engine
- [x] Create fractal filtering system with golden ratio scaling
- [x] Add key-tracked processing capabilities
- [x] Implement full GUI with real-time visual feedback
- [x] Add spectrograph visualization component
- [x] Integrate professional open source DSP libraries
- [x] Enhance processing capabilities with industrial-grade algorithms
- [ ] Complete modular processing chain
- [ ] Polish user interface and UX

## High-Level Features

### Core Processing Features
- [x] Spectral morphing using FFT-based real-time processing
- [x] Fractal filtering with self-similar harmonic structures
- [x] Key-tracking for all frequency-dependent parameters
- [x] Dual filter types (LPF and BPF variants)
- [x] **Professional EQ system** using chowdsp_utils (3-band parametric)
- [x] **Advanced distortion algorithms** from chowdsp_waveshapers
- [x] Modular 4-slot processing chain
- [ ] Advanced routing matrix (Serial/Parallel/Mid-Side/Feedback)

### Module Library
#### Filter Modules
- [x] Fractal Filter (LPF)
- [x] Bandpass Fractal Filter
- [x] **ChowEQModule** - Professional 3-band parametric EQ
- [ ] Classic Ladder Filter
- [ ] State Variable Filter (via chowdsp_utils)
- [ ] Formant Bank Filter
- [ ] Spectral Notches Filter
- [ ] Karplus-Strong Pluck Filter

#### Distortion Modules
- [x] DistortionForge (Wavefolder, Clipper, Bitcrusher)
- [x] FormantTracker
- [x] **chowdsp_waveshapers** - West Coast folder, Wave Multiplier
- [ ] Dynamic Alkane
- [ ] Ohmicide Swarm
- [ ] FM Distort

### User Interface Features
- [x] Real-time spectrograph visualization
- [ ] XY pad control for spectral morphing
- [ ] Modular chain visualization
- [ ] Routing matrix interface
- [ ] "Magic Forge" randomization button
- [ ] Preset management system

### Open Source Library Integrations
- [x] **chowdsp_utils** (304⭐️) - Professional DSP algorithms
- [x] **SpectrogramComponent** - Custom JUCE-based real-time visualization
- [ ] chowdsp_filters - Advanced filter collection (Butterworth, Chebyshev, Elliptic)
- [ ] JUMP - Professional metering and UI utilities

### Advanced Features
- [ ] Auto-harmonic enhancement
- [ ] Adaptive EQ for mix clarity
- [ ] Stereo widening
- [ ] MIDI-controlled parameter modulation

## Current Focus: UI Implementation
Looking for appropriate open source UI libraries to implement the planned spectrograph and visualization components.

## Completion Criteria
- All core spectral processing features functional
- Intuitive and responsive UI
- Real-time performance (<5ms latency)
- Professional sound quality
- Comprehensive module library

## Future Scalability Considerations
- Expand module library with community contributions
- Add cloud-based preset sharing
- Implement advanced morphing techniques
- Support for sidechain and external modulation
- Hardware acceleration for real-time processing
