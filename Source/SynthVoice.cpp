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
    pWheel = std::pow(2, ((newPitchWheelValue - 8192) / (8192.f)));
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

void SynthVoice::wetDryChanged(float w, float d) {
    wet = w;
    dry = d;
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
        float dryL = audioBlock.getSample(0, samp) * dry;
        float dryR = audioBlock.getSample(1, samp) * dry;
        float wetL = getSampleFromTable(false, position) * wet;
        float wetR = getSampleFromTable(true, position) * wet;
        if (adsr.isActive()) {
            audioBlock.setSample(0, samp, wetL);
            audioBlock.setSample(1, samp, wetR);
        }
        else {
            audioBlock.setSample(0, samp, dryL);
            audioBlock.setSample(1, samp, dryR);
        }

        if (usePortamento) frequency = linDecay(portamentoBase, frequency, frequencyTarget * pWheel, portamento, getSampleRate());
        else frequency = frequencyTarget * pWheel;

        if (exp) formant = expDecay(formant, formantTarget, formantRate, getSampleRate());
        else formant = linDecay(formantBase, formant, formantTarget, formantRate, getSampleRate());

        cycleLength = getSampleRate() * formant / frequency;

        if (adsr.isActive()) {
            position += formant;
            if (position >= leftTable.size()) {
                position -= cycleLength;
            }
        }
    }
    if (adsr.isActive()) adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
}