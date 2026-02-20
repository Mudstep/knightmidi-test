#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

class PSG {
public:
    enum waveTypes {
        square_12p5,
        square_25,
        square_50,
        square_75,
        sine
    };

    void updateAngleDelta();
    float getWave(waveTypes waveType, double angle);

    double currentSampleRate = 44100.0; // default to 44.1khz, but JUCE replaces it with whatever the system's audio device uses
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double currentFrequency = 440.0;
    waveTypes currentWaveType = square_50;
};