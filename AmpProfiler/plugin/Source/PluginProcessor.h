#pragma once
#include <JuceHeader.h>
#include "Dsp/OnnxModel.h"
#include "Dsp/PartitionedConvolver.h"
#include "Dsp/SignalUtils.h"
class AmpProfilerAudioProcessor : public juce::AudioProcessor {
public:
    AmpProfilerAudioProcessor();
    ~AmpProfilerAudioProcessor() override = default;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "AmpProfiler"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    int currentOS = 1;
    OnnxModel modelA, modelB;
    std::unique_ptr<PartitionedConvolver> cabA, cabB;
    juce::AudioBuffer<float> monoIn, upA, upB, downA, downB;
    juce::File modelAFile, modelBFile, cabAFile, cabBFile, profileFile;
    struct Profile { int osDefault=2; std::vector<int> osAllowed{1,2,4}; } profile;
    void loadAssets(); void loadProfile(); int resolveOS() const; void rebuildOversampling(int factor, int samplesPerBlock);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpProfilerAudioProcessor)
};
