#pragma once
#include <JuceHeader.h>
class PartitionedConvolver {
public:
    void loadIR(const juce::File& wav, size_t sampleRate, size_t head=128, size_t part=1024) {
        juce::AudioFormatManager fm; fm.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> r (fm.createReaderFor (wav)); if (!r) return;
        juce::AudioBuffer<float> irBuf (1, (int)r->lengthInSamples);
        r->read(&irBuf, 0, (int)r->lengthInSamples, 0, true, false);
        conv.reset();
        conv.loadImpulseResponse(std::move(irBuf), (double)sampleRate,
                                 juce::dsp::Convolution::Stereo::no,
                                 juce::dsp::Convolution::Trim::no,
                                 juce::dsp::Convolution::Normalise::no);
        juce::dsp::ProcessSpec spec { (double)sampleRate, 512, 1 }; conv.prepare(spec);
    }
    void process(float* mono, int num) {
        juce::AudioBuffer<float> buf(mono, 1, num);
        juce::dsp::AudioBlock<float> blk(buf);
        juce::dsp::ProcessContextReplacing<float> ctx(blk);
        conv.process(ctx);
    }
private: juce::dsp::Convolution conv;
};
