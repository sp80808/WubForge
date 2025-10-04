#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <set>

//==============================================================================
class KeyTracker
{
public:
    KeyTracker();
    ~KeyTracker();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    void processMidi (const juce::MidiBuffer& midiMessages, int numSamples);

    //==============================================================================
    float getCurrentFrequency() const { return currentFrequency; }
    float getKeyTrackAmount() const { return keyTrackAmount; }
    void setKeyTrackAmount (float amount) { keyTrackAmount = amount; }

    //==============================================================================
    // Key tracking modes for different use cases
    enum class KeyTrackMode
    {
        LatestNote,     // Track the most recent note (monophonic)
        HighestNote,    // Track the highest note (polyphonic)
        LowestNote,     // Track the lowest note (polyphonic)
        AverageNote     // Track the average of all notes (polyphonic)
    };

    void setKeyTrackMode (KeyTrackMode mode) { keyTrackMode = mode; }

    //==============================================================================
    // Utility functions for MIDI note to frequency conversion
    static float midiNoteToFrequency (int midiNote);
    static float midiNoteToFrequencyLogarithmic (int midiNote, float baseFreq = 440.0f, int baseMidiNote = 69);

private:
    //==============================================================================
    float currentFrequency = 440.0f;  // Default to A4
    float keyTrackAmount = 1.0f;
    double sampleRate = 44100.0;
    KeyTrackMode keyTrackMode = KeyTrackMode::LatestNote;

    // MIDI state tracking for polyphony
    std::set<int> activeNotes;  // Track all currently active notes
    int lastNoteNumber = -1;
    bool sustainPedalPressed = false;

    //==============================================================================
    void updateFrequencyFromActiveNotes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyTracker)
};
