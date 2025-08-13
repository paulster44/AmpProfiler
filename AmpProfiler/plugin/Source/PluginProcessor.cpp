#include "PluginProcessor.h"
#include "PluginEditor.h"
AmpProfilerAudioProcessor::AmpProfilerAudioProcessor()
  : juce::AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    apvts = std::make_unique<juce::AudioProcessorValueTreeState>(*this, nullptr, "PARAMS", createLayout());
}
juce::AudioProcessorValueTreeState::ParameterLayout AmpProfilerAudioProcessor::createLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.emplace_back(std::make_unique<juce::AudioParameterFloat>("inTrim","Input Trim", -24.f, 24.f, 0.f));
    p.emplace_back(std::make_unique<juce::AudioParameterFloat>("outTrim","Output Trim",-24.f, 24.f, 0.f));
    p.emplace_back(std::make_unique<juce::AudioParameterChoice>("os","Oversampling", juce::StringArray{"Auto","1x","2x","4x"}, 0));
    p.emplace_back(std::make_unique<juce::AudioParameterBool>("cabOn","Cab Enabled", true));
    p.emplace_back(std::make_unique<juce::AudioParameterFloat>("morph","Morph", 0.f, 1.f, 0.f));
    return { p.begin(), p.end() };
}
bool AmpProfilerAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const {
    return l.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || l.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}
void AmpProfilerAudioProcessor::loadProfile() {
    profile = {}; if (!profileFile.existsAsFile()) return;
    auto v = juce::JSON::parse(profileFile); if (!v.isObject()) return;
    auto* o = v.getDynamicObject();
    if (o->hasProperty("oversample_default")) profile.osDefault = int(o->getProperty("oversample_default"));
    if (o->hasProperty("oversample_allowed")) { profile.osAllowed.clear();
        if (auto* arr = o->getProperty("oversample_allowed").getArray())
            for (auto& a : *arr) profile.osAllowed.push_back(int(a)); }
}
int AmpProfilerAudioProcessor::resolveOS() const {
    int choice = apvts->getRawParameterValue("os")->load(); // 0=Auto,1=1x,2=2x,3=4x
    int desired = choice==0 ? profile.osDefault : (choice==1?1:choice==2?2:4);
    if (std::find(profile.osAllowed.begin(), profile.osAllowed.end(), desired) == profile.osAllowed.end())
        desired = profile.osDefault; return desired;
}
void AmpProfilerAudioProcessor::rebuildOversampling(int factor, int samplesPerBlock) {
    const unsigned stages = (factor==4?2:factor==2?1:0);
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(1, stages,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
    oversampling->initProcessing((size_t)samplesPerBlock);
    currentOS = factor;
    upA.setSize(1, samplesPerBlock * factor);
    upB.setSize(1, samplesPerBlock * factor);
    downA.setSize(1, samplesPerBlock);
    downB.setSize(1, samplesPerBlock);
}
void AmpProfilerAudioProcessor::loadAssets() {
    auto dir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
    auto modelsDir = dir.getChildFile("Models");
    modelAFile = modelsDir.getChildFile("modelA.onnx");
    modelBFile = modelsDir.getChildFile("modelB.onnx");
    cabAFile   = modelsDir.getChildFile("cabA.wav");
    cabBFile   = modelsDir.getChildFile("cabB.wav");
    profileFile= modelsDir.getChildFile("profile.json");
    loadProfile();
    if (modelAFile.existsAsFile()) modelA.load(modelAFile);
    if (modelBFile.existsAsFile()) modelB.load(modelBFile);
    cabA = std::make_unique<PartitionedConvolver>(); cabB = std::make_unique<PartitionedConvolver>();
    auto sr = getSampleRate();
    if (cabAFile.existsAsFile()) cabA->loadIR(cabAFile, (size_t)sr, 128, 1024);
    if (cabBFile.existsAsFile()) cabB->loadIR(cabBFile, (size_t)sr, 128, 1024);
}
void AmpProfilerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    monoIn.setSize(1, samplesPerBlock); loadAssets(); rebuildOversampling(resolveOS(), samplesPerBlock);
}
void AmpProfilerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    const int N = buffer.getNumSamples();
    monoIn.setSize(1, N); monoIn.clear();
    for (int ch=0; ch<buffer.getNumChannels(); ++ch) monoIn.addFrom(0,0,buffer,ch,0,N, ch==0?1.f:0.5f);
    float inTrim = juce::Decibels::decibelsToGain(apvts->getRawParameterValue("inTrim")->load());
    float outTrim= juce::Decibels::decibelsToGain(apvts->getRawParameterValue("outTrim")->load());
    monoIn.applyGain(inTrim);
    int desired = resolveOS(); if (desired != currentOS) rebuildOversampling(desired, N);
    {   juce::dsp::AudioBlock<float> inBlk(monoIn); auto up = oversampling->processSamplesUp(inBlk);
        upA.copyFrom(0,0, up.getChannelPointer(0), up.getNumSamples());
        modelA.process(upA.getWritePointer(0), upA.getNumSamples());
        juce::dsp::AudioBlock<float> upBlk(upA);
		oversampling->processSamplesDown(upBlk);                   // in-place
		downA.copyFrom(0,0, upBlk.getChannelPointer(0), N);
    }
    if (modelBFile.existsAsFile()) {
        juce::dsp::AudioBlock<float> inBlk(monoIn); auto up = oversampling->processSamplesUp(inBlk);
        upB.copyFrom(0,0, up.getChannelPointer(0), up.getNumSamples());
        modelB.process(upB.getWritePointer(0), upB.getNumSamples());
        juce::dsp::AudioBlock<float> upBlk(upB);
		oversampling->processSamplesDown(upBlk);                   // in-place
		downB.copyFrom(0,0, upBlk.getChannelPointer(0), N);
    } else downB.makeCopyOf(downA);
    bool cabOn = apvts->getRawParameterValue("cabOn")->load() > 0.5f;
    if (cabOn) { if (cabA) cabA->process(downA.getWritePointer(0), N); if (cabB) cabB->process(downB.getWritePointer(0), N); }
    float m = apvts->getRawParameterValue("morph")->load();
    juce::AudioBuffer<float> monoOut(1, N); monoOut.makeCopyOf(downA); monoOut.applyGain(1.f - m);
    monoOut.addFrom(0,0, downB, 0,0, N, m); monoOut.applyGain(outTrim);
    for (int ch=0; ch<buffer.getNumChannels(); ++ch) buffer.copyFrom(ch,0, monoOut,0,0,N);
}
void AmpProfilerAudioProcessor::getStateInformation (juce::MemoryBlock& dest){}
void AmpProfilerAudioProcessor::setStateInformation (const void*, int){}

bool AmpProfilerAudioProcessor::hasEditor() const
{
    return true;   // we do have a GUI editor
}

juce::AudioProcessorEditor* AmpProfilerAudioProcessor::createEditor()
{
    return new AmpProfilerAudioProcessorEditor(*this);
}

// JUCE factory required by VST3 build
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpProfilerAudioProcessor(); // <-- use your exact processor class name
}