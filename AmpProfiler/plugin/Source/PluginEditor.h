#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class AmpProfilerAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    AmpProfilerAudioProcessorEditor (AmpProfilerAudioProcessor& p);
    void resized() override;
private:
    AmpProfilerAudioProcessor& proc;
    juce::Slider inTrim, outTrim, morph;
    juce::ComboBox osBox; juce::ToggleButton cabToggle {"Cab"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inAttach, outAttach, morphAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> cabAttach;
};
