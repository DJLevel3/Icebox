/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class IceboxAudioProcessorEditor  : public AudioProcessorEditor, private Slider::Listener
{
public:
    IceboxAudioProcessorEditor (IceboxAudioProcessor&);
    ~IceboxAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    void sliderValueChanged(Slider* slider) override;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    IceboxAudioProcessor& audioProcessor;

    Slider formantSlider;
    Slider formantDecaySlider;
    Slider formantDecayRateSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IceboxAudioProcessorEditor)
};
