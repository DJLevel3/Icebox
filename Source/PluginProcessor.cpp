#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
IceboxAudioProcessor::IceboxAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                       )
#endif
{
    synth.addSound(new SynthSound());
    synth.addVoice(new SynthVoice(96000));
    addParameter(formant = new AudioParameterFloat("formant", "Formant", -24, 24, 0));
    addParameter(formantDecay = new AudioParameterFloat("formantDecay", "Decay", -24, 24, 0));
    addParameter(formantDecayRate = new AudioParameterFloat("formantDecayRate", "Rate", 0.01, 2, 0.01));
    addParameter(formantDecayLinear = new AudioParameterBool("formantDecayLinear", "Linear", false));

    addParameter(attack = new AudioParameterFloat("attack", "Attack", 0, 1, DEF_ATTACK));
    addParameter(decay = new AudioParameterFloat("decay", "Decay", 0, 1, DEF_DECAY));
    addParameter(sustain = new AudioParameterFloat("sustain", "Sustain", 0, 100, DEF_SUSTAIN));
    addParameter(release = new AudioParameterFloat("release", "Release", 0, 2, DEF_RELEASE));

    addParameter(portamento = new AudioParameterFloat("portamento", "Portamento", 0, 100, 100));

    addParameter(wet = new AudioParameterFloat("wet", "Wet", 0, 100, 100));
    addParameter(dry = new AudioParameterFloat("dry", "Dry", 0, 100, 0));
}

IceboxAudioProcessor::~IceboxAudioProcessor()
{
}

//==============================================================================
const String IceboxAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IceboxAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool IceboxAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool IceboxAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double IceboxAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int IceboxAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int IceboxAudioProcessor::getCurrentProgram()
{
    return 0;
}

void IceboxAudioProcessor::setCurrentProgram (int index)
{
}

const String IceboxAudioProcessor::getProgramName (int index)
{
    return {};
}

void IceboxAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void IceboxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
            voice->formantChanged((*formant).get());
            voice->formantEnvelopeChanged((*formantDecay).get(), (*formantDecayRate).get(), true);
        }
    }
}

void IceboxAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IceboxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        return false;

    return true;
  #endif
}
#endif

void IceboxAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    for (int j = 0; j < buffer.getNumSamples(); j++) {
        if (SynthVoice* voice = dynamic_cast<SynthVoice*>(synth.getVoice(0))) {
            checkParams(voice);
            voice->leftRoll.writeSample(buffer.getSample(0, j));
            voice->rightRoll.writeSample(buffer.getSample(1, j));
        }
    }

    float b0 = buffer.getSample(0, 0);
    float b1 = buffer.getSample(1, 0);

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

void IceboxAudioProcessor::checkParams(SynthVoice* voice) {
    bool anythingChanged = false;

    // formant
    if ((*formant).get() != lastFormant) {
        lastFormant = (*formant).get();
        updateMe[0] = true;
        voice->formantChanged(lastFormant);
        anythingChanged = true;
    }

    // formant envelope
    if ((*formantDecay).get() != lastFormantDecay || (*formantDecayRate).get() != lastFormantDecayRate || ((*formantDecayLinear).get() != lastLinear)) {
        if (lastFormantDecay != (*formantDecay).get()) {
            lastFormantDecay = (*formantDecay).get();
            updateMe[1] = true;
        }
        if (lastFormantDecayRate != (*formantDecayRate).get()) {
            lastFormantDecayRate = (*formantDecayRate).get();
            updateMe[2] = true;
        }
        if (lastLinear != (*formantDecayLinear).get()) {
            lastLinear = (*formantDecayLinear).get();
            updateMe[3] = true;
        }
        voice->formantEnvelopeChanged(lastFormantDecay, lastFormantDecayRate, lastLinear);
        anythingChanged = true;
    }

    // adsr
    if ((*attack).get() != lastAttack || (*decay).get() != lastDecay || (*sustain).get() != lastSustain || (*release).get() != lastRelease) {
        if (lastAttack != (*attack).get()) {
            lastAttack = (*attack).get();
            updateMe[4] = true;
        }
        if (lastDecay != (*decay).get()) {
            lastDecay = (*decay).get();
            updateMe[5] = true;
        }
        if (lastSustain != (*sustain).get()) {
            lastSustain = (*sustain).get();
            updateMe[6] = true;
        }
        if (lastRelease != (*release).get()) {
            lastRelease = (*release).get();
            updateMe[7] = true;
        }
        voice->adsrChanged(lastAttack, lastDecay, lastSustain / 100, lastRelease);
        anythingChanged = true;
    }

    // portamento
    if ((*portamento).get() != lastPortamento) {
        lastPortamento = (*portamento).get();
        updateMe[8] = true;
        voice->portamentoChanged(lastPortamento / 100);
        anythingChanged = true;
    }

    // wet/dry
    if (lastWet != (*wet).get()) {
        lastWet = (*wet).get();
        updateMe[9] = true;
        voice->wetDryChanged(lastWet / 100., lastDry / 100.);
        anythingChanged = true;
    }
    
    if (lastDry != (*dry).get()) {
        lastDry = (*dry).get();
        updateMe[10] = true;
        voice->wetDryChanged(lastWet / 100., lastDry / 100.);
        anythingChanged = true;
    }

    if (anythingChanged) broadcaster.sendChangeMessage();
}

//==============================================================================
bool IceboxAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* IceboxAudioProcessor::createEditor()
{
    return new IceboxAudioProcessorEditor (*this);
}

//==============================================================================
void IceboxAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto stream = MemoryOutputStream(destData, true);
    stream.writeFloat((*formant).get());
    stream.writeFloat((*formantDecay).get());
    stream.writeFloat((*formantDecayRate).get());
    stream.writeBool((*formantDecayLinear).get());

    stream.writeFloat((*attack).get());
    stream.writeFloat((*decay).get());
    stream.writeFloat((*sustain).get());
    stream.writeFloat((*release).get());

    stream.writeFloat((*portamento).get());

    stream.writeFloat((*wet).get());
    stream.writeFloat((*dry).get());
}

void IceboxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto stream = MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false);
    (*formant).setValueNotifyingHost((*formant).convertTo0to1(stream.readFloat()));
    (*formantDecay).setValueNotifyingHost((*formantDecay).convertTo0to1(stream.readFloat()));
    (*formantDecayRate).setValueNotifyingHost((*formantDecayRate).convertTo0to1(stream.readFloat()));
    (*formantDecayLinear).setValueNotifyingHost(stream.readBool());

    (*attack).setValueNotifyingHost((*attack).convertTo0to1(stream.readFloat()));
    (*decay).setValueNotifyingHost((*attack).convertTo0to1(stream.readFloat()));
    (*sustain).setValueNotifyingHost((*attack).convertTo0to1(stream.readFloat()));
    (*release).setValueNotifyingHost((*attack).convertTo0to1(stream.readFloat()));

    (*portamento).setValueNotifyingHost((*attack).convertTo0to1(stream.readFloat()));

    (*wet).setValueNotifyingHost((*wet).convertTo0to1(stream.readFloat()));
    (*dry).setValueNotifyingHost((*dry).convertTo0to1(stream.readFloat()));

    lastFormant = -30;
    lastFormantDecay = -30;
    lastFormantDecayRate = -1;
    lastLinear = !(*formantDecayLinear).get();

    lastAttack = -1;
    lastDecay = -1;
    lastSustain = -1;
    lastRelease = -1;

    lastPortamento = -1;

    lastWet = -1;
    lastDry = -1;
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IceboxAudioProcessor();
}

// 