'''
audio settings
Sad Mood:

base_frequency=700.0: A lower base pitch to convey a heavier, more somber tone.
pitch_bend_range=200.0: Reduced pitch variation, making the sound less erratic and more subdued.
warble_rate=3.0: Slower "dribble" or warble, contributing to a feeling of slowness or dejection.
warble_depth=0.05: Less pronounced warble, making it less energetic.
harmonics_strength=0.1: Lower harmonic strength for a less bright, more muted sound.
noise_factor=0.02: Slightly increased noise for a touch of static or "weariness."
bass_factor=0.1: A noticeable bass undertone to add to the "heavy" or "gloomy" feel.

Happy Mood:

base_frequency=1500.0: A higher, more cheerful base pitch.
pitch_bend_range=800.0: Increased pitch variation for a more lively and expressive sound.
warble_rate=15.0: Faster "dribble" for a more energetic and bubbly feel.
warble_depth=0.2: More pronounced warble.
harmonics_strength=0.3: Higher harmonic strength for a richer, more "sparkling" sound.
noise_factor=0.005: Very low noise, indicating a clear, positive sound.
bass_factor=0.0: No added bass, keeping the sound lighter.

Excited Mood (Newly Added):

base_frequency=1800.0: The highest base pitch for a very enthusiastic and high-energy sound.
pitch_bend_range=1000.0: The most dramatic pitch shifts, mimicking rapid, excited vocalizations.
warble_rate=20.0: The fastest and most intense "dribble," conveying high excitement or rapid communication.
warble_depth=0.25: The deepest warble, making the pitch fluctuations very noticeable.
harmonics_strength=0.4: The highest harmonic strength for a bright, almost shrill, and very metallic sound.
noise_factor=0.01: A small amount of noise, keeping it clean but still electronic.
bass_factor=0.0: No added bass to maintain a light and agile feel.
'''

import numpy as np
import pyaudio
import time
import math

# --- Audio Parameters ---
SAMPLE_RATE = 44100  # samples per second
DURATION = 1.0       # seconds of sound
VOLUME = 0.5         # 0.0 to 1.0

# --- R2D2 Voice Customization Parameters ---
# These parameters will be mapped to moods later
class R2D2Params:
    def __init__(self,
                 base_frequency=1000.0,  # Base pitch of the sound (Hz)
                 pitch_bend_range=500.0, # How much the pitch can bend/vary (Hz)
                 warble_rate=10.0,       # Frequency of the pitch warble/dribble (Hz)
                 warble_depth=0.1,       # Depth of the pitch warble (0.0 to 1.0)
                 harmonics_strength=0.2, # Strength of additional harmonics for richness
                 noise_factor=0.01,      # Adds a bit of static/sand (0.0 to 1.0)
                 bass_factor=0.0         # Adds a lower frequency undertone (0.0 to 1.0)
                ):
        self.base_frequency = base_frequency
        self.pitch_bend_range = pitch_bend_range
        self.warble_rate = warble_rate
        self.warble_depth = warble_depth
        self.harmonics_strength = harmonics_strength
        self.noise_factor = noise_factor
        self.bass_factor = bass_factor

# --- Mood-to-Parameter Mapping Examples ---
# You can define different parameter sets for various moods.
# These are just examples; experiment to find what sounds best!
MOOD_PRESETS = {
    "neutral": R2D2Params(), # Default values

    "happy": R2D2Params(
        base_frequency=1500.0,
        pitch_bend_range=800.0,
        warble_rate=15.0,
        warble_depth=0.2,
        harmonics_strength=0.3,
        noise_factor=0.005 # Less noise for happy
    ),
    "sad": R2D2Params(
        base_frequency=700.0,
        pitch_bend_range=200.0,
        warble_rate=3.0,
        warble_depth=0.05, # Slower, less pronounced warble
        harmonics_strength=0.1,
        noise_factor=0.02,
        bass_factor=0.1 # A bit more bass for a 'heavy' feel
    ),
    "cry": R2D2Params( # More exaggerated sad
        base_frequency=600.0,
        pitch_bend_range=300.0,
        warble_rate=2.0,
        warble_depth=0.1,
        harmonics_strength=0.05,
        noise_factor=0.03,
        bass_factor=0.2
    ),
    "tired": R2D2Params(
        base_frequency=800.0,
        pitch_bend_range=150.0,
        warble_rate=5.0,
        warble_depth=0.08,
        harmonics_strength=0.15,
        noise_factor=0.015,
        bass_factor=0.05
    )
}

