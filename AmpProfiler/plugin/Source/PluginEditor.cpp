#include "PluginEditor.h"
AmpProfilerAudioProcessorEditor::AmpProfilerAudioProcessorEditor (AmpProfilerAudioProcessor& p)
: juce::AudioProcessorEditor (&p), proc (p) {
    setSize(420, 180);
    addAndMakeVisible(inTrim); inTrim.setTextValueSuffix(" dB"); inTrim.setRange(-24, 24, 0.1);
    addAndMakeVisible(outTrim); outTrim.setTextValueSuffix(" dB"); outTrim.setRange(-24, 24, 0.1);
    addAndMakeVisible(morph); morph.setRange(0.0, 1.0, 0.001);
    addAndMakeVisible(cabToggle);
    osBox.addItem("Auto",1); osBox.addItem("1x",2); osBox.addItem("2x",3); osBox.addItem("4x",4);
    addAndMakeVisible(osBox);
    inAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(*proc.apvts, "inTrim", inTrim);
    outAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(*proc.apvts, "outTrim", outTrim);
    morphAttach= std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(*proc.apvts, "morph", morph);
    osAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(*proc.apvts, "os", osBox);
    cabAttach  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(*proc.apvts, "cabOn", cabToggle);
}
void AmpProfilerAudioProcessorEditor::resized() {
    auto r = getLocalBounds().reduced(12);
    inTrim.setBounds(r.removeFromTop(30)); r.removeFromTop(8);
    outTrim.setBounds(r.removeFromTop(30)); r.removeFromTop(8);
    osBox.setBounds(r.removeFromTop(24));  r.removeFromTop(8);
    cabToggle.setBounds(r.removeFromTop(24)); r.removeFromTop(8);
    morph.setBounds(r.removeFromTop(30));
}
