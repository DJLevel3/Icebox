/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include "SynthVoice.h"
#include "SynthSound.h"
#include "FixedDelayBuffer.h"

//==============================================================================
/**
*/
class IceboxAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    IceboxAudioProcessor();
    ~IceboxAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    AudioParameterFloat formant{ "formant", "Formant", NormalisableRange<float>(-24,24), 0 };
    AudioParameterFloat formantDecay{ "formant", "Formant", NormalisableRange<float>(-24,24), 0 };
    AudioParameterFloat formantDecayRate{ "formant", "Formant", NormalisableRange<float>(0.01,2), 0.01 };

    float lastFormant = -30;
    float lastFormantDecay = -30;
    float lastFormantDecayRate = -1;


private:
    //==============================================================================
    FixedDelayBuffer<float> rollBufferL = FixedDelayBuffer<float>();
    FixedDelayBuffer<float> rollBufferR = FixedDelayBuffer<float>();
    Synthesiser synth;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IceboxAudioProcessor)
};
