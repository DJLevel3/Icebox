/*
  ==============================================================================

    SynthVoice.cpp
    Created: 7 Apr 2024 12:34:58pm
    Author:  DJ_Level_3

  ==============================================================================
*/

#include "SynthVoice.h"

bool SynthVoice::canPlaySound(SynthesiserSound* sound)
{
    return dynamic_cast<SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
    leftTable.clear();
    leftTable.addArray(leftRoll.getBufferOrdered());
    rightTable.clear();
    rightTable.addArray(rightRoll.getBufferOrdered());
    frequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    formant = formantBase;
    cycleLength = getSampleRate() * formant / frequency;
    position = leftTable.size() - cycleLength;
    adsr.noteOn();
    takingData = false;
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    takingData = true;
    adsr.noteOff();
}

void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{

}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{

}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels)
{
    adsr.setSampleRate(sampleRate);
    
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;

    isPrepared = true;
}

float SynthVoice::getSampleFromTable(bool chan, float pos) {
    int lower = std::floor(pos);
    int upper = (lower + 1) % leftTable.size();
    float sLower = chan ? rightTable[lower] : leftTable[lower];
    float sUpper = chan ? rightTable[upper] : leftTable[upper];
    float t = pos - lower;
    return sLower + t * (sUpper - sLower);
}

void SynthVoice::formantChanged(float newFormant)
{
    float oldBase = formantBase;
    formantBase = std::pow(2, (newFormant / 12));
    formant = formant * formantBase / oldBase;
    formantTarget = formantTarget * formantBase / oldBase;
    cycleLength = getSampleRate() * formant / frequency;
}

void SynthVoice::formantEnvelopeChanged(float depth, float newRate, bool exponential) {
    float tempTarget = formantBase * std::pow(2, (depth / 12));
    if (tempTarget / frequency * sampleRate > MidiMessage::getMidiNoteInHertz(0)) formantTarget = tempTarget;
    formantRate = 1 - (0.0001 * newRate);
}

float SynthVoice::expDecay(float now, float targ, float rate)
{
    return targ + (now - targ) * rate;
}

void SynthVoice::renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    jassert(isPrepared);
    dsp::AudioBlock<float> audioBlock{ outputBuffer };
    if (!takingData) {
        for (int samp = startSample; samp < startSample + numSamples; samp++) {
            audioBlock.setSample(0, samp, getSampleFromTable(false, position));
            audioBlock.setSample(1, samp, getSampleFromTable(true, position));
            formant = expDecay(formant, formantTarget, formantRate);
            cycleLength = getSampleRate() * formant / frequency;
            position += formant;
            if (position > leftTable.size()) {
                position -= cycleLength;
            }
        }
    }
    //adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
}