#pragma once

#include <JuceHeader.h>

// Forward-declare your editor (defined in PluginEditor.h)
class AmpProfilerAudioProcessorEditor;

/**
 * AmpProfilerAudioProcessor
 *  - Minimal, safe JUCE processor scaffold
 *  - Adds async file loaders used by your BrowseList/UI:
 *      void loadAmpProfileAsync(const juce::File& file);
 *      void loadCabIRAsync(const juce::File& file);
 */
class AmpProfilerAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    AmpProfilerAudioProcessor();
    ~AmpProfilerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #if ! JucePlugin_IsMidiEffect
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock; // keep double version hidden warnings away

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override          { return JucePlugin_WantsMidiInput; }
    bool producesMidi() const override         { return JucePlugin_ProducesMidiOutput; }
    bool isMidiEffect() const override         { return JucePlugin_IsMidiEffect; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override              { return 1; }
    int getCurrentProgram() override           { return 0; }
    void setCurrentProgram (int) override      {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // -------- NEW: async loaders BrowseList / BrowserPanel call --------
    void loadAmpProfileAsync (const juce::File& file);
    void loadCabIRAsync      (const juce::File& file);

    // (optional) simple accessors for UI
    const juce::String& getCurrentProfilePath() const noexcept { return currentProfilePath; }
    const juce::String& getCurrentIRPath()      const noexcept { return currentIRPath; }

private:
    //==============================================================================
    // Synchronous workers used by the async wrappers
    void loadAmpProfile (const juce::File& file);
    void loadCabIR      (const juce::File& file);

    // Convolution for cabinet IR
    juce::dsp::Convolution cabConvolution;
    juce::dsp::ProcessSpec processSpec {};
    bool specPrepared = false;

    // Swap-in a prepared convolution safely while audio is running
    juce::SpinLock convSwapLock;

    // Simple state that UI may read
    juce::String currentProfilePath;
    juce::String currentIRPath;

    // --- YOUR EXISTING MEMBERS (if any) ---
    // e.g. AudioProcessorValueTreeState apvts; ONNX runtime handles; parameters…
    // Copy them back here after pasting this file if you had them previously.

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpProfilerAudioProcessor)
};

