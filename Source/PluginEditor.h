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
class IceboxAudioProcessorEditor  : public AudioProcessorEditor, private Slider::Listener, private ToggleButton::Listener, public juce::ChangeListener
{
public:
    IceboxAudioProcessorEditor (IceboxAudioProcessor&);
    ~IceboxAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    void sliderValueChanged(Slider* slider) override;
    void buttonStateChanged(Button* button) override;
    void buttonClicked(Button* button) override;
    void somethingChanged(int ID);
    void reload();
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    IceboxAudioProcessor& audioProcessor;

    Slider formantSlider;
    Slider formantDecaySlider;
    Slider formantDecayRateSlider;

    ToggleButton linearToggle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IceboxAudioProcessorEditor)
};
