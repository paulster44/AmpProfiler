#pragma once
#include <JuceHeader.h>
inline void sumToMono(const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& monoOut) {
    const int N = in.getNumSamples(); monoOut.setSize(1, N); monoOut.clear();
    for (int ch = 0; ch < in.getNumChannels(); ++ch)
        monoOut.addFrom(0, 0, in, ch, 0, N, ch == 0 ? 1.f : 0.5f);
}
