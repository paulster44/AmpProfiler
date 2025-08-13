#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <thread> // for the async loaders

//==============================================================================

AmpProfilerAudioProcessor::AmpProfilerAudioProcessor()
#if ! JucePlugin_IsMidiEffect
     : AudioProcessor (
        BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
#endif
{
    // --- YOUR EXISTING CONSTRUCTION (if any) ---
}

AmpProfilerAudioProcessor::~AmpProfilerAudioProcessor() = default;

//==============================================================================

void AmpProfilerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    processSpec.sampleRate = sampleRate;
    processSpec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    processSpec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    cabConvolution.reset();
    cabConvolution.prepare (processSpec);
    specPrepared = true;
}

void AmpProfilerAudioProcessor::releaseResources()
{
    juce::SpinLock::ScopedLockType lock (convSwapLock);
    cabConvolution.reset();
    specPrepared = false;
}

#if ! JucePlugin_IsMidiEffect
bool AmpProfilerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Allow mono or stereo I/O with matching channel counts
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
}
#endif

void AmpProfilerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);

    juce::ScopedNoDenormals noDenormals;

    // If mono input and stereo output, duplicate channel to keep convolution happy
    if (getTotalNumInputChannels() < getTotalNumOutputChannels())
        for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
            buffer.copyFrom (ch, 0, buffer, 0, 0, buffer.getNumSamples());

    // Convolve (cabinet)
    if (specPrepared)
    {
        juce::SpinLock::ScopedTryLockType lock (convSwapLock);
        if (lock.isLocked())
        {
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> context (block);
            cabConvolution.process (context);
        }
    }
}

//==============================================================================

bool AmpProfilerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AmpProfilerAudioProcessor::createEditor()
{
    // Use your custom editor if present
    return new AmpProfilerAudioProcessorEditor (*this);

    // Or fall back to generic editor:
    // return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================

void AmpProfilerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("AmpProfilerState");
    state.setProperty ("profilePath", currentProfilePath, nullptr);
    state.setProperty ("irPath",      currentIRPath,      nullptr);

    juce::MemoryOutputStream mos (destData, false);
    state.writeToStream (mos);
}

void AmpProfilerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream mis (data, (size_t) sizeInBytes, false);
    if (auto vt = juce::ValueTree::readFromStream (mis))
    {
        currentProfilePath = vt.getProperty ("profilePath").toString();
        currentIRPath      = vt.getProperty ("irPath").toString();

        if (currentIRPath.isNotEmpty())
            loadCabIRAsync (juce::File (currentIRPath));

        // Profile reload is left to you; often heavier
        // if (currentProfilePath.isNotEmpty()) loadAmpProfileAsync (juce::File (currentProfilePath));
    }
}

//==============================================================================
// ------------------------ NEW: Async loaders ---------------------------------

void AmpProfilerAudioProcessor::loadAmpProfileAsync (const juce::File& file)
{
    std::thread ([this, f = file] { loadAmpProfile (f); }).detach();
}

void AmpProfilerAudioProcessor::loadCabIRAsync (const juce::File& file)
{
    std::thread ([this, f = file] { loadCabIR (f); }).detach();
}

// Synchronous worker: profile/model (stub to keep build green)
void AmpProfilerAudioProcessor::loadAmpProfile (const juce::File& file)
{
    if (! file.existsAsFile())
        return;

    // TODO: Replace with your ONNX/weights load
    currentProfilePath = file.getFullPathName();

    // Notify your editor if needed (ChangeBroadcaster, ValueTree, etc.)
}

// Synchronous worker: cabinet IR
void AmpProfilerAudioProcessor::loadCabIR (const juce::File& file)
{
    if (! file.existsAsFile() || ! specPrepared)
        return;

    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (file));
    if (reader == nullptr)
        return;

    // Read IR to a buffer
    juce::AudioBuffer<float> irBuffer ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&irBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

    // Prepare a new convolution with the IR, then atomically swap it in
    juce::dsp::Convolution newConv;
    newConv.prepare (processSpec);
    newConv.loadImpulseResponse (std::move (irBuffer),
                                 reader->sampleRate,
                                 juce::dsp::Convolution::Stereo::yes,
                                 juce::dsp::Convolution::Trim::no,
                                 juce::dsp::Convolution::Normalise::yes);

    {
        juce::SpinLock::ScopedLockType lock (convSwapLock);
        cabConvolution = std::move (newConv);
        currentIRPath  = file.getFullPathName();
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpProfilerAudioProcessor();
}

