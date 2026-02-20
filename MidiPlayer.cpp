#include "MidiPlayer.h"

void MidiPlayer::loadMidiFile(std::function<void(const juce::String&)> onLoaded)
{
    // open up file selection dialogue
    midiFileChooser = std::make_unique<juce::FileChooser>(
        "Select a MIDI file...",
        juce::File{},
        "*.mid;*.midi"
    );

    midiFileChooser->launchAsync(juce::FileBrowserComponent::openMode, [this, onLoaded](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (!file.existsAsFile())
            return;
        
        juce::FileInputStream inputStream(file);
        if (!inputStream.openedOk())
            return;

        midiFile.readFrom(inputStream); // load the MIDI data into a MIDI file object

        timeFormat = midiFile.getTimeFormat();

        // check for initial tempo value. this entire block is temporary until proper tempo changing is implemented
        for (int t = 0; t < midiFile.getNumTracks(); ++t) {
            auto const* track = midiFile.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); ++i) {
                auto& msg = track->getEventPointer(i)->message;
                if (msg.isTempoMetaEvent()) {
                    currentTempo = msg.getTempoSecondsPerQuarterNote() * 1000000.0;
                    break;
                }
            }
        }

        // check for first track with note data
        bool fileHasNoteData = false;
        for (int t = 0; t < midiFile.getNumTracks() && !fileHasNoteData; ++t) {
            auto const* track = midiFile.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); ++i) {
                auto& msg = track->getEventPointer(i)->message;
                // check if current indexed track has note data, if it does, it's the first track with note data and the only one that'll be played
                if (msg.isNoteOn()) {
                    midiTrack = *track;
                    fileHasNoteData = true;
                    break;
                }
            }
        }

        if (!fileHasNoteData)
            return;

        stopPlayback();
        midiFilePath = file.getFullPathName();
        updateSamplesPerTick();
        if (onLoaded) onLoaded(midiFilePath);
    });
}

void MidiPlayer::playMidiData() {
    // if there's any actual midi data, reset instance variables and start playing it
    if (midiTrack.getNumEvents() > 0) {
        currentEventIndex = 0;
        ticksProcessed = 0.0;
        noteOn = false;
        isPlaying = true;
    }
}

void MidiPlayer::stopPlayback() {
    // reset instance variables and stop playback
    currentEventIndex = 0;
    ticksProcessed = 0.0;
    noteOn = false;
    isPlaying = false;
}

void MidiPlayer::updateSamplesPerTick() {
    samplesPerTick = (currentTempo / 1000000.0 / timeFormat) * currentSampleRate;
}