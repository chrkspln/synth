#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "notes.h"

class KeyBeep : public juce::AudioAppComponent
{
private:
    float frequency = C4;
    float phase     = 0.0f; // phase of the sine wave
    double rate     = 44100.0;
    float amplitude = 0.3f; // influences volume
    int block_size  = 888;
    bool going_up   = true;

public:
    KeyBeep()
    {
        setAudioChannels(0, 2); // 0 input channels, 2 output channels
    }
    ~KeyBeep()
    {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override
    {
        std::cout << "audio-set sample rate: " << newSampleRate << std::endl;
        std::cout << "audio-set block size: " << samplesPerBlockExpected << std::endl;

        std::cout << "my sample rate: " << rate << std::endl;
        std::cout << "my block size: " << block_size << std::endl;
    }

    void releaseResources() override
    {
        std::cout << "auto-release resources I guess... or not?" << std::endl;
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();
        auto og_phase = phase;

        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            phase = og_phase;
            auto* ch_data = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                ch_data[sample] = amplitude * std::sin(phase);
                float phase_delta = static_cast<float>((juce::MathConstants<double>::twoPi * frequency / rate));
                phase = std::fmod(phase + phase_delta, juce::MathConstants<double>::twoPi);

                if (sample % block_size == 0)
                {
                    going_up ? frequency += 1.0f : frequency -= 5.0f;
                    if (frequency >= E5) { going_up = false; block_size *= 2; }
                    else if (frequency <= C4) { going_up = true; block_size /= 5; }
                }
            }
        }
    }
};

class BeepAppWin : public juce::DocumentWindow
{
private:
    KeyBeep beepComponent;

public:
    BeepAppWin() : DocumentWindow("SoundStuff",
                                  juce::Colours::lightgrey,
                                  DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(&beepComponent, true);
        setResizable(true, true);
        centreWithSize(300, 200);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class KeyBeepApp : public juce::JUCEApplication
{
public:
    KeyBeepApp() {}

    const juce::String getApplicationName() override { return "A Key Beep I guess (I hope)"; }
    const juce::String getApplicationVersion() override { return "69"; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new BeepAppWin());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<BeepAppWin> mainWindow;
};
