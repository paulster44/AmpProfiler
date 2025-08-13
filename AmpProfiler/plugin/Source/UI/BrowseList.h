// plugin/Source/UI/BrowseList.h
#pragma once
#include <JuceHeader.h>

class AmpProfilerAudioProcessor; // fwd

/** Minimal “load files” strip using JUCE 8's async FileChooser. */
class BrowseList : public juce::Component
{
public:
    explicit BrowseList (AmpProfilerAudioProcessor& p)
        : proc (p)
    {
        addAndMakeVisible (loadAmpBtn);
        loadAmpBtn.setButtonText ("Load Amp Profile…");
        loadAmpBtn.onClick = [this]
        {
            pickFile ("*.json;*.onnx", [this] (juce::File f) { proc.loadAmpProfileAsync (f); });
        };

        addAndMakeVisible (loadCabBtn);
        loadCabBtn.setButtonText ("Load Cabinet IR…");
        loadCabBtn.onClick = [this]
        {
            pickFile ("*.wav;*.aif;*.aiff", [this] (juce::File f) { proc.loadCabIRAsync (f); });
        };
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (12);
        loadAmpBtn.setBounds (r.removeFromTop (32));
        r.removeFromTop (8);
        loadCabBtn.setBounds (r.removeFromTop (32));
    }

private:
    // JUCE 8: non-blocking file chooser. Keep it alive with a member unique_ptr.
    void pickFile (const juce::String& wildcard, std::function<void(juce::File)> onChosen)
    {
        auto flags = juce::FileBrowserComponent::openMode
                   | juce::FileBrowserComponent::canSelectFiles;

        // Create with last used directory and wildcard
        fileChooser = std::make_unique<juce::FileChooser> ("Select file", lastDir, wildcard, true);

        fileChooser->launchAsync (flags, [this, cb = std::move (onChosen)] (const juce::FileChooser& fc) mutable
        {
            auto f = fc.getResult();
            if (f.existsAsFile())
            {
                lastDir = f.getParentDirectory();
                cb (f);
            }

            // Important: release chooser after callback returns
            fileChooser.reset();
        });
    }

    AmpProfilerAudioProcessor& proc;
    juce::TextButton loadAmpBtn, loadCabBtn;

    juce::File lastDir { juce::File::getSpecialLocation (juce::File::userHomeDirectory) };
    std::unique_ptr<juce::FileChooser> fileChooser; // keeps chooser alive during async op
};
