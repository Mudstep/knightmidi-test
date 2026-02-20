#include "PSG.h"

void PSG::updateAngleDelta() {
    auto cyclesPerSample = currentFrequency / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

float PSG::getWave(waveTypes waveType, double angle) {
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
