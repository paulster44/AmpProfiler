// plugin/Source/UI/BrowserPanel.h
#pragma once
#include <JuceHeader.h>
#include "BrowseList.h"

class AmpProfilerAudioProcessor;

/** Simple two-tab browser: Profiles and Cabinets. */
class BrowserPanel : public juce::Component
{
public:
    explicit BrowserPanel (AmpProfilerAudioProcessor& p)
        : proc (p),
          tabs (juce::TabbedButtonBar::TabsAtTop)   // constructor takes orientation only
    {
        // Build pages
        profilePage = std::make_unique<BrowseList> (proc);
        profilePage->setName ("Profiles");

        cabPage = std::make_unique<BrowseList> (proc);
        cabPage->setName ("Cabinets");

        // Add tabs – TabbedComponent takes ownership when deleteComponentWhenNotNeeded = true
        tabs.addTab ("Profiles", juce::Colours::transparentBlack, profilePage.release(), true);
        tabs.addTab ("Cabinets", juce::Colours::transparentBlack, cabPage.release(), true);

        addAndMakeVisible (tabs);
    }

    void resized() override
    {
        tabs.setBounds (getLocalBounds());
    }

private:
    AmpProfilerAudioProcessor& proc;

    // DO NOT brace-initialise here; we already initialise in the ctor init-list.
    juce::TabbedComponent tabs;

    // Temporary owners until we hand pages to the TabbedComponent
    std::unique_ptr<BrowseList> profilePage, cabPage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrowserPanel)
};

