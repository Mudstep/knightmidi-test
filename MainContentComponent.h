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

    double currentSampleRate = 0.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double currentFrequency = 440.0;
    double targetFrequency = 440.0;
    waveTypes currentWaveType = square_50;

    // MIDI stuff
    std::unique_ptr<juce::FileChooser> midiFileChooser;
    juce::MidiFile midiFile;
    juce::MidiMessageSequence midiTrack;
    int currentEventIndex = 0;
    double samplesProcessed = 0.0;
    bool isPlaying = false;
    bool noteOn = false;

    juce::CriticalSection midiLock;
};