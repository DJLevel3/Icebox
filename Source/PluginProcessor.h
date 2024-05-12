#pragma once

#include <JuceHeader.h>
#include <vector>
#include "SynthVoice.h"
#include "SynthSound.h"
#include "FixedDelayBuffer.h"

#define DEF_ATTACK 0.01
#define DEF_DECAY 0
#define DEF_SUSTAIN 100
#define DEF_RELEASE 0.1

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

    void checkParams(SynthVoice* voice);


    AudioParameterFloat* formant;
    AudioParameterFloat* formantDecay;
    AudioParameterFloat* formantDecayRate;
    AudioParameterBool* formantDecayLinear;

    AudioParameterFloat* attack;
    AudioParameterFloat* decay;
    AudioParameterFloat* sustain;
    AudioParameterFloat* release;

    AudioParameterFloat* portamento;

    float lastFormant = -30;
    float lastFormantDecay = -30;
    float lastFormantDecayRate = -1;
    bool lastLinear = true;

    float lastAttack = -1;
    float lastDecay = -1;
    float lastSustain = -1;
    float lastRelease = -1;

    float lastPortamento = -1;

    ChangeBroadcaster broadcaster;

private:
    //==============================================================================
    Synthesiser synth;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IceboxAudioProcessor)
};