def generate_r2d2_sound(params: R2D2Params, duration=DURATION, sample_rate=SAMPLE_RATE):
    """
    Generates an R2D2-like sound based on the given parameters.
    """
    t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)

    # Base frequency with a subtle, non-repeating random pitch bend
    # This gives it a more "electronic" and less perfectly musical feel
    pitch_mod = params.pitch_bend_range * (np.sin(2 * np.pi * t * (1.0 / (duration * 0.7))) + \
                                          0.5 * np.sin(2 * np.pi * t * (1.0 / (duration * 0.4))))
    current_freq = params.base_frequency + pitch_mod

    # Add warble (dribble) effect: rapid frequency modulation
    warble = params.warble_depth * np.sin(2 * np.pi * t * params.warble_rate) * params.base_frequency
    current_freq += warble

    # Generate the primary waveform (sine wave)
    waveform = np.sin(2 * np.pi * current_freq * t)

    # Add higher harmonics for a richer, more metallic sound
    for i in range(2, 5): # Add 2nd, 3rd, 4th harmonics
        waveform += params.harmonics_strength * np.sin(2 * np.pi * (current_freq * i) * t)

    # Add a lower frequency tone for "bass"
    if params.bass_factor > 0:
        bass_freq = params.base_frequency / 4.0 # A sub-octave or lower
        waveform += params.bass_factor * np.sin(2 * np.pi * bass_freq * t)

    # Add random noise for a "sandy" or static effect
    noise = params.noise_factor * (np.random.rand(len(t)) * 2 - 1) # Random values between -1 and 1
    waveform += noise

    # Normalize and apply volume
    audio = waveform * VOLUME / np.max(np.abs(waveform)) # Normalize to avoid clipping
    return audio.astype(np.float32)

def play_sound(audio_data, sample_rate=SAMPLE_RATE):
    """
    Plays the given audio data.
    """
    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paFloat32,
                    channels=1,
                    rate=sample_rate,
                    output=True)

    stream.write(audio_data.tobytes())

    stream.stop_stream()
    stream.close()
    p.terminate()

if __name__ == "__main__":
    print("R2D2 Voice Synthesizer")
    print("----------------------")
    print("Press Enter to play the current sound. Type 'exit' to quit.")
    print("Available moods:", ", ".join(MOOD_PRESETS.keys()))
    print("You can type a mood name (e.g., 'happy') to change parameters.")
    print("You can also type 'custom' to manually set parameters.")

    current_params = MOOD_PRESETS["neutral"]

    while True:
        user_input = input(f"\nCurrent Mood: {current_params.__class__.__name__} (or 'custom' / mood name / 'exit'): ").strip().lower()

        if user_input == 'exit':
            break
        elif user_input in MOOD_PRESETS:
            current_params = MOOD_PRESETS[user_input]
            print(f"Switched to '{user_input}' mood preset.")
        elif user_input == 'custom':
            print("\nEnter custom parameters (press Enter for default, or type a number):")
            try:
                base_freq = float(input(f"  Base Frequency (Hz, current={current_params.base_frequency}): ") or current_params.base_frequency)
                pitch_bend = float(input(f"  Pitch Bend Range (Hz, current={current_params.pitch_bend_range}): ") or current_params.pitch_bend_range)
                warble_rate = float(input(f"  Warble Rate (Hz, current={current_params.warble_rate}): ") or current_params.warble_rate)
                warble_depth = float(input(f"  Warble Depth (0.0-1.0, current={current_params.warble_depth}): ") or current_params.warble_depth)
                harmonics = float(input(f"  Harmonics Strength (0.0-1.0, current={current_params.harmonics_strength}): ") or current_params.harmonics_strength)
                noise_f = float(input(f"  Noise Factor (0.0-1.0, current={current_params.noise_factor}): ") or current_params.noise_factor)
                bass_f = float(input(f"  Bass Factor (0.0-1.0, current={current_params.bass_factor}): ") or current_params.bass_factor)

                current_params = R2D2Params(
                    base_frequency=base_freq,
                    pitch_bend_range=pitch_bend,
                    warble_rate=warble_rate,
                    warble_depth=warble_depth,
                    harmonics_strength=harmonics,
                    noise_factor=noise_f,
                    bass_factor=bass_f
                )
                print("Custom parameters set.")
            except ValueError:
                print("Invalid input. Keeping current parameters.")
        else:
            print("Invalid input. Please type a mood name, 'custom', or 'exit'.")
            continue

        print("Generating sound...")
        audio_output = generate_r2d2_sound(current_params)
        print("Playing sound...")
        play_sound(audio_output)
        time.sleep(0.1) # Small delay to ensure sound finishes
