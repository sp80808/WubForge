# WubForge UI/UX Design Document

## 1. Design Philosophy

*   **Immediate Visual Feedback**: The user should *see* what they are hearing. The core of the UI will be a real-time visualization that shows exactly how the audio is being transformed at every stage.
*   **Playful but Powerful**: The interface should be engaging and fun to use, encouraging experimentation. Complex actions like modulation and routing should be achievable with simple, direct interactions.
*   **One-Screen Workflow**: All core functions—module chaining, parameter editing, and modulation—should be accessible from a single, uncluttered screen without needing to switch tabs or open new windows.

## 2. Proposed Layout: The Three Sections

The UI will be divided into three main horizontal sections:

### **Section A: The Stage (Chain & Visualization)**

This is the main, top-most section and the heart of the plugin.

*   **Background**: The entire background of this section is a real-time **spectrograph**, constantly displaying the frequency content of the audio *at the output of the chain*.
*   **Module Chain**: The 5 module slots are represented as a series of connected nodes overlaid on the spectrograph. This allows the user to see the signal flow and the effect it has in real-time.
*   **Interaction**:
    *   **Select**: Clicking a module node selects it, showing its parameters in "The Controls" section below. The selected node will glow.
    *   **Re-order**: Dragging and dropping a module node allows you to re-order the chain instantly.
    *   **Add/Remove**: Clicking a `+` icon on an empty slot opens a simple browser to select a new module. A small `x` appears on modules when hovered, allowing them to be removed.
*   **Routing Control**: A set of four simple icons (`Serial`, `Parallel`, `Mid/Side`, `Feedback`) is located here. Clicking one instantly changes the visual path of the chain.
    *   *Parallel/Mid-Side*: The chain visually splits into two lanes.
    *   *Feedback*: A glowing animated line loops from the output back to the input.

### **Section B: The Controls (Dynamic Panel)**

This middle section is dedicated to parameter editing.

*   **Content**: This area is populated with the controls for the module currently selected in "The Stage".
*   **Model Selection**: If the selected module is a `Universal` type (like our distortion), a "Model" dropdown will appear at the top of this section, which then determines which specific controls are shown below.
*   **Controls**: All controls will be large, clear knobs and sliders.

### **Section C: The Modulators & Magic**

This bottom section is for adding movement and experimentation.

*   **Modulation Sources**: A small, expandable panel contains the available modulators (e.g., LFO 1, LFO 2, Envelope 1).
*   **Drag-and-Drop Modulation**: To assign modulation, the user simply drags a modulator's icon (e.g., "LFO 1") and drops it onto any knob in "The Controls" section.
*   **Modulation Visualization**: When a knob is modulated, a colored ring appears around it, animating to show the effect of the modulation, exactly like in FabFilter plugins.
*   **Magic Forge Button**: A large, prominent, glowing button will be located here. Pressing it will trigger the smart randomization function we've already built.

## 3. Implementation Path

1.  **Standardize on Foleys**: To build this, we must commit to using the `foleys::MagicProcessor` framework.
2.  **Build the Static Layout**: Create the three main sections and the global controls (Routing, Magic Forge).
3.  **Implement the Module Chain**: Create the custom JUCE components for the module nodes and implement the selection logic.
4.  **Implement the Dynamic Control Panel**: Connect the UI to the `Universal` modules' "Model" parameter to show/hide controls.
5.  **Implement Drag-and-Drop**: Add drag-and-drop functionality for both the module chain and the modulation system.
6.  **Implement Visualization**: Integrate the FFT-based spectrograph as the background of "The Stage".
