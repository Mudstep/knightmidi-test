#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

class MainContentComponent : public juce::AudioAppComponent {
public:
    MainContentComponent();
    ~MainContentComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void resized() override;

private:
    // Engine stuff
    void updateAngleDelta();
    void updateSamplesPerTick();
    void loadMidiFile();
    void playMidiData();
    void stopPlayback();

    // UI stuff
    juce::TextButton playButton { "Play MIDI Data" };
    juce::TextButton stopButton { "Stop Playback" };
    juce::TextButton loadButton { "Load MIDI File" };
    juce::Label midiPathLabel { {}, "No MIDI file loaded"};
    juce::ComboBox waveSelector;

    // Synth stuff
    enum waveTypes {
        square_12p5,
        square_25,
        square_50,
        square_75,
        sine
    };

    float getWave(waveTypes waveType, double angle);

    double currentSampleRate = 44100.0; // default to 44.1khz, but JUCE replaces it with whatever the system's audio device uses
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double currentFrequency = 440.0;
    waveTypes currentWaveType = square_50;

    // MIDI stuff
    std::unique_ptr<juce::FileChooser> midiFileChooser;
    juce::MidiFile midiFile;
    juce::MidiMessageSequence midiTrack;
    int currentEventIndex = 0;
    double ticksProcessed = 0.0;
    double samplesPerTick = 0.0;
    int timeFormat = 0;
    double currentTempo = 500000.0; // microseconds per beat, default to 120 BPM
    bool isPlaying = false;
    bool noteOn = false;

    juce::CriticalSection midiLock;
};