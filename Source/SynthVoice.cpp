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
    frequencyTarget = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    if (frequencyInit) {
        frequency = frequencyTarget;
        frequencyInit = false;
    }
    portamentoBase = frequency;
    formant = formantBase;
    cycleLength = getSampleRate() * formant / frequency;
    position = leftTable.size() - cycleLength;
    adsr.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff) adsr.noteOff();
    else adsr.reset();
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
    adsr.setParameters(adsrParams);
    adsr.reset();
    
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

void SynthVoice::formantEnvelopeChanged(float depth, float newRate, bool linear) {
    float tempTarget = formantBase * std::pow(2, (depth / 12));
    if (tempTarget / frequency * sampleRate > MidiMessage::getMidiNoteInHertz(0)) formantTarget = tempTarget;
    else formantTarget = MidiMessage::getMidiNoteInHertz(0);
    formantRate = 1 - (0.0001 * newRate);
    exp = !linear;
}

void SynthVoice::adsrChanged(float a, float d, float s, float r) {
    adsrParams = ADSR::Parameters(a, d, s, r);
    adsr.setParameters(adsrParams);
}

void SynthVoice::portamentoChanged(float p) {
    usePortamento = (p < 1);
    portamento = 1 - (0.001 * p);
}

float SynthVoice::expDecay(float now, float targ, float rate, float sRate)
{
    return targ + ((now - targ) * rate);
}

float SynthVoice::linDecay(float base, float now, float targ, float rate, float sRate)
{
    float delta = (base - targ) * (1 - rate) * 19200 / sRate;
    if (delta >= 0) { // Decreasing
        return (now - delta < targ) ? targ : (now - delta);
    }
    else { // Increasing
        return (now - delta > targ) ? targ : (now - delta);
    }
}

void SynthVoice::renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    jassert(isPrepared);
    dsp::AudioBlock<float> audioBlock{ outputBuffer };
    for (int samp = startSample; samp < startSample + numSamples; samp++) {
        audioBlock.setSample(0, samp, getSampleFromTable(false, position));
        audioBlock.setSample(1, samp, getSampleFromTable(true, position));

        if (usePortamento) frequency = linDecay(portamentoBase, frequency, frequencyTarget, portamento, getSampleRate());
        else frequency = frequencyTarget;

        if (exp) formant = expDecay(formant, formantTarget, formantRate, getSampleRate());
        else formant = linDecay(formantBase, formant, formantTarget, formantRate, getSampleRate());

        cycleLength = getSampleRate() * formant / frequency;
        position += formant;
        if (position > leftTable.size()) {
            position -= cycleLength;
        }
    }
    adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
}