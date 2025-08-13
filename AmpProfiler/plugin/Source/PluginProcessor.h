#pragma once

#include <JuceHeader.h>
#include "Dsp/OnnxModel.h"

//==============================================================================
// Forward declaration of your editor (defined in PluginEditor.h)
class AmpProfilerAudioProcessorEditor;

//==============================================================================
// Main processor
class AmpProfilerAudioProcessor : public juce::AudioProcessor
{
public:
    AmpProfilerAudioProcessor();
    ~AmpProfilerAudioProcessor() override;

    //==============================================================================
    // AudioProcessor overrides
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
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
    // Build the parameter layout (defined in .cpp)
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Parameters
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "PARAMS", createParameterLayout() };

    // DSP
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;  // configured in prepareToPlay
    OnnxModel model;                                               // ONNX runtime wrapper
    juce::dsp::ProcessSpec spec {};                                // cached spec

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpProfilerAudioProcessor)
};
