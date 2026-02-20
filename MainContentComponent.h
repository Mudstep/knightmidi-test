#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "MidiPlayer.h"
#include "PSG.h"

class MainContentComponent : public juce::AudioAppComponent {
public:
    MainContentComponent();
    ~MainContentComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void resized() override;

private:
    // UI stuff
    juce::TextButton playButton { "Play MIDI Data" };
    juce::TextButton stopButton { "Stop Playback" };
    juce::TextButton loadButton { "Load MIDI File" };
    juce::Label midiPathLabel { {}, "No MIDI file loaded"};
    juce::ComboBox waveSelector;

    MidiPlayer midiPlayer;
    PSG psg;

    juce::CriticalSection midiLock;
};