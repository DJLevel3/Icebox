#include "PluginEditor.h"

//==============================================================================
IceboxAudioProcessorEditor::IceboxAudioProcessorEditor (IceboxAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    resized();

    formantSlider.setSliderStyle(Slider::Rotary);
    formantSlider.setRange(-24, 24, 0.1);
    formantSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    formantSlider.setPopupDisplayEnabled(true, false, this);
    formantSlider.setTextValueSuffix(" semitones");
    formantSlider.setValue(0);
    formantSlider.addListener(this);

    formantDecaySlider.setSliderStyle(Slider::LinearVertical);
    formantDecaySlider.setRange(-24, 24, 0.1);
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

    aSlider.setSliderStyle(Slider::LinearVertical);
    aSlider.setRange(0, 1, 0.01);
    aSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    aSlider.setPopupDisplayEnabled(true, false, this);
    aSlider.setTextValueSuffix(" seconds");
    aSlider.setValue(0);
    aSlider.addListener(this);

    dSlider.setSliderStyle(Slider::LinearVertical);
    dSlider.setRange(0, 1, 0.01);
    dSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    dSlider.setPopupDisplayEnabled(true, false, this);
    dSlider.setTextValueSuffix(" seconds");
    dSlider.setValue(0);
    dSlider.addListener(this);

    sSlider.setSliderStyle(Slider::LinearVertical);
    sSlider.setRange(0, 100, 0.1);
    sSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    sSlider.setPopupDisplayEnabled(true, false, this);
    sSlider.setTextValueSuffix("%");
    sSlider.setValue(100);
    sSlider.addListener(this);

    rSlider.setSliderStyle(Slider::LinearVertical);
    rSlider.setRange(0, 2, 0.01);
    rSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    rSlider.setPopupDisplayEnabled(true, false, this);
    rSlider.setTextValueSuffix(" seconds");
    rSlider.setValue(0);
    rSlider.addListener(this);

    portamentoSlider.setSliderStyle(Slider::LinearVertical);
    portamentoSlider.setRange(0, 100, 0.1);
    portamentoSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    portamentoSlider.setPopupDisplayEnabled(true, false, this);
    portamentoSlider.setTextValueSuffix(" rate");
    portamentoSlider.setValue(100);
    portamentoSlider.addListener(this);

    wetSlider.setSliderStyle(Slider::Rotary);
    wetSlider.setRange(0, 100, 1);
    wetSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    wetSlider.setPopupDisplayEnabled(true, false, this);
    wetSlider.setTextValueSuffix("%");
    wetSlider.setValue(100);
    wetSlider.addListener(this);

    drySlider.setSliderStyle(Slider::Rotary);
    drySlider.setRange(0, 100, 1);
    drySlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    drySlider.setPopupDisplayEnabled(true, false, this);
    drySlider.setTextValueSuffix("%");
    drySlider.setValue(0);
    drySlider.addListener(this);

    formantSlider.setComponentID("0");
    formantDecaySlider.setComponentID("1");
    formantDecayRateSlider.setComponentID("2");
    linearToggle.setComponentID("3");
    aSlider.setComponentID("4");
    dSlider.setComponentID("5");
    sSlider.setComponentID("6");
    rSlider.setComponentID("7");
    portamentoSlider.setComponentID("8");
    wetSlider.setComponentID("9");
    drySlider.setComponentID("10");

    addAndMakeVisible(formantSlider);
    addAndMakeVisible(formantDecaySlider);
    addAndMakeVisible(formantDecayRateSlider);
    addAndMakeVisible(linearToggle);
    addAndMakeVisible(aSlider);
    addAndMakeVisible(dSlider);
    addAndMakeVisible(sSlider);
    addAndMakeVisible(rSlider);
    addAndMakeVisible(portamentoSlider);
    addAndMakeVisible(wetSlider);
    addAndMakeVisible(drySlider);

    audioProcessor.broadcaster.addChangeListener(this);
}

IceboxAudioProcessorEditor::~IceboxAudioProcessorEditor()
{
    audioProcessor.broadcaster.removeChangeListener(this);
}

void IceboxAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    int compID = slider->getComponentID().getIntValue();
    somethingChanged(compID);
}

void IceboxAudioProcessorEditor::buttonStateChanged(Button* button)
{
    int compID = button->getComponentID().getIntValue();
    somethingChanged(compID);
}
void IceboxAudioProcessorEditor::buttonClicked(Button* button)
{
    int compID = button->getComponentID().getIntValue();
    somethingChanged(compID);
}

