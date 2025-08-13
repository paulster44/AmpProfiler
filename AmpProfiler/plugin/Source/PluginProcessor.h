#pragma once

#include <JuceHeader.h>
#include <vector>
#include "Dsp/OnnxModel.h"
#include "Dsp/PartitionedConvolver.h"

// Forward declaration of your editor (defined in PluginEditor.h)
class AmpProfilerAudioProcessorEditor;

// Main processor
class AmpProfilerAudioProcessor : public juce::AudioProcessor
{
public:
    AmpProfilerAudioProcessor();
    ~AmpProfilerAudioProcessor() override = default;

    // AudioProcessor overrides
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Program handling (single program)
    int getNumPrograms() override                         { return 1; }
    int getCurrentProgram() override                      { return 0; }
    void setCurrentProgram (int) override                 {}
    const juce::String getProgramName (int) override      { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // UI
    bool hasEditor() const override                       { return true; }   // keep inline to avoid redefinition
    juce::AudioProcessorEditor* createEditor() override;                     // implemented in .cpp

    // Info
    const juce::String getName() const override           { return "AmpProfiler"; }
    bool acceptsMidi() const override                     { return false; }
    bool producesMidi() const override                    { return false; }
    bool isMidiEffect() const override                    { return false; }
    double getTailLengthSeconds() const override          { return 0.0; }

    // State
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Access to parameters for the editor
    juce::AudioProcessorValueTreeState& getVTS()          { return apvts; }

private:
    // ---- Parameters ----
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "PARAMS", createParameterLayout() };

    // ---- Oversampling helpers ----
    void rebuildOversampling (int factor, int samplesPerBlock);
    int  resolveOS() const;

    // ---- Asset loading ----
    void loadAssets();
    void loadProfile();

    // Profile for oversampling defaults/allowlist
    struct Profile {
        int osDefault = 1;              // default to 1x unless profile.json overrides
        std::vector<int> osAllowed { 1, 2, 4 };
    } profile;

    // Files discovered next to the built plugin
    juce::File modelAFile, modelBFile, cabAFile, cabBFile, profileFile;

    // DSP
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling; // configured in prepareToPlay
    int currentOS = 1; // 1x/2x/4x

    OnnxModel modelA, modelB;
    std::unique_ptr<PartitionedConvolver> cabA, cabB;

    // Working buffers
    juce::AudioBuffer<float> monoIn, upA, upB, downA, downB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpProfilerAudioProcessor)
};
