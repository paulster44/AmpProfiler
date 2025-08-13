// ProfileManager.h
#pragma once
#include <juce_core/juce_core.h>
#include "../Util/PathHelpers.h"

// Scans the user profiles folder (*.onnx) and remembers the current selection.
class ProfileManager : public juce::ChangeBroadcaster
{
public:
    struct Entry { juce::File file; juce::String name; };

    void rescan()
    {
        entries.clearQuick();
        auto dir = ap::getProfilesDir();

        for (juce::DirectoryIterator it(dir, false, "*.onnx"); it.next(); )
        {
            auto f = it.getFile();
            entries.add({ f, f.getFileNameWithoutExtension() });
        }
        sendChangeMessage();
    }

    const juce::Array<Entry>& get() const noexcept { return entries; }

    void setCurrent (juce::File f)
    {
        current = std::move(f);
        sendChangeMessage();
    }

    const juce::File& getCurrent() const noexcept { return current; }

private:
    juce::Array<Entry> entries;
    juce::File current;
};