void IceboxAudioProcessorEditor::somethingChanged(int compID)
{
    switch (compID) {
    case 0:
        (*audioProcessor.formant).setValueNotifyingHost((*audioProcessor.formant).convertTo0to1(formantSlider.getValue()));
        break;
    case 1:
        (*audioProcessor.formantDecay).setValueNotifyingHost((*audioProcessor.formantDecay).convertTo0to1(formantDecaySlider.getValue()));
        break;
    case 2:
        (*audioProcessor.formantDecayRate).setValueNotifyingHost((*audioProcessor.formantDecayRate).convertTo0to1(formantDecayRateSlider.getValue()));
        break;
    case 3:
        (*audioProcessor.formantDecayLinear).setValueNotifyingHost(linearToggle.getToggleState());
        break;
    case 4:
        (*audioProcessor.attack).setValueNotifyingHost((*audioProcessor.attack).convertTo0to1(aSlider.getValue()));
        break;
    case 5:
        (*audioProcessor.decay).setValueNotifyingHost((*audioProcessor.decay).convertTo0to1(dSlider.getValue()));
        break;
    case 6:
        (*audioProcessor.sustain).setValueNotifyingHost((*audioProcessor.sustain).convertTo0to1(sSlider.getValue()));
        break;
    case 7:
        (*audioProcessor.release).setValueNotifyingHost((*audioProcessor.release).convertTo0to1(rSlider.getValue()));
        break;
    case 8:
        (*audioProcessor.portamento).setValueNotifyingHost((*audioProcessor.portamento).convertTo0to1(portamentoSlider.getValue()));
        break;
    case 9:
        (*audioProcessor.wet).setValueNotifyingHost((*audioProcessor.wet).convertTo0to1(wetSlider.getValue()));
        break;
    case 10:
        (*audioProcessor.dry).setValueNotifyingHost((*audioProcessor.dry).convertTo0to1(drySlider.getValue()));
        break;
    }
}

//==============================================================================
void IceboxAudioProcessorEditor::paint (Graphics& g)
{
    auto bounds = getLocalBounds();
    bounds.removeFromLeft(10);
    bounds.removeFromRight(10);
    bounds.removeFromBottom(10);
    bounds.removeFromTop(10);

    auto box = bounds.removeFromTop(320);
    box.removeFromBottom(10);
    g.fillAll (Colours::skyblue);

    g.setColour (Colours::black);
    g.setFont (30.0f);
    g.drawFittedText ("Icebox", box.removeFromTop(50).removeFromTop(30), Justification::centred, 1);
    auto left = box.removeFromLeft(320);

    auto lu = left.removeFromLeft(120);
    auto ll = lu.removeFromBottom(lu.getHeight() / 2);

    g.setFont(20.0f);

    g.drawFittedText("Formant", left.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    formantSlider.setBounds(left);

    g.setFont(15.0f);

    g.drawFittedText("Wet Level", lu.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    wetSlider.setBounds(lu);

    g.drawFittedText("Dry Level", ll.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    drySlider.setBounds(ll);
    

    g.drawFittedText("Envelope", box.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    linearToggle.setBounds(box.removeFromBottom(25));
    left = box.removeFromLeft(80);

    g.drawFittedText("Target", left.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    formantDecaySlider.setBounds(left);

    g.drawFittedText("Rate", box.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    formantDecayRateSlider.setBounds(box);

    auto aBox = bounds.removeFromLeft(80);
    auto dBox = bounds.removeFromLeft(80);
    auto sBox = bounds.removeFromLeft(80);
    auto rBox = bounds.removeFromLeft(80);
    bounds.removeFromLeft(35);
    auto pBox = bounds.removeFromLeft(90);

    g.drawFittedText("Attack", aBox.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    aSlider.setBounds(aBox);

    g.drawFittedText("Decay", dBox.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    dSlider.setBounds(dBox);

    g.drawFittedText("Sustain", sBox.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    sSlider.setBounds(sBox);

    g.drawFittedText("Release", rBox.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    rSlider.setBounds(rBox);

    g.drawFittedText("Portamento", pBox.removeFromTop(25).removeFromBottom(20), Justification::centred, 1);
    portamentoSlider.setBounds(pBox);
}

void IceboxAudioProcessorEditor::resized()
{
    setSize(500, 500);
}

void IceboxAudioProcessorEditor::reload() {
    float f = audioProcessor.lastFormant;
    float fd = (audioProcessor.lastFormantDecay);
    float fdr = (audioProcessor.lastFormantDecayRate);
    bool l = (audioProcessor.lastLinear);

    float a = (audioProcessor.lastAttack);
    float d = (audioProcessor.lastDecay);
    float s = (audioProcessor.lastSustain);
    float r = (audioProcessor.lastRelease);

    float p = (audioProcessor.lastPortamento);

    float wet = (audioProcessor.lastWet);
    float dry = (audioProcessor.lastDry);

    if (audioProcessor.updateMe[0]) formantSlider.setValue(f, juce::dontSendNotification);
    if (audioProcessor.updateMe[1]) formantDecaySlider.setValue(fd, juce::dontSendNotification);
    if (audioProcessor.updateMe[2]) formantDecayRateSlider.setValue(fdr, juce::dontSendNotification);
    if (audioProcessor.updateMe[3]) linearToggle.setToggleState(l, false);

    if (audioProcessor.updateMe[4]) aSlider.setValue(a, juce::dontSendNotification);
    if (audioProcessor.updateMe[5]) dSlider.setValue(d, juce::dontSendNotification);
    if (audioProcessor.updateMe[6]) sSlider.setValue(s, juce::dontSendNotification);
    if (audioProcessor.updateMe[7]) rSlider.setValue(r, juce::dontSendNotification);

    if (audioProcessor.updateMe[8]) portamentoSlider.setValue(p, juce::dontSendNotification);

    if (audioProcessor.updateMe[9]) wetSlider.setValue(wet, juce::dontSendNotification);
    if (audioProcessor.updateMe[10]) drySlider.setValue(dry, juce::dontSendNotification);
    for (int i = 0; i < 11; i++) {
        audioProcessor.updateMe[i] = false;
    }
}

void IceboxAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor.broadcaster) {
        reload();
        resized();
        repaint();
    }
}