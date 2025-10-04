# Current Task: Implement Fibonacci Spiral Distort (FSD) Algorithm

## Objective
Implement the novel **Fibonacci Spiral Distort (FSD)** algorithm as a new distortion module for WubForge, creating self-similar, consonant harmonics using golden ratio (φ ≈ 1.618) spacing and Fibonacci approximations for "pleasing" richness without dissonance.

## Context
The FSD algorithm combines φ-spaced resonator banks with Fibonacci-ratio modulated distortion stages, creating "spiraling growls" that transform sterile sines into evolving, resonant monsters. This fits perfectly with WubForge's dubstep focus and modular architecture.

## Current Status
✅ **Enhanced FSD Implementation Complete**
- Created FibonacciSpiralDistort class with golden ratio algorithms
- Implemented φ-resonator bank, Fibonacci distortion cascade, and spiral veil filter
- Added real-time performance optimizations for <3% CPU usage

✅ **Advanced Features Implemented**
- **Key Dependence**: Full MIDI scaling for octave-consistent timbre (A4=69=440Hz base)
- **Psychoacoustic Leveling**: A-weighted RMS normalization to -12dBFS perceived loudness
- **Ear-Friendly Processing**: Bark-band spacing, A-weighting, 200-5kHz sweet spot emphasis
- **Self-Oscillating Resonators**: Feedback loops for evolving, living harmonics
- **Morphing Control**: 0.0-1.0 morph between simple WT and full spiral complexity

✅ **Plugin Integration Complete**
- Added FSD module to PluginProcessor factory system
- Module appears in GUI as "Fibonacci Spiral Distort" option
- Integrated with existing 5-slot modular architecture

✅ **Documentation Setup Complete**
- Created cline_docs structure with project roadmap
- Established comprehensive task tracking for FSD implementation

## Implementation Plan

### **Phase 1: Core FSD Module** (Current)
1. **FibonacciSpiralDistort Class**: Create new distortion module inheriting from DistortionModule
2. **φ-Resonator Bank**: Implement golden-spaced sine resonators (3-7 harmonics)
3. **Fibonacci Distortion Cascade**: 4-stage waveshaper with Fib-ratio drive modulation
4. **Spiral Veil Filter**: Cascaded LPs with φ-cutoff spacing for cohesion
5. **Parameter Integration**: Add to existing value tree system

### **Phase 2: Advanced Features**
1. **Envelope Following**: Transient-triggered modulation for dynamic blooming
2. **Key Tracking**: MIDI-responsive frequency scaling
3. **Fibonacci Depth Control**: Variable n=5-15 for coarse-to-fine ratios
4. **φ-Toggle Mode**: Switch between true golden ratio and Fib approximations

### **Phase 3: Integration & Testing**
1. **Module Factory**: Add to module creation system
2. **Performance Optimization**: Ensure <3% CPU usage target
3. **Preset Integration**: Create "Spiral Growl" and "Fib Bass" presets
4. **Documentation**: Update module README and algorithm docs

## Technical Specifications

### Algorithm Architecture
```
Input Signal → [φ-Resonator Bank] → [Fib Distort Cascade] → [Spiral Veil] → Output
                     ↓                         ↓                    ↓
              Golden-spaced              Fib-ratio drives    φ-cutoff LPs
              harmonics (3-7)           (1.6→1.618)         (500→1300Hz)
```

### Key Parameters
- **Spiral Depth** (0.0-1.0): φ-resonator bank mix amount
- **Fib Drive** (0.0-2.0): Base distortion intensity with Fib scaling
- **Bloom Rate** (0.001-2.0s): Envelope attack/release for harmonic blooming
- **Veil Cutoff** (200-5000Hz): Base frequency for spiral filter cascade
- **Resonance** (0.0-0.8): φ-resonator feedback amount

### Performance Targets
- **CPU Usage**: ~3% with 4-stage cascade at 44.1kHz
- **Latency**: Zero additional latency (sample-accurate)
- **Memory**: ~1MB for resonator states and filter coefficients
- **Threading**: Audio thread safe with GUI parameter updates

## Testing Criteria

### **Sound Validation**
- **Input**: 55Hz sine wave (A1 bass fundamental)
- **Expected Output**: Rich harmonics at ~165/267/432/699Hz with progressive blooming
- **Measurement**: +12dB mid-frequency enhancement with consonant ratios

### **Performance Validation**
- **Target**: <15% total CPU with full 5-slot chain including FSD
- **Method**: Profile with typical dubstep project (120 BPM, 4/4)
- **Baseline**: Current CPU usage without FSD module

## Next Steps After Implementation
1. **Gun Bass FM Engine**: Implement tearout/riddim FM synthesis
2. **Tearout Rhythm System**: Add LFO bank and phaser for dubstep rhythms
3. **Dubstep Forge Mode**: Create one-click preset system

## Results Expected
The FSD module will provide WubForge users with a unique distortion algorithm that creates organic, evolving bass textures using mathematical beauty (golden ratio) for professional dubstep production. The self-similar harmonics and Fibonacci envelopes will offer new creative possibilities not available in existing distortion plugins.
