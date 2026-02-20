#include "MainContentComponent.h"

MainContentComponent::MainContentComponent()
{
    // Add and control UI stuff
    addAndMakeVisible(playButton);
    playButton.onClick = [this] {
        psg.currentAngle = 0.0;
        midiPlayer.playMidiData();
    };
    
    addAndMakeVisible(stopButton);
    stopButton.onClick = [this] {
        midiPlayer.stopPlayback();
        psg.currentAngle = 0.0;
    };
    
    addAndMakeVisible(loadButton);
    loadButton.onClick = [this] {
        midiPlayer.loadMidiFile([this](const juce::String& path) {
            midiPathLabel.setText(path, juce::dontSendNotification);
        });
    };

    addAndMakeVisible(midiPathLabel);

    waveSelector.addItem("Square 12.5%", 1);
    waveSelector.addItem("Square 25%", 2);
    waveSelector.addItem("Square 50%", 3);
    waveSelector.addItem("Square 75%", 4);
    waveSelector.addItem("Sine", 5);
    waveSelector.setSelectedId(3); // default is a 50% square wave
    waveSelector.onChange = [this] {
        switch (waveSelector.getSelectedId()) {
            case 1: psg.currentWaveType = PSG::square_12p5; break;
            case 2: psg.currentWaveType = PSG::square_25; break;
            case 3: psg.currentWaveType = PSG::square_50; break;
            case 4: psg.currentWaveType = PSG::square_75; break;
            case 5: psg.currentWaveType = PSG::sine; break;
        }
    };
    addAndMakeVisible(waveSelector);

    setSize(800, 600);
    setAudioChannels(0, 2);
}

MainContentComponent::~MainContentComponent() {
    shutdownAudio();
}

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    // set sample rate to the sample rate of the system's audio device
    psg.currentSampleRate = sampleRate;
    psg.updateAngleDelta();

    midiPlayer.currentSampleRate = sampleRate;
    midiPlayer.updateSamplesPerTick();
}

void MainContentComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    auto level = 0.125f;
    auto* leftBuffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

    juce::ScopedLock sl(midiLock); // prevent modification of MIDI data while it's being read (I think?)
    
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
        if (midiPlayer.isPlaying && psg.currentSampleRate > 0) {
            midiPlayer.ticksProcessed += 1.0 / midiPlayer.samplesPerTick;

            while (midiPlayer.currentEventIndex < midiPlayer.midiTrack.getNumEvents()) {
                auto* event = midiPlayer.midiTrack.getEventPointer(midiPlayer.currentEventIndex);
                if (event->message.getTimeStamp() > midiPlayer.ticksProcessed)
                    break;

                auto& msg = event->message;

                // if current MIDI event is a note on event, store the frequency of the currently playing note
                if (msg.isNoteOn()) {
                    psg.currentFrequency = juce::MidiMessage::getMidiNoteInHertz(msg.getNoteNumber()); // <- thank god for this function lmao I was gonna make a table with all the notes and frequencies
                    psg.updateAngleDelta();
                    midiPlayer.noteOn = true;
                }
                
                // if it's a note off, then turn off note on flag
                else if (msg.isNoteOff()) {
                    midiPlayer.noteOn = false;
                }

                midiPlayer.currentEventIndex++;
            }

            // if there's no events left, stop playing
            if (midiPlayer.currentEventIndex >= midiPlayer.midiTrack.getNumEvents()) {
                midiPlayer.isPlaying = false;
            }
        }

        float currentSample = 0.0f;

        if (midiPlayer.noteOn) {
            // generate sample from current waveform and volume
            currentSample = psg.getWave(psg.currentWaveType, psg.currentAngle) * level;
            psg.currentAngle += psg.angleDelta;

            // reset radian cycle if it grows too big
            if (psg.currentAngle > juce::MathConstants<double>::twoPi) psg.currentAngle -= juce::MathConstants<double>::twoPi;
        }

        // write sample to output buffer
        leftBuffer[sample] = currentSample;
        rightBuffer[sample] = currentSample;
    }
}

void MainContentComponent::releaseResources() {
    juce::Logger::getCurrentLogger()->writeToLog ("Releasing audio resources");
}

void MainContentComponent::resized() {
    playButton.setBounds(16, 40, 160, 40);
    stopButton.setBounds(playButton.getRight() + 16, 40, 160, 40);
    loadButton.setBounds(16, 80, 160, 40);
    midiPathLabel.setBounds(loadButton.getRight() + 16, 80, 320, 40);
    waveSelector.setBounds(16, 120, 160, 40);
}