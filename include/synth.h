#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <fstream>
#include <string>  
#include "notes.h"

static constexpr int8_t MAX_NOTES = 10;
static constexpr float speakers_ch_amplitude = 0.2f;
void loadSelectedMelody();

struct Note
{
    float frequency     = 0.0f;
    float phase         = 0.0f;
    float phase_delta   = 0.0f;
    float amplitude     = 1.0f;
    bool is_active      = false;
    int key_code        = -1;
};

struct VisualNote
{
    int key_code           = -1;
    bool is_lit            = false;
    float splash_radius    = 0.0f;
    float splash_opacity   = 0.0f;
    juce::Rectangle<float> bounds;
    juce::Colour base_colour;
    
};



class Synth : public juce::AudioAppComponent, public juce::KeyListener, public juce::Timer
{
private:
    enum class WaveformType
    {
        Sine,
        Sawtooth,
        Square,
        Triangle
    };
    WaveformType waveform = WaveformType::Sine;
    float sample_rate{};
    std::array<Note, MAX_NOTES> active_notes{};
    std::map<int, VisualNote> visual_notes;
    std::map<int, Note> note_map =
    {
        // piano
        {'A', {E3}}, {'W', {F3}}, {'S', {G3b}}, {'E', {G3}},
        {'D', {A3b}}, {'F', {A3}}, {'T', {B3b}}, {'G', {B3}},
        {'Y', {C4}}, {'H', {D4b}}, {'U', {D4}}, {'J', {E4b}},
        {'K', {E4}}, {'O', {F4}}, {'L', {G4b}}, {'P', {G4}},
        {';', {A4b}}, {'Z', {A4}}, {'X', {B4b}}, {'C', {B4}},
        {'V', {C5}}, {'B', {D5b}}, {'N', {D5}}, {'M', {E5b}},
        {'.', {E5}}, {'Z', {F5}}
    };

    juce::ComboBox melodySelector;
    juce::TextButton playButton{ "Play Melody" };
    juce::Label melodyLabel{ "Melody:", "Select Melody:" };

    std::vector<MelodyNote> melody{};
    bool is_playing_melody = false;
    double melody_start_time = 0.0;
    int next_melody_note_index = 0;
public:
    void log(const std::string& message) const {
        std::cout << message << std::endl;
    }

    Synth()
    {
        addAndMakeVisible(melodyLabel);
        addAndMakeVisible(melodySelector);
        addAndMakeVisible(playButton);
        
        // Populate the dropdown with melody options
        melodySelector.addItem("melody_ddlc", 1);
        melodySelector.addItem("melody1", 2);
        melodySelector.addItem("melody2", 3);
        melodySelector.addItem("melody_cinematic", 4);
        melodySelector.addItem("melody_edm", 5);
        melodySelector.addItem("melody_jazz", 6);
        melodySelector.addItem("melody_ambient", 7);
        melodySelector.addItem("melody_funk", 8);
        melodySelector.addItem("melody_classical", 9);
        melodySelector.addItem("melody_minimalist", 10);
        melodySelector.addItem("melody_epic", 11);
        melodySelector.addItem("melody_odyssey", 12);
        melodySelector.addItem("melody_symphony", 13);
        melodySelector.setSelectedId(1); // Default to first melody

        playButton.onClick = [this] {
            // Load the selected melody first
            loadSelectedMelody();
            
            // Start playback
            is_playing_melody = true;
            melody_start_time = juce::Time::getMillisecondCounter() / 1000.0;
            next_melody_note_index = 0;
            log("Melody playback started: " + melodySelector.getText().toStdString());
        };
        
        // Load the default melody
        loadSelectedMelody();

        setAudioChannels(0, 2);
        setWantsKeyboardFocus(true);
        addKeyListener(this);
        constexpr float min_freq = E3;
        constexpr float max_freq = F5;

        float x = 30.0f;
        float y = 30.0f;
        const float size = 70.0f;
        const float padding = 10.0f;
        const float start_hue = 0.7f; // purple
        const float end_hue = 0.04f;  // orange

        for (const auto& pair : note_map)
        {
            VisualNote v;
            v.key_code = pair.first;

            // mapping frequency to hue: lower freq -> darker colour and vice versa
            float hue = juce::jmap(pair.second.frequency, min_freq, max_freq, start_hue, end_hue);
            // making the color based on hue value, saturation(90%), brightness(100%) and opacity(100%)
            v.base_colour = juce::Colour::fromHSV(hue, 0.9f, 1.0f, 1.0f);

            v.bounds = { x, y, size, size };
            visual_notes[v.key_code] = v;

            // next row
            x += size + padding;
            if (x > 700) // wrap-up
            {
                x = 30.0f;
                y += size + padding;
            }
        }
        log("=== Synth Started ===");
        startTimerHz(60);
    }

    ~Synth()
    {
        stopTimer();
        log("=== Synth Shutting Down ===");
        removeKeyListener(this);
        shutdownAudio();
    }

    void startNote(int key_code)
    {
        // is this note already playing?
        for (const auto& voice : active_notes)
            if (voice.is_active && voice.key_code == key_code)
                return;

        auto it = note_map.find(key_code);
        if (it != note_map.end())
        {
            for (auto& voice : active_notes)
            {
                if (!voice.is_active && voice.amplitude == 0.0f)
                {
                    voice.frequency = it->second.frequency;
                    voice.phase = 0.0f;
                    voice.amplitude = 1.0f;
                    voice.is_active = true;
                    voice.key_code = key_code;

                    auto& v = visual_notes[key_code];
                    v.is_lit = true;
                    v.splash_radius = 0.0f;
                    v.splash_opacity = 1.0f;
                    repaint();
                    return;
                }
            }
        }
    }

    void stopNote(int key_code)
    {
        for (auto& voice : active_notes)
        {
            if (voice.is_active && voice.key_code == key_code)
            {
                voice.is_active = false;
                visual_notes[key_code].is_lit = false;
                repaint();
                return;
            }
        }
    }

    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override
    {
        log("Preparing to play...");
        log("Samples per block set to: " + std::to_string(samplesPerBlockExpected));
        sample_rate = newSampleRate;
        log("Sample rate set to: " + std::to_string(sample_rate));
    }

    void releaseResources() override {}

    void loadSelectedMelody()
    {
        int selectedId = melodySelector.getSelectedId();
        
        switch (selectedId)
        {
        case 1: // DDLC Theme
            melody = melody_ddlc;
            // Apply speed multiplier for DDLC
            {
                const double speed_multiplier = 1.5;
                for (auto& note : melody)
                {
                    note.startTimeSecs /= speed_multiplier;
                    note.durationSecs /= speed_multiplier;
                }
            }
            break;
        case 2:
            melody = melody1;
            break;
        case 3:
            melody = melody2;
            break;
        case 4:
            melody = melody_cinematic;
            break;
        case 5:
            melody = melody_edm;
            break;
        case 6:
            melody = melody_jazz;
            break;
        case 7:
            melody = melody_ambient;
            break;
        case 8:
            melody = melody_funk;
            break;
        case 9:
            melody = melody_classical;
            break;
        case 10:
            melody = melody_minimalist;
            break;
        case 11:
            melody = melody_epic;
            break;
        case 12:
            melody = melody_odyssey;
            break;
        case 13:
            melody = melody_symphony;
            break;
        default:
            melody = melody1;
            break;
        }
        log("Loaded melody: " + melodySelector.getText().toStdString() + 
            " (" + std::to_string(melody.size()) + " notes)");
    }

    void resized() override
    {
        // Layout the controls in the bottom right
        int controlWidth = 150;
        int controlHeight = 25;
        int padding = 10;
        int rightMargin = 10;
        int bottomMargin = 10;
        
        int startX = getWidth() - controlWidth - rightMargin;
        int startY = getHeight() - (controlHeight * 3 + padding * 2) - bottomMargin;
        
        // Stack them vertically
        melodyLabel.setBounds(startX, startY, controlWidth, controlHeight);
        melodySelector.setBounds(startX, startY + controlHeight + 5, controlWidth, controlHeight);
        playButton.setBounds(startX, startY + (controlHeight + 5) * 2, controlWidth, controlHeight);
    }

    void timerCallback() override
    {
        // --- 1. Animation Logic (no changes) ---
        bool needs_repaint = false;
        for (auto& pair : visual_notes)
        {
            auto& v = pair.second;
            // If this note has an active splash animation...
            if (v.splash_opacity > 0.0f)
            {
                // ...update its properties.
                v.splash_radius += 1.5f;   // velocity of expansion of the splash
                v.splash_opacity -= 0.02f; // velocity of fading out the splash
                needs_repaint = true;      // to redraw
            }
        }

        if (needs_repaint)
        {
            repaint();
        }

        // --- 2. NEW Melody Playback Logic ---
        if (is_playing_melody)
        {
            const double currentTime = (juce::Time::getMillisecondCounter() / 1000.0) - melody_start_time;

            // Check if it's time to START the next note
            if (next_melody_note_index < melody.size())
            {
                const auto& note = melody[next_melody_note_index];
                if (currentTime >= note.startTimeSecs)
                {
                    startNote(note.keyCode);
                    next_melody_note_index++;
                }
            }

            // Check all notes in the melody to see if it's time to STOP them
            for (const auto& note : melody)
            {
                const double noteEndTime = note.startTimeSecs + note.durationSecs;
                if (currentTime >= noteEndTime && currentTime < noteEndTime + 0.05) // Small window to send "note off"
                {
                    stopNote(note.keyCode);
                }
            }
            
            // Stop playback after the last note has finished
            if (next_melody_note_index >= melody.size() && currentTime > melody.back().startTimeSecs + melody.back().durationSecs + 1.0)
            {
                is_playing_melody = false;
                log("Melody playback finished.");
            }
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        for (const auto& pair : visual_notes)
        {
            const auto& v = pair.second;

            // --- 1. DRAW THE SPLASH EFFECT (UNDERNEATH) ---
            if (v.splash_opacity > 0.0f)
            {
                // Set the color to the note's color but with the current splash opacity
                g.setColour(v.base_colour.withAlpha(v.splash_opacity));
                // Calculate the splash circle's current diameter
                float splashDiameter = v.bounds.getWidth() + v.splash_radius;
                // Draw the splash as an outline, centered on the main circle
                g.drawEllipse(v.bounds.getCentreX() - splashDiameter / 2,
                              v.bounds.getCentreY() - splashDiameter / 2,
                              splashDiameter, splashDiameter, 2.0f);
            }

            // --- 2. DRAW THE MAIN CIRCLE (ON TOP) ---
            if (v.is_lit)
            {
                g.setColour(v.base_colour);
                g.fillEllipse(v.bounds);
                g.setColour(v.base_colour);
            }
            else
            {
                g.setColour(juce::Colours::black);
                g.fillEllipse(v.bounds);
                g.setColour(v.base_colour);
            }
            g.drawEllipse(v.bounds, 3.0f);
        }
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* leftBuffer  = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            float mix_sample = 0.0f;

            for (auto& voice : active_notes)
            {
                if (voice.is_active || voice.amplitude > 0.0f)
                {
                    switch (waveform)
                    {
                    // 1. sine wave (smooth, classic sound)
                    case WaveformType::Sine:
                        mix_sample += voice.amplitude * std::sin(voice.phase);
                        break;
                    // 2. sawtooth wave (bright, classic synth sound)
                    case WaveformType::Sawtooth:
                        mix_sample += voice.amplitude * ((voice.phase / juce::MathConstants<float>::pi) - 1.0f);
                        break;
                    // 3. square wave (buzzy, retro 8-bit sound)
                    case WaveformType::Square:
                        mix_sample += voice.amplitude * ((voice.phase < juce::MathConstants<float>::pi) ? 0.5f : -0.5f);
                        break;
                    // 4. triangle wave (smooth, rising and falling sound)
                    case WaveformType::Triangle:
                        mix_sample += voice.amplitude * (std::abs((voice.phase / juce::MathConstants<float>::pi) - 1.0f) * 2.0f - 1.0f);
                        break;
                    default:
                        mix_sample += voice.amplitude * std::sin(voice.phase); // sine by default
                        break;
                    }

                    // advance the phase for this voice (move the wave forward a tiny bit)
                    voice.phase_delta = (juce::MathConstants<double>::twoPi * voice.frequency / sample_rate);
                    // ensure that we stay in [0, 2π) bound:
                    // returned value of std::fmod(x,y) has the same sign as x and is less than y in magnitude
                    voice.phase = std::fmod(voice.phase + voice.phase_delta, juce::MathConstants<double>::twoPi);

                    if (!voice.is_active)
                    {
                        voice.amplitude -= 0.001f; // fade out volume
                        if (voice.amplitude < 0.0f)
                        {
                            voice.amplitude = 0.0f; // ensure it goes quiet
                        }
                    }
                }
            }

            // Write the final mixed sample to both channels
            leftBuffer[sample]  = mix_sample * speakers_ch_amplitude;
            rightBuffer[sample] = mix_sample * speakers_ch_amplitude;
        }
    }

    bool keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/) override
    {
        const int key_code = key.getKeyCode();

        // is it a control key?
        switch (key_code)
        {
        case 49: // 1
            waveform = WaveformType::Sine;
            log("Waveform set to Sine");
            return true;
        case 50: // 2
            waveform = WaveformType::Sawtooth;
            log("Waveform set to Sawtooth");
            return true;
        case 51: // 3
            waveform = WaveformType::Square;
            log("Waveform set to Square");
            return true;
        case 52: // 4
            waveform = WaveformType::Triangle;
            log("Waveform set to Triangle");
            return true;
        default:
            break;
        }

        startNote(key_code);
        return true;
    }

    bool keyStateChanged(bool isKeyDown, juce::Component* /*originatingComponent*/) override
    {
        if (isKeyDown)
        {
            return false;
        }

        log("keyStateChanged(up) event received.");

        for (auto& voice : active_notes)
        {
            if (voice.is_active)
            {
                // is the key for this specific voice still being held down?
                if (!juce::KeyPress::isKeyCurrentlyDown(voice.key_code))
                {
                    // it's not, the key has been released. fading out
                    stopNote(voice.key_code);
                    //voice.is_active = false;
                    log("Key OFF: '" + std::to_string(voice.key_code) + "' -> Starting fade out.");
                    //visual_notes[voice.key_code].is_lit = false;
                    //repaint();
                }
            }
        }
        return false;
    }
};

class SynthAppWin : public juce::DocumentWindow
{
private:
    Synth synthComponent;

public:
    SynthAppWin() : DocumentWindow("SoundStuff",
                                  juce::Colours::lightpink,
                                  DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(&synthComponent, true);

        setResizable(true, true);
        centreWithSize(780, 400);
        setVisible(true);
        synthComponent.grabKeyboardFocus();
    }

    void resized() override
    {
        DocumentWindow::resized();
    }
    
    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class SynthApp : public juce::JUCEApplication
{
public:
    SynthApp() {}

    const juce::String getApplicationName() override { return "A Synth App"; }
    const juce::String getApplicationVersion() override { return "1.69"; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new SynthAppWin());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<SynthAppWin> mainWindow;
};