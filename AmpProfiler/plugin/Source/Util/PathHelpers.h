// PathHelpers.h
#pragma once
#include <juce_core/juce_core.h>

namespace ap
{
inline juce::File getUserDataDir()
{
    auto d = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("AmpProfiler");
    d.createDirectory();
    return d;
}

inline juce::File getProfilesDir()
{
    auto d = getUserDataDir().getChildFile("Profiles");
    d.createDirectory();
    return d;
}

inline juce::File getCabsDir()
{
    auto d = getUserDataDir().getChildFile("Cabinets");
    d.createDirectory();
    return d;
}
} // namespace ap
