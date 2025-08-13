#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AmpProfilerAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit AmpProfilerAudioProcessorEditor (AmpProfilerAudioProcessor& p);

    void paint (juce::Graphics&) override;   // <-- add this to match your .cpp
    void resized() override;

private:
    AmpProfilerAudioProcessor& proc;

    juce::Slider inTrim, outTrim, morph;
    juce::ComboBox osBox;
    juce::ToggleButton cabToggle { "Cab" };

    using APVTS = juce::AudioProcessorValueTreeState;
    std::unique_ptr<APVTS::SliderAttachment>   inAttach, outAttach, morphAttach;
    std::unique_ptr<APVTS::ComboBoxAttachment> osAttach;
    std::unique_ptr<APVTS::ButtonAttachment>   cabAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpProfilerAudioProcessorEditor)
};
