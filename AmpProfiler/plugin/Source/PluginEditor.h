// PluginEditor.h
#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "UI/BrowserPanel.h"

//==============================================================================
// NOTE: This assumes you already have the controls used in your UI (inTrim,
// outTrim, osBox, cabToggle, morph). If their names differ in your project,
// just adjust in the .cpp layout.
class AmpProfilerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit AmpProfilerAudioProcessorEditor (AmpProfilerAudioProcessor&);
    ~AmpProfilerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AmpProfilerAudioProcessor& proc;

    // Your existing controls (already present in your project)
    juce::Slider inTrim, outTrim, morph;
    juce::ComboBox osBox;
    juce::ToggleButton cabToggle;

    std::unique_ptr<BrowserPanel> browser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpProfilerAudioProcessorEditor)
};
