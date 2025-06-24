// ESP32 Bluetooth Speaker Text-to-Speech Client
// Connects to a Bluetooth speaker and plays a pre-recorded audio sequence.
// For full Text-to-Speech (TTS), audio bytes for the phrase would be pre-generated
// (e.g., from a server or stored on SD card) and placed in the audioData array.

#include "BluetoothA2DP.h" // For Bluetooth A2DP Sink (Speaker) functionality
// Install this library via Arduino IDE Library Manager:
// Search for "Bluetooth A2DP" by pschatzmann

BluetoothA2DP a2dp;

// Define your Bluetooth speaker's name (optional, but good for specific connections)
// If you leave this empty "", it will try to connect to the last connected device or
// allow any device to connect (depending on a2dp.start() variant used).
const char* bluetooth_speaker_name = "CY786"; // e.g., "JBL Flip 5", "MyBTspeaker"

// Placeholder for your pre-generated audio data for the phrase:
// "hello human how are you doing fuck you guys"
// This is a dummy sine wave audio for demonstration.
// YOU MUST REPLACE THIS WITH ACTUAL WAV/PCM AUDIO DATA FOR YOUR PHRASE.
// This data should be raw PCM (e.g., 44.1kHz, 16-bit, mono) suitable for A2DP.
const int AUDIO_SAMPLE_RATE = 44100; // Hz
const int AUDIO_DURATION_SECONDS = 3; // Length of placeholder audio
const int SAMPLE_BITS = 16;
const int NUM_CHANNELS = 1; // Mono

// Function to generate a simple sine wave for demonstration purposes
// In a real TTS scenario, you would load pre-generated audio bytes here.
size_t generate_placeholder_audio(int16_t *buffer, size_t num_samples) {
    static float phase = 0.0;
    float frequency = 440.0; // A4 note for placeholder
    float volume_scale = 0.5; // Reduce volume slightly

    for (size_t i = 0; i < num_samples; ++i) {
        float sample_value = sin(2.0 * PI * frequency * (i / (float)AUDIO_SAMPLE_RATE) + phase) * volume_scale;
        buffer[i] = (int16_t)(sample_value * 32767.0); // Convert float to 16-bit integer
    }
    phase += 2.0 * PI * frequency * (num_samples / (float)AUDIO_SAMPLE_RATE);
    return num_samples;
}

// Global buffer to hold the audio data
// For real phrases, you might need a much larger buffer or stream from flash/SD.
#define MAX_AUDIO_BUFFER_SIZE (AUDIO_SAMPLE_RATE * AUDIO_DURATION_SECONDS * NUM_CHANNELS * (SAMPLE_BITS / 8))
int16_t audio_buffer[MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8)]; // Array of 16-bit samples

// --- A2DP Callback Function (required by the library) ---
// This function is called by the A2DP library when it needs more audio data.
// Here, we provide our pre-generated "speech" data.
size_t write_audio_data(const uint8_t *data, size_t len) {
  // This function is for the A2DP SOURCE mode (ESP32 sending audio).
  // For A2DP SINK (ESP32 receiving audio), you would implement your audio processing here.
  // Since we are *sending* audio to a speaker (acting as source), this part is slightly different.
  // However, the example aims to push pre-generated data out.
  return len; // Indicate that all data was "written" (consumed by A2DP library internally)
}


// A2DP "data_stream_callback" is used when ESP32 is the A2DP SOURCE.
// It generates audio samples and pushes them to the connected sink.
int32_t get_audio_data_callback(int16_t *data, int32_t len) {
    // This callback is invoked by the A2DP library to get audio samples.
    // 'len' is the number of samples requested (not bytes).
    // The data should be written into the 'data' buffer.

    static size_t current_audio_position = 0; // Tracks position in our pre-recorded audio

    // If we have "pre-recorded" data (our placeholder tone), send it
    if (current_audio_position < (MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8))) {
        size_t samples_to_copy = std::min((size_t)len, (MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8)) - current_audio_position);
        memcpy(data, &audio_buffer[current_audio_position], samples_to_copy * sizeof(int16_t));
        current_audio_position += samples_to_copy;
        return samples_to_copy;
    } else {
        // If we've finished playing our "phrase", you might want to stop/restart
        // For this example, we loop the placeholder audio.
        // In a real application, you might stop playing or load next audio segment.
        current_audio_position = 0; // Loop the audio for demonstration
        Serial.println("Finished playing phrase. Looping...");
        return 0; // Return 0 samples if nothing to play (or indicate end)
    }
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting ESP32 Bluetooth A2DP TTS Client...");

  // Initialize the audio buffer with our placeholder audio (simple tone)
  // For a real application, you'd load actual WAV/PCM data here.
  generate_placeholder_audio(audio_buffer, MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8));
  Serial.printf("Placeholder audio generated: %d samples, %d bytes.\n",
                MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8), MAX_AUDIO_BUFFER_SIZE);


  // Set the callback function that provides audio data to the A2DP library
  a2dp.set_stream_reader(get_audio_data_callback);

  // Initialize Bluetooth A2DP Source with the device name and sample rate
  // This makes the ESP32 discoverable and able to connect to a Bluetooth speaker.
  // Ensure your speaker is in pairing mode.
  // Using the device name below allows you to specify the ESP32's Bluetooth name.
  // The second parameter is the sample rate for the audio stream.
  a2dp.start("ESP32_TTS_Player", AUDIO_SAMPLE_RATE);
  // If you want to connect to a specific speaker by its name:
  // a2dp.start(bluetooth_speaker_name, true); // True for auto-reconnect if desired (sink mode)
  // For source mode (ESP32 is sending audio), start with:
  // a2dp.start("ESP32_TTS_Player", AUDIO_SAMPLE_RATE);

  Serial.println("Bluetooth A2DP started. Waiting for connection...");
  Serial.printf("Looking for Bluetooth speaker: %s (or any if name is not set)\n", bluetooth_speaker_name);
}

void loop() {
  // The A2DP library handles audio streaming in the background.
  // You can add other tasks here if needed.
  if (a2dp.is_connected()) {
      // Serial.println("Connected to Bluetooth speaker. Playing audio...");
      // Audio is continuously pushed via the callback.
  } else {
      Serial.println("Waiting for Bluetooth speaker connection...");
      delay(2000); // Wait for connection attempts
  }
}
