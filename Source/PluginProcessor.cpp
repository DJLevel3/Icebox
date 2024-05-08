/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

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
            voice->formantChanged(formant.get());
            voice->formantEnvelopeChanged(formantDecay.get(), formantDecayRate.get(), true);
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
    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();
    std::vector<float> inputData[2];

    for (int j = 0; j < buffer.getNumSamples(); j++) {
        inputData[0].push_back(buffer.getSample(0, j));
        inputData[1].push_back(buffer.getSample(1, j));
        for (int i = 0; i < synth.getNumVoices(); i++) {
            if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i))) {
                if (formant.get() != lastFormant) {
                    voice->formantChanged(formant.get());
                    lastFormant = formant.get();
                }
                if (formantDecay.get() != lastFormantDecay || formantDecayRate.get() != lastFormantDecayRate) {
                    voice->formantEnvelopeChanged(formantDecay.get(), formantDecayRate.get(), true);
                    lastFormantDecay = formantDecay.get();
                    lastFormantDecayRate = formantDecayRate.get();
                }
                voice->leftRoll.writeSample(buffer.getSample(0, j));
                voice->rightRoll.writeSample(buffer.getSample(1, j));
            }
        }
    }

    buffer.clear(0, buffer.getNumSamples());

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    for (int j = 0; j < buffer.getNumSamples(); j++) {
        buffer.setSample(0, j, buffer.getSample(0, j));
        buffer.setSample(1, j, buffer.getSample(1, j));
    }
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
    MemoryOutputStream(destData, true).writeFloat(formant.get());
    MemoryOutputStream(destData, true).writeFloat(formantDecay.get());
    MemoryOutputStream(destData, true).writeFloat(formantDecayRate.get());
}

void IceboxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto stream = MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false);
    formant.setValueNotifyingHost(formant.convertTo0to1(stream.readFloat()));
    formantDecay.setValueNotifyingHost(formantDecay.convertTo0to1(stream.readFloat()));
    formantDecayRate.setValueNotifyingHost(formantDecayRate.convertTo0to1(stream.readFloat()));

    lastFormant = formant.get() - 1;
    lastFormantDecay = formantDecay.get() - 1;
    lastFormantDecayRate = formantDecayRate.get() - 1;

}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IceboxAudioProcessor();
}

// 