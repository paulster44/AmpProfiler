// BrowseList.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Util/PathHelpers.h"

// Generic ListBox wrapper for simple file lists (profiles or cabs).
template <typename ManagerType>
class BrowseList : public juce::Component,
                   public juce::ListBoxModel,
                   public juce::ChangeListener
{
public:
    using SelectFn = std::function<void(const juce::File&)>;

    BrowseList (ManagerType& mgr, juce::String /*title*/, SelectFn onSelect)
        : manager(mgr), onChosen(std::move(onSelect))
    {
        manager.addChangeListener(this);
        addAndMakeVisible(list);
        addAndMakeVisible(rescan);
        addAndMakeVisible(addBtn);

        list.setModel(this);
        rescan.setButtonText("Rescan");
        addBtn.setButtonText("Add…");

        rescan.onClick = [this] { manager.rescan(); };
        addBtn.onClick = [this]
        {
            juce::FileChooser fc("Choose file", juce::File(), "*");
            if (fc.browseForFileToOpen())
            {
                auto chosen = fc.getResult();
                juce::File targetFolder = pickTargetFolder();
                auto dest = targetFolder.getChildFile(chosen.getFileName());
                if (chosen != dest) chosen.copyFileTo(dest);
                manager.rescan();
            }
        };

        manager.rescan();
    }

    ~BrowseList() override { manager.removeChangeListener(this); }

    // Layout
    void resized() override
    {
        auto r = getLocalBounds().reduced(8);
        auto top = r.removeFromTop(28);
        rescan.setBounds(top.removeFromRight(90));
        top.removeFromRight(6);
        addBtn.setBounds(top.removeFromRight(90));
        list.setBounds(r);
    }

    // ListBoxModel
    int getNumRows() override { return manager.get().size(); }

    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (! juce::isPositiveAndBelow(row, getNumRows())) return;
        if (selected) g.fillAll(juce::Colours::darkgrey.withAlpha(0.25f));

        g.setColour(juce::Colours::white);
        g.drawText(manager.get()[row].name, 8, 0, width-16, height,
                   juce::Justification::centredLeft, false);
    }

    void listBoxItemClicked (int row, const juce::MouseEvent&) override
    {
        if (! juce::isPositiveAndBelow(row, getNumRows())) return;
        const auto& f = manager.get()[row].file;
        manager.setCurrent(f);
        if (onChosen) onChosen(f);
    }

    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        list.updateContent();
        repaint();
    }

private:
    juce::File pickTargetFolder() const
    {
        if constexpr (std::is_same<ManagerType, class ProfileManager>::value)
            return ap::getProfilesDir();
        else
            return ap::getCabsDir();
    }

    ManagerType& manager;
    juce::ListBox list;
    juce::TextButton rescan, addBtn;
    SelectFn onChosen;
};
