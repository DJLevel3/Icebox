/*
  ==============================================================================

    SynthVoice.h
    Created: 7 Apr 2024 12:34:58pm
    Author:  DJ_Level_3

  ==============================================================================
*/

#pragma once
#include "SynthSound.h"
#include "FixedDelayBuffer.h"

class SynthVoice : public SynthesiserVoice
{
public:
    static float expDecay(float now, float targ, float rate, float sRate = 96000);
    static float linDecay(float base, float now, float targ, float rate, float sRate = 96000);
    bool canPlaySound(SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    void formantChanged(float newFormant);
    void formantEnvelopeChanged(float depth, float newWidth, bool linear = false);
    float getSampleFromTable(bool chan, float pos);
    bool takingData = true;
    FixedDelayBuffer<float> leftRoll;
    FixedDelayBuffer<float> rightRoll;
    SynthVoice (int sr) {
    }
private:
    Array<float> leftTable;
    Array<float> rightTable;
    float formant = 1;
    float formantBase = 1;
    float formantTarget = 1;
    float formantRate = 0.9999;
    float formantRateLinear = 0.001;
    float frequency = 440;

    float sampleRate = 96000;

    float position = 0;

    bool exp = true;

    float cycleLength = 96000 / 440;
    ADSR adsr;
    ADSR::Parameters adsrParams;
    bool isPrepared{ false };
};