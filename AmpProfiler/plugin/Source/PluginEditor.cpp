// PluginEditor.cpp
#include "PluginEditor.h"

//==============================================================================

AmpProfilerAudioProcessorEditor::AmpProfilerAudioProcessorEditor (AmpProfilerAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setResizable (true, true);
    setSize (780, 420);

    // Minimal styling for your existing controls so this file is drop-in safe.
    for (auto* s : { &inTrim, &outTrim, &morph })
    {
        addAndMakeVisible (*s);
        s->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    }

    addAndMakeVisible (osBox);
    addAndMakeVisible (cabToggle);
    osBox.addItemList (juce::StringArray{ "Auto", "1x", "2x", "4x" }, 1);
    osBox.setSelectedId (1, juce::dontSendNotification);
    cabToggle.setButtonText ("Cab");

    // Create the browser (left panel)
    browser = std::make_unique<BrowserPanel>(
        proc.getProfileManager(),
        proc.getCabManager(),
        // On profile chosen:
        [this](const juce::File& f) { proc.loadProfileAsync(f); },
        // On cab chosen:
        [this](const juce::File& f) { proc.loadCabAsync(f); }
    );
    addAndMakeVisible (*browser);
}

AmpProfilerAudioProcessorEditor::~AmpProfilerAudioProcessorEditor() = default;

//==============================================================================

void AmpProfilerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void AmpProfilerAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (12);

    auto left = r.removeFromLeft (260);
    if (browser) browser->setBounds(left);

    inTrim.setBounds   (r.removeFromTop (30)); r.removeFromTop (8);
    outTrim.setBounds  (r.removeFromTop (30)); r.removeFromTop (8);
    osBox.setBounds    (r.removeFromTop (24)); r.removeFromTop (8);
    cabToggle.setBounds(r.removeFromTop (24)); r.removeFromTop (8);
    morph.setBounds    (r.removeFromTop (30));
}
