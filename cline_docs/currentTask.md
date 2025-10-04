# Current Task: Implement Gun Bass FM Engine

## Objective
Implement the **Gun Bass FM Engine** as a new synthesis module for WubForge, focusing on tearout/riddim FM synthesis with 8-voice unison for aggressive bass sounds.

## Context
The Gun Bass FM Engine will provide WubForge users with a powerful tool for creating modern, aggressive dubstep basslines. It will leverage FM synthesis principles combined with unison and modulation capabilities to deliver the characteristic "tearout" and "riddim" sounds popular in contemporary dubstep. This fits perfectly with WubForge's dubstep focus and modular architecture.

## Current Status
- **Initial Research**: Exploring various FM synthesis architectures and unison implementation strategies.
- **Core FM Engine Design**: Outlining the fundamental components of the FM engine (oscillators, envelopes, LFOs).

## Implementation Plan

### **Phase 1: Core FM Engine**
1.  **GunBassFmEngine Class**: Create new synthesis module inheriting from AudioModule.
2.  **FM Oscillators**: Implement multiple operators with adjustable ratios and modulation indices.
3.  **Envelopes**: Design ADSR envelopes for amplitude and modulation.
4.  **LFOs**: Integrate flexible LFOs for dynamic parameter modulation.
5.  **Parameter Integration**: Add to existing value tree system.

### **Phase 2: Unison and Voicing**
1.  **8-Voice Unison**: Implement detuning and spread for thick, wide bass sounds.
2.  **Voice Management**: Efficient voice allocation and stealing.

### **Phase 3: Integration & Testing**
1.  **Module Factory**: Add to module creation system.
2.  **Performance Optimization**: Ensure efficient CPU usage.
3.  **Preset Integration**: Create aggressive "Gun Bass" and "Tearout" presets.
4.  **Documentation**: Update module README and algorithm docs.

## Technical Specifications

### Algorithm Architecture
```
MIDI Input → GunBassFmEngine → Output
                 ↓
            [FM Oscillators]
                 ↓
            [Envelopes]
                 ↓
            [LFOs]
                 ↓
            [Unison Engine]
```

### Key Parameters
-   **Operator Ratios**: Control the harmonic content of the FM synthesis.
-   **Modulation Index**: Adjust the intensity of FM modulation.
-   **Envelope Controls**: Shape the amplitude and modulation over time.
-   **LFO Rate/Depth**: Control the speed and intensity of modulation.
-   **Unison Detune/Spread**: Adjust the thickness and width of the unison voices.

### Performance Targets
-   **CPU Usage**: Aim for efficient CPU usage, considering 8-voice unison.
-   **Latency**: Zero additional latency (sample-accurate).
-   **Memory**: Optimized memory footprint for multiple voices.
-   **Threading**: Audio thread safe with GUI parameter updates.

## Testing Criteria

### **Sound Validation**
-   **Input**: MIDI notes.
-   **Expected Output**: Aggressive, tearing FM bass sounds with rich harmonic content and wide unison.
-   **Measurement**: Spectral analysis to confirm FM harmonics and unison spread.

### **Performance Validation**
-   **Target**: <15% total CPU with full 5-slot chain including Gun Bass FM Engine.
-   **Method**: Profile with typical dubstep project (120 BPM, 4/4).
-   **Baseline**: Current CPU usage without Gun Bass FM Engine module.

## Next Steps After Implementation
1.  **Tearout Rhythm System**: Add LFO bank and phaser for dubstep rhythms.
2.  **Enhanced Filtering**: Gaussian Bin-Shave LP with spectral smoothing.
3.  **Dubstep Forge Mode**: Create one-click preset system.

## Results Expected
The Gun Bass FM Engine will provide WubForge users with a dedicated and powerful FM synthesis module for creating cutting-edge dubstep bass sounds, further solidifying WubForge's position as a specialized tool for modern bass music production.