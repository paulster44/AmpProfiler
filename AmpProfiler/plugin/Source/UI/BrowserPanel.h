// BrowserPanel.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Profiles/ProfileManager.h"
#include "../Profiles/CabManager.h"
#include "BrowseList.h"

// A left-side panel with tabs for Profiles & Cabinets.
// Exposes two callbacks to your editor/processor when the user selects files.
class BrowserPanel : public juce::Component
{
public:
    using SelectFn = std::function<void(const juce::File&)>;

    BrowserPanel (ProfileManager& pm, CabManager& cm,
                  SelectFn onProfile, SelectFn onCab)
        : tabs(juce::TabbedButtonBar::TabsAtTop),
          profiles(pm, "Profiles", std::move(onProfile)),
          cabs(cm, "Cabinets", std::move(onCab))
    {
        addAndMakeVisible(tabs);
        tabs.addTab("Profiles", juce::Colours::transparentBlack, &profiles, false);
        tabs.addTab("Cabinets", juce::Colours::transparentBlack, &cabs, false);
        tabs.setTabBarDepth(32);
    }

    void resized() override { tabs.setBounds(getLocalBounds()); }

private:
    juce::TabbedComponent tabs;
    BrowseList<ProfileManager> profiles;
    BrowseList<CabManager>     cabs;
};
