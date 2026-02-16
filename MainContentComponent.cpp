#include "MainContentComponent.h"

MainContentComponent::MainContentComponent()
{
    // Add and control UI stuff
    addAndMakeVisible(playButton);
    playButton.onClick = [this] { playMidiData(); };
    
    addAndMakeVisible(stopButton);
    stopButton.onClick = [this] { stopPlayback(); };
    
    addAndMakeVisible(loadButton);
    loadButton.onClick = [this] { loadMidiFile(); };

    addAndMakeVisible(midiPathLabel);

    waveSelector.addItem("Square 12.5%", 1);
    waveSelector.addItem("Square 25%", 2);
    waveSelector.addItem("Square 50%", 3);
    waveSelector.addItem("Square 75%", 4);
    waveSelector.addItem("Sine", 5);
    waveSelector.setSelectedId(3); // default is a 50% square wave
    waveSelector.onChange = [this] {
        switch (waveSelector.getSelectedId()) {
            case 1: currentWaveType = square_12p5; break;
            case 2: currentWaveType = square_25; break;
            case 3: currentWaveType = square_50; break;
            case 4: currentWaveType = square_75; break;
            case 5: currentWaveType = sine; break;
        }
    };
    addAndMakeVisible(waveSelector);

    setSize(800, 600);
    setAudioChannels(0, 2);
}

MainContentComponent::~MainContentComponent()
{
    shutdownAudio();
}

void MainContentComponent::loadMidiFile()
{
    // open up file selection dialogue
    midiFileChooser = std::make_unique<juce::FileChooser>(
        "Select a MIDI file...",
        juce::File{},
        "*.mid;*.midi"
    );

    midiFileChooser->launchAsync(juce::FileBrowserComponent::openMode, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (!file.existsAsFile())
            return;

        juce::FileInputStream inputStream(file);
        if (!inputStream.openedOk())
            return;

        juce::ScopedLock sl(midiLock); // prevents playing back audio while MIDI data is being modified  (I think?)

        midiFile.readFrom(inputStream); // load the MIDI data into a MIDI file object
        midiFile.convertTimestampTicksToSeconds(); // convert ticks to seconds, although idk if this'll be a good idea in the long run and if we'll wanna deal directly with ticks

        for (int t = 0; t < midiFile.getNumTracks(); ++t) {
            midiTrack = *midiFile.getTrack(t);
            for (int i = 0; i < midiTrack.getNumEvents(); ++i) {
                if (midiTrack.getEventPointer(i)->message.isNoteOn()) {
                    midiPathLabel.setText(file.getFullPathName(), juce::dontSendNotification); // update path label
                    stopPlayback(); // resets instance variables and stops playback if anything was already playing
                    return;
                }
            }
        } 
    });
}

void MainContentComponent::playMidiData() {
    // if there's any actual midi data, reset instance variables and start playing it
    if (midiTrack.getNumEvents() > 0) {
        currentEventIndex = 0;
        samplesProcessed = 0.0;
        currentAngle = 0.0;
        noteOn = false;
        isPlaying = true;
    }
}

void MainContentComponent::stopPlayback() {
    // reset instance variables and stop playback
    currentEventIndex = 0;
    samplesProcessed = 0.0;
    currentAngle = 0.0;
    noteOn = false;
    isPlaying = false;
}

float MainContentComponent::getWave(waveTypes waveType, double angle) {
    // return float representation of a wave at a given angle
    switch (waveType) {
        case square_12p5:
            return std::fmod(angle, juce::MathConstants<double>::twoPi) > 1.75f * juce::MathConstants<double>::pi ? 1.0f : -1.0f; // 12.5% up, 87.5% down
        case square_25:
            return std::fmod(angle, juce::MathConstants<double>::twoPi) > 1.5f * juce::MathConstants<double>::pi ? 1.0f : -1.0f; // 25% up, 75% down
        default: // default to 50% square
        case square_50:
            return std::fmod(angle, juce::MathConstants<double>::twoPi) > juce::MathConstants<double>::pi ? 1.0f : -1.0f; // 50% up, 50% down
        case square_75:
            return std::fmod(angle, juce::MathConstants<double>::twoPi) > 0.5f * juce::MathConstants<double>::pi ? 1.0f : -1.0f; // 75% up, 25% down
        case sine:
            return std::sin(angle);
    }
}

void MainContentComponent::updateAngleDelta()
{
    auto cyclesPerSample = currentFrequency / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    updateAngleDelta();
}

void MainContentComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto level = 0.125f;
    auto* leftBuffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

    juce::ScopedLock sl(midiLock); // prevent modification of MIDI data while it's being read (I think?)
    
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
        if (isPlaying && currentSampleRate > 0) {
            double currentTime = samplesProcessed / currentSampleRate;

            while (currentEventIndex < midiTrack.getNumEvents()) {
                auto* event = midiTrack.getEventPointer(currentEventIndex);
                if (event->message.getTimeStamp() > currentTime)
                    break;

                auto& msg = event->message;

                // if current MIDI event is a note on event, store the frequency of the currently playing note
                if (msg.isNoteOn()) {
                    currentFrequency = juce::MidiMessage::getMidiNoteInHertz(msg.getNoteNumber()); // <- thank god for this function lmao I was gonna make a table with all the notes and frequencies
                    updateAngleDelta();
                    noteOn = true;
                }
                
                // if it's a note off, then turn off note on flag
                else if (msg.isNoteOff()) {
                    noteOn = false;
                }

                currentEventIndex++;
            }

            // if there's no events left, stop playing
            if (currentEventIndex >= midiTrack.getNumEvents()) {
                isPlaying = false;
            }

            samplesProcessed++;
        }

        float currentSample = 0.0f;

        if (noteOn) {
            // generate sample from current waveform and volume
            currentSample = getWave(currentWaveType, currentAngle) * level;
            currentAngle += angleDelta;

            // reset radian cycle if it grows too big
            if (currentAngle > juce::MathConstants<double>::twoPi) currentAngle -= juce::MathConstants<double>::twoPi;
        }

        // write sample to output buffer
        leftBuffer[sample] = currentSample;
        rightBuffer[sample] = currentSample;
    }
}

void MainContentComponent::releaseResources()
{
    juce::Logger::getCurrentLogger()->writeToLog ("Releasing audio resources");
}


void MainContentComponent::resized()
{
    playButton.setBounds(16, 40, 160, 40);
    stopButton.setBounds(playButton.getRight() + 16, 40, 160, 40);
    loadButton.setBounds(16, 80, 160, 40);
    midiPathLabel.setBounds(loadButton.getRight() + 16, 80, 320, 40);
    waveSelector.setBounds(16, 120, 160, 40);
}