# Synth

### main features
- notes mapped to keyboard keys and pitches (E3 to F5)
- visual feedback with color-coded keys and animation
- real-time audio synthesis with polyphonic support (up to 10 simultaneous notes)

### waveforms
- **sine** (Key: 1) - Smooth, classic sound
- **sawtooth** (Key: 2) - Bright, classic synth sound  
- **square** (Key: 3) - Buzzy, retro 8-bit sound
- **triangle** (Key: 4) - Smooth, rising and falling sound

### interface
- color-coded keys based on frequency (purple to orange gradient)
- animated splash effects when notes are played
- dropdown melody selector

## key mapping (e3-f5) - subject to change

| Key | Note | Key | Note | Key | Note | Key | Note |
|-----|------|-----|------|-----|------|-----|------|
| A | E3 | W | F3 | S | G♭3 | E | G3 |
| D | A♭3 | F | A3 | T | B♭3 | G | B3 |
| Y | C4 | H | D♭4 | U | D4 | J | E♭4 |
| K | E4 | O | F4 | L | G♭4 | P | G4 |
| ; | A♭4 | Z | A4 | X | B♭4 | C | B4 |
| V | C5 | B | D♭5 | N | D5 | M | E♭5 |

## architecture

- **`Synth`** - main synthesizer component handling audio and UI
- **`Note`** - audio voice structure with frequency, phase, and amplitude
- **`VisualNote`** - visual representation with color and animation
- **`MelodyNote`** - timed note sequences for playback
- **`Real-time Audio Processing`** - low-latency synthesis using JUCE's audio callback system

## tech

- **framework**: JUCE 8.0.10
- **language**: C++20
- **build system**: CMake (minimum 3.28)
- **platform**: cross-platform (Windows, macOS, Linux)