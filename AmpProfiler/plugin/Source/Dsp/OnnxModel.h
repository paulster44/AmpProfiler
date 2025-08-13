#pragma once
#include <onnxruntime_cxx_api.h>
#include <JuceHeader.h>
#include <array>
#include <cstring>
#include <string>

class OnnxModel {
public:
    struct Settings { size_t context = 1024; } cfg;

    void load(const juce::File& onnxFile) {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "AmpProfiler");

        Ort::SessionOptions opt;
        opt.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // Use the platform-correct character type for ORT paths
        using ort_char_t = ORTCHAR_T;
        std::basic_string<ort_char_t> modelPath;
       #if defined(_WIN32)
        modelPath = std::wstring(onnxFile.getFullPathName().toWideCharPointer());
       #else
        modelPath = std::string(onnxFile.getFullPathName().toRawUTF8());
       #endif

        session = std::make_unique<Ort::Session>(*env, modelPath.c_str(), opt);

        Ort::AllocatorWithDefaultOptions alloc;
        inputName  = session->GetInputNameAllocated(0, alloc).get();
        outputName = session->GetOutputNameAllocated(0, alloc).get();

        ctx.setSize(1, (int)cfg.context);
        ctx.clear();
        tmp.setSize(1, 0);
    }

    void process(float* samples, int num) {
        if (!session) return;

        const int T = (int)cfg.context + num;

        tmp.setSize(1, T);
        // buffer -> buffer (6 args)
        tmp.copyFrom(0, 0, ctx, 0, 0, (int)cfg.context);
        // pointer -> buffer (4 args)
        tmp.copyFrom(0, (int)cfg.context, samples, num);

        std::array<int64_t, 2> shape{ 1, (int64_t)T };
        Ort::MemoryInfo mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        auto* dataIn = tmp.getWritePointer(0);
        auto inTensor = Ort::Value::CreateTensor<float>(mem, dataIn, (size_t)T, shape.data(), 2);

        const char* inNames[]  = { inputName.c_str() };
        const char* outNames[] = { outputName.c_str() };
        auto out = session->Run(Ort::RunOptions{nullptr}, inNames, &inTensor, 1, outNames, 1);

        float* y = out.front().GetTensorMutableData<float>();
        std::memcpy(samples, y + cfg.context, sizeof(float) * (size_t)num);

        // Update rolling context with tail of input
        ctx.setSize(1, (int)cfg.context);
        std::memcpy(ctx.getWritePointer(0),
                    dataIn + (T - (int)cfg.context),
                    sizeof(float) * (size_t)cfg.context);
    }

private:
    std::unique_ptr<Ort::Env>     env;
    std::unique_ptr<Ort::Session> session;
    std::string inputName, outputName;
    juce::AudioBuffer<float> tmp, ctx;
};
