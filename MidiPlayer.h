#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

class MidiPlayer {
public:
    void loadMidiFile(std::function<void(const juce::String&)> onLoaded = nullptr);
    void playMidiData();
    void stopPlayback();
    void updateSamplesPerTick();

    juce::MidiFile midiFile;
    juce::String midiFilePath;
    juce::MidiMessageSequence midiTrack;
    int currentEventIndex = 0;
    double ticksProcessed = 0.0;
    double samplesPerTick = 0.0;
    int timeFormat = 0;
    double currentTempo = 500000.0; // microseconds per beat, default to 120 BPM
    bool isPlaying = false;
    bool noteOn = false;
    double currentSampleRate = 44100.0; // default to 44.1khz, but JUCE replaces it with whatever the system's audio device uses

private:
    std::unique_ptr<juce::FileChooser> midiFileChooser;
};