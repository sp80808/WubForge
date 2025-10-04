# WubForge Codebase Summary

## Project Overview
WubForge is a JUCE-based audio plugin focused on dubstep bass synthesis with novel DSP algorithms. The project emphasizes modular architecture, professional sound quality, and innovative processing techniques.

## Key Components and Their Interactions

### **Core Architecture**
- **PluginProcessor**: Main audio processing hub managing 5-slot module chain
- **PluginEditor**: GUI interface with modular slot components
- **Module System**: Factory pattern for interchangeable DSP modules

### **Audio Modules**
- **SampleMorpher**: Granular synthesis for sample-to-bass transformation
- **FractalFilter**: Self-similar filtering algorithms
- **UniversalFilterModule**: Professional filtering with ChowDSP integration
- **UniversalDistortionModule**: Waveshaping and distortion processing
- **MDASubSynthModuleDirect**: Classic sub bass enhancement

### **Supporting Components**
- **KeyTracker**: MIDI frequency detection and tracking
- **FormantTracker**: Vocal formant analysis for morphing
- **SpectrogramComponent**: Real-time spectral visualization
- **Presets**: Factory and management system for plugin states

## Data Flow
```
MIDI Input → KeyTracker → Module Chain → Output
     ↓              ↓            ↓         ↓
Audio Input → FormantTracker → SampleMorpher → Filters → Distortion
```

## External Dependencies
- **ChowDSP Utils**: Professional EQ, filters, and dynamics processing
- **JUCE**: Core framework for audio plugin functionality
- **MDA Plugins**: Classic effects for sub bass enhancement

## Recent Significant Changes
- **SampleMorpher Integration**: Added granular synthesis module for Serum 2-style processing
- **Modular Architecture**: Refactored to support 5-slot processing chain
- **Performance Optimization**: CPU usage targeting <15% with full module chain

## User Feedback Integration and Its Impact on Development
- **Dubstep Focus**: Shifted from general synthesis to specialized dubstep bass production
- **Sample Morphing**: User requests for Serum-style processing implemented via SampleMorpher
- **Performance Priority**: Maintained low CPU usage despite advanced algorithms

## Current Development Focus
- **Fibonacci Spiral Distort**: Novel golden ratio-based distortion algorithm
- **Gun Bass FM Engine**: Tearout/riddim FM synthesis for aggressive bass sounds
- **Tearout Rhythm System**: LFO and modulation system for dubstep rhythms

## Code Quality Metrics
- **Modularity**: High - clean separation between processing modules
- **Performance**: Optimized for real-time audio with <15% CPU target
- **Maintainability**: Well-documented with clear interfaces between components
- **Extensibility**: Factory pattern allows easy addition of new module types

## Architecture Strengths
- **Scalable Module System**: Easy to add new processing algorithms
- **Professional DSP Integration**: ChowDSP provides industry-standard components
- **Real-time Performance**: Lock-free parameter updates and efficient processing
- **Cross-platform Compatibility**: JUCE framework ensures broad plugin format support
