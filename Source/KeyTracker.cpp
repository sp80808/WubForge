#include "KeyTracker.h"

//==============================================================================
KeyTracker::KeyTracker()
{
}

KeyTracker::~KeyTracker()
{
}

//==============================================================================
void KeyTracker::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    reset();
}

void KeyTracker::reset()
{
    currentFrequency = 440.0f;  // Reset to A4
    lastNoteNumber = -1;
    sustainPedalPressed = false;
    activeNotes.clear();
}

//==============================================================================
void KeyTracker::processMidi (const juce::MidiBuffer& midiMessages, int numSamples)
{
    // Process MIDI messages to track note on/off events
    for (const auto& metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            int noteNumber = message.getNoteNumber();

            // Add to active notes for polyphonic tracking
            activeNotes.insert(noteNumber);
            lastNoteNumber = noteNumber;

            // Update frequency based on current tracking mode
            updateFrequencyFromActiveNotes();
        }
        else if (message.isNoteOff())
        {
            int noteNumber = message.getNoteNumber();

            // Remove from active notes
            activeNotes.erase(noteNumber);

            // If sustain pedal is not pressed, update frequency
            if (!sustainPedalPressed)
            {
                updateFrequencyFromActiveNotes();
            }
        }
        else if (message.isSustainPedalOn())
        {
            sustainPedalPressed = true;
        }
        else if (message.isSustainPedalOff())
        {
            sustainPedalPressed = false;
            // When sustain pedal is released, remove all notes and update frequency
            activeNotes.clear();
            updateFrequencyFromActiveNotes();
        }
        else if (message.isPitchWheel())
        {
            // Handle pitch bend - apply to current frequency
            float pitchBendSemitones = (message.getPitchWheelValue() - 8192) / 8192.0f * 2.0f;
            float pitchBendRatio = std::pow (2.0f, pitchBendSemitones / 12.0f);

            // Get base frequency from current tracking mode
            float baseFreq = midiNoteToFrequency(lastNoteNumber > 0 ? lastNoteNumber : 69);
            currentFrequency = baseFreq * pitchBendRatio * keyTrackAmount + baseFreq * (1.0f - keyTrackAmount);
        }
    }
}

//==============================================================================
void KeyTracker::updateFrequencyFromActiveNotes()
{
    if (activeNotes.empty())
    {
        // No active notes, keep current frequency or reset to default
        currentFrequency = 440.0f;
        lastNoteNumber = -1;
        return;
    }

    float newFrequency = 0.0f;
    int referenceNote = 69;  // A4

    switch (keyTrackMode)
    {
        case KeyTrackMode::LatestNote:
            // Use the most recent note (lastNoteNumber should be set)
            if (lastNoteNumber > 0)
            {
                newFrequency = midiNoteToFrequencyLogarithmic(lastNoteNumber);
            }
            break;

        case KeyTrackMode::HighestNote:
            // Use the highest active note
            newFrequency = midiNoteToFrequencyLogarithmic(*activeNotes.rbegin());
            lastNoteNumber = *activeNotes.rbegin();
            break;

        case KeyTrackMode::LowestNote:
            // Use the lowest active note
            newFrequency = midiNoteToFrequencyLogarithmic(*activeNotes.begin());
            lastNoteNumber = *activeNotes.begin();
            break;

        case KeyTrackMode::AverageNote:
            // Calculate average of all active notes
            float sum = 0.0f;
            for (int note : activeNotes)
            {
                sum += static_cast<float>(note);
            }
            float averageNote = sum / activeNotes.size();
            newFrequency = midiNoteToFrequencyLogarithmic(static_cast<int>(std::round(averageNote)));
            lastNoteNumber = static_cast<int>(std::round(averageNote));
            break;
    }

    // Apply key track amount for blending between tracked and fixed frequency
    float baseFreq = midiNoteToFrequency(69);  // A4 as base
    currentFrequency = newFrequency * keyTrackAmount + baseFreq * (1.0f - keyTrackAmount);
}

//==============================================================================
float KeyTracker::midiNoteToFrequency (int midiNote)
{
    // Standard MIDI note to frequency conversion (A4 = 440 Hz, MIDI note 69)
    return 440.0f * std::pow (2.0f, (midiNote - 69) / 12.0f);
}

float KeyTracker::midiNoteToFrequencyLogarithmic (int midiNote, float baseFreq, int baseMidiNote)
{
    // Logarithmic frequency conversion for more natural pitch scaling
    return baseFreq * std::pow (2.0f, (midiNote - baseMidiNote) / 12.0f);
}
