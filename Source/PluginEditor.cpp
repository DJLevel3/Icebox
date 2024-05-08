/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
IceboxAudioProcessorEditor::IceboxAudioProcessorEditor (IceboxAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (480, 360);

    formantSlider.setSliderStyle(Slider::Rotary);
    formantSlider.setRange(-24, 24, 1);
    formantSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    formantSlider.setPopupDisplayEnabled(true, false, this);
    formantSlider.setTextValueSuffix(" semitones");
    formantSlider.setValue(0);
    formantSlider.addListener(this);

    formantDecaySlider.setSliderStyle(Slider::LinearVertical);
    formantDecaySlider.setRange(-24, 24, 1);
    formantDecaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    formantDecaySlider.setPopupDisplayEnabled(true, false, this);
    formantDecaySlider.setTextValueSuffix(" semitones");
    formantDecaySlider.setValue(0);
    formantDecaySlider.addListener(this);

    formantDecayRateSlider.setSliderStyle(Slider::LinearVertical);
    formantDecayRateSlider.setRange(0.01, 2, 0.01);
    formantDecayRateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    formantDecayRateSlider.setPopupDisplayEnabled(true, false, this);
    formantDecayRateSlider.setTextValueSuffix(" rate");
    formantDecayRateSlider.setValue(0.01);
    formantDecayRateSlider.addListener(this);

    linearToggle.setButtonText("Linear Decay");
    linearToggle.setToggleState(false, true);
    linearToggle.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
    linearToggle.setColour(ToggleButton::ColourIds::tickColourId, Colours::black);
    linearToggle.setColour(ToggleButton::ColourIds::tickDisabledColourId, Colours::black);
    linearToggle.addListener(this);

    resized();

    addAndMakeVisible(formantSlider);
    addAndMakeVisible(formantDecaySlider);
    addAndMakeVisible(formantDecayRateSlider);
    addAndMakeVisible(linearToggle);

    audioProcessor.broadcaster.addChangeListener(this);
}

IceboxAudioProcessorEditor::~IceboxAudioProcessorEditor()
{
    audioProcessor.broadcaster.removeChangeListener(this);
}

void IceboxAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    (*audioProcessor.formant).setValueNotifyingHost((*audioProcessor.formant).convertTo0to1(formantSlider.getValue()));
    (*audioProcessor.formantDecay).setValueNotifyingHost((*audioProcessor.formantDecay).convertTo0to1(formantDecaySlider.getValue()));
    (*audioProcessor.formantDecayRate).setValueNotifyingHost((*audioProcessor.formantDecayRate).convertTo0to1(formantDecayRateSlider.getValue()));
}

void IceboxAudioProcessorEditor::buttonStateChanged(Button* button)
{
    (*audioProcessor.formantDecayLinear).setValueNotifyingHost(linearToggle.getToggleState());
}
void IceboxAudioProcessorEditor::buttonClicked(Button* button)
{
    (*audioProcessor.formantDecayLinear).setValueNotifyingHost((*audioProcessor.formantDecayLinear).convertTo0to1(linearToggle.getToggleState()));
}

//==============================================================================
void IceboxAudioProcessorEditor::paint (Graphics& g)
{
    auto box = getLocalBounds();
    box.removeFromLeft(10);
    box.removeFromRight(10);
    box.removeFromBottom(10);
    box.removeFromTop(10);
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::skyblue);

    g.setColour (Colours::black);
    g.setFont (30.0f);
    g.drawFittedText ("Icebox", box.removeFromTop(50), Justification::centred, 1);
    auto left = box.removeFromLeft(box.getWidth()*2/3);

    g.setFont(15.0f);

    g.drawFittedText("Formant", left.removeFromTop(25), Justification::centred, 1);
    formantSlider.setBounds(left);

    g.drawFittedText("Envelope", box.removeFromTop(25), Justification::centred, 1);
    linearToggle.setBounds(box.removeFromBottom(25));
    left = box.removeFromLeft(box.getWidth()/2);

    g.drawFittedText("Target", left.removeFromTop(25), Justification::centred, 1);
    formantDecaySlider.setBounds(left);

    g.drawFittedText("Rate", box.removeFromTop(25), Justification::centred, 1);
    formantDecayRateSlider.setBounds(box);

}

void IceboxAudioProcessorEditor::resized()
{
    setSize(480, 360);

    float f = (*audioProcessor.formant).get();
    float fd = (*audioProcessor.formantDecay).get();
    float fdr = (*audioProcessor.formantDecayRate).get();
    bool l = (*audioProcessor.formantDecayLinear).get();

    linearToggle.setToggleState(l, false);
    formantSlider.setValue(f);
    formantDecaySlider.setValue(fd);
    formantDecayRateSlider.setValue(fdr);
}

void IceboxAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor.broadcaster) {
        resized();
        repaint();
    }
}