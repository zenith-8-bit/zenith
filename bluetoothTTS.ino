// ESP32 Bluetooth Speaker Text-to-Speech Client (using direct IDF APIs)
// This sketch demonstrates Bluetooth A2DP Source functionality using the
// ESP-IDF's native Bluetooth APIs directly, as an alternative to the
// 'BluetoothA2DP' library. This approach is significantly more complex.

#include "esp_bt.h"           // Bluetooth common types
#include "esp_bt_main.h"      // Bluetooth controller initialization
#include "esp_gap_bt_api.h"   // Bluetooth GAP (Generic Access Profile) for discovery/pairing
#include "esp_a2d_source.h"   // Bluetooth A2DP Source profile
#include "esp_log.h"          // ESP-IDF logging utility
#include "freertos/FreeRTOS.h" // FreeRTOS for tasks/queues
#include "freertos/task.h"    // FreeRTOS task management
#include "freertos/queue.h"   // FreeRTOS queue management

// Define a TAG for logging messages from this module
static const char *TAG = "A2DP_SOURCE";

// --- Bluetooth Speaker Connection Configuration ---
// Define your Bluetooth speaker's MAC address if known for direct connection.
// If set to all zeros, the ESP32 will attempt discovery.
// You can use a Bluetooth scanner app on your phone to find your speaker's MAC.
static esp_bd_addr_t peer_bd_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Example: {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
// Define your Bluetooth speaker's name. This will be used during discovery.
// Replace "YOUR_BT_SPEAKER_NAME" with the actual name of your speaker (e.g., "JBL Flip 5").
static const char* bluetooth_speaker_name = "CY786";

// --- State Variables for A2DP Source ---
static bool is_connected = false;     // True if connected to a Bluetooth speaker
static bool start_playing = false;    // Flag to start/stop audio streaming

// --- Audio Data Placeholder ---
// This section defines the properties and buffer for the audio data to be played.
// For the phrase "hello human how are you doing fuck you guys", you would need to:
// 1. Use a Text-to-Speech (TTS) service (e.g., Google Cloud TTS) to generate
//    the audio as a WAV (PCM, 44.1kHz, 16-bit, mono) file.
// 2. Convert that WAV file into a C/C++ byte array (many online tools can do this).
// 3. Replace the `audio_buffer` content with your generated data.
//
// NOTE: The phrase "fuck you guys" is included as per your request for TTS.
// Please be mindful of the content you choose to generate and play.
const int AUDIO_SAMPLE_RATE = 44100; // Hz (Standard for A2DP)
const int AUDIO_DURATION_SECONDS = 3; // Length of current placeholder audio (in seconds)
const int SAMPLE_BITS = 16;         // 16-bit audio
const int NUM_CHANNELS = 1;         // Mono audio

// Calculate the maximum size of the audio buffer in bytes
#define MAX_AUDIO_BUFFER_SIZE (AUDIO_SAMPLE_RATE * AUDIO_DURATION_SECONDS * NUM_CHANNELS * (SAMPLE_BITS / 8))
// Global buffer to hold the audio data (array of 16-bit signed integers)
int16_t audio_buffer[MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8)];
static size_t current_audio_position = 0; // Tracks the current position within the audio_buffer

/**
 * @brief Generates a simple sine wave for demonstration purposes.
 * In a real TTS scenario, you would load pre-generated audio bytes here.
 * @param buffer Pointer to the 16-bit audio buffer.
 * @param num_samples Number of samples to generate.
 * @return Number of samples generated.
 */
size_t generate_placeholder_audio(int16_t *buffer, size_t num_samples) {
    static float phase = 0.0;
    float frequency = 440.0; // A4 note (440 Hz) for the sine wave placeholder
    float volume_scale = 0.5; // Reduce volume slightly to prevent clipping

    for (size_t i = 0; i < num_samples; ++i) {
        // Generate a sine wave sample
        float sample_value = sin(2.0 * PI * frequency * (i / (float)AUDIO_SAMPLE_RATE) + phase) * volume_scale;
        // Convert float sample to 16-bit integer format expected by A2DP
        buffer[i] = (int16_t)(sample_value * 32767.0);
    }
    // Update phase for continuous waveform if called repeatedly
    phase += 2.0 * PI * frequency * (num_samples / (float)AUDIO_SAMPLE_RATE);
    return num_samples;
}

/**
 * @brief A2DP Source Data Callback function.
 * This is called by the A2DP source to request audio data to be sent to the sink.
 * @param data Pointer to the buffer where audio data should be placed.
 * @param len The number of bytes requested by the A2DP stack.
 * @return The number of bytes actually written to the buffer.
 */
int32_t a2d_data_cb(uint8_t *data, int32_t len) {
    if (!start_playing) {
        return 0; // Return 0 bytes if not in playing state
    }

    size_t samples_requested = len / (SAMPLE_BITS / 8); // Convert bytes to samples

    // Check if we have enough data remaining in our buffer
    if (current_audio_position < (MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8))) {
        size_t samples_to_copy = std::min(samples_requested, (MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8)) - current_audio_position);
        // Copy audio data from our buffer to the A2DP buffer
        memcpy(data, &audio_buffer[current_audio_position], samples_to_copy * sizeof(int16_t));
        current_audio_position += samples_to_copy; // Advance our position
        return samples_to_copy * (SAMPLE_BITS / 8); // Return the number of bytes written
    } else {
        // If we reached the end of our audio buffer, loop back to the beginning
        current_audio_position = 0;
        ESP_LOGI(TAG, "Finished playing phrase. Looping...");
        return 0; // Return 0 bytes to indicate no more data for this call
    }
}

/**
 * @brief A2DP Source Event Callback function.
 * Handles various A2DP connection and audio state changes.
 * @param event The A2DP callback event type.
 * @param param Parameters associated with the event.
 */
void a2d_source_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT: {
            uint8_t *bda = param->conn_stat.remote_bda; // Bluetooth Device Address
            ESP_LOGI(TAG, "A2DP connection state: %s, [%02x:%02x:%02x:%02x:%02x:%02x]",
                     param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED ? "connected" : "disconnected",
                     bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
            if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
                is_connected = true;
                start_playing = true; // Start playing audio once connected
            } else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
                is_connected = false;
                start_playing = false;
                // If disconnected, you might want to restart discovery or reconnect
                ESP_LOGI(TAG, "Disconnected. Restarting discovery...");
                esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_UNLIMITED_LIAC, 10);
            }
            break;
        }
        case ESP_A2D_AUDIO_STATE_EVT: {
            ESP_LOGI(TAG, "A2DP audio state: %s",
                     param->audio_stat.state == ESP_A2D_AUDIO_STATE_STARTED ? "started" :
                     param->audio_stat.state == ESP_A2D_AUDIO_STATE_STOPPED ? "stopped" : "idle");
            break;
        }
        case ESP_A2D_AUDIO_CFG_EVT: {
            ESP_LOGI(TAG, "A2DP audio stream configuration, codec type %d", param->audio_cfg.codec_type);
            // ESP_A2D_MCT_SBC is the standard codec for A2DP
            if (param->audio_cfg.codec_type == ESP_A2D_MCT_SBC) {
                // You can inspect the SBC codec information if needed
                ESP_LOGI(TAG, "SBC codec info: 0x%x, 0x%x, 0x%x, 0x%x",
                         param->audio_cfg.codec_info[0], param->audio_cfg.codec_info[1],
                         param->audio_cfg.codec_info[2], param->audio_cfg.codec_info[3]);
            }
            break;
        }
        default:
            ESP_LOGI(TAG, "A2DP event: %d", event);
            break;
    }
}

/**
 * @brief Bluetooth GAP (Generic Access Profile) Event Callback function.
 * Handles events related to device discovery, pairing, etc.
 * @param event The GAP callback event type.
 * @param param Parameters associated with the event.
 */
void esp_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
        case ESP_BT_GAP_DISC_RES_EVT: { // Discovery result event
            for (int i = 0; i < param->disc_res.num_prop; i++) {
                if (param->disc_res.prop[i].type == ESP_BT_GAP_DISC_PROP_BOD) {
                    uint8_t *bda = param->disc_res.bda; // MAC address of discovered device
                    ESP_LOGI(TAG, "Found device: [%02x:%02x:%02x:%02x:%02x:%02x]",
                             bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);

                    // If a specific speaker name is provided, check if this is our target
                    if (strlen(bluetooth_speaker_name) > 0 &&
                        param->disc_res.prop[i].type == ESP_BT_GAP_DISC_PROP_BDNAME &&
                        strcmp((const char*)param->disc_res.prop[i].val, bluetooth_speaker_name) == 0) {
                        
                        memcpy(peer_bd_addr, bda, ESP_BD_ADDR_LEN); // Store MAC address
                        ESP_LOGI(TAG, "Found target speaker '%s', attempting to connect...", bluetooth_speaker_name);
                        esp_a2d_source_connect(peer_bd_addr); // Initiate A2DP connection
                        esp_bt_gap_cancel_discovery(); // Stop discovery once target found
                    }
                }
            }
            break;
        }
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: { // Discovery state changed event
            ESP_LOGI(TAG, "Discovery state changed: %s",
                     param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED ? "started" : "stopped");
            break;
        }
        case ESP_BT_GAP_AUTH_CMPL_EVT: { // Authentication complete event
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Authentication success: %s", param->auth_cmpl.device_name);
            } else {
                ESP_LOGE(TAG, "Authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        case ESP_BT_GAP_SET_REMOTE_BOND_DEV_NAME_EVT: { // Remote device name set event
            ESP_LOGI(TAG, "Set remote device name: %s", param->set_rmt_dev_name.name);
            break;
        }
        default: {
            ESP_LOGI(TAG, "GAP event: %d", event);
            break;
        }
    }
}

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  delay(1000); // Small delay to allow serial to initialize
  Serial.println("Starting ESP32 Bluetooth A2DP TTS Client (using direct IDF APIs)...");

  // Initialize the audio buffer with our placeholder audio (simple tone)
  generate_placeholder_audio(audio_buffer, MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8));
  Serial.printf("Placeholder audio generated: %d samples, %d bytes.\n",
                MAX_AUDIO_BUFFER_SIZE / (SAMPLE_BITS / 8), MAX_AUDIO_BUFFER_SIZE);

  // --- Initialize Bluetooth Controller ---
  // The BT controller handles the low-level radio operations.
  if (!esp_bt_controller_init()) {
    ESP_LOGE(TAG, "Failed to initialize Bluetooth controller");
    return; // Halt if initialization fails
  }
  // Enable the Bluetooth controller in BTDM (Classic Bluetooth + BLE) mode.
  if (!esp_bt_controller_enable(ESP_BT_MODE_BTDM)) {
    ESP_LOGE(TAG, "Failed to enable Bluetooth controller");
    return;
  }

  // --- Initialize Bluedroid (Bluetooth Host Stack) ---
  // Bluedroid provides higher-level Bluetooth profiles like A2DP.
  if (!esp_bluedroid_init()) {
    ESP_LOGE(TAG, "Failed to initialize Bluedroid");
    return;
  }
  if (!esp_bluedroid_enable()) {
    ESP_LOGE(TAG, "Failed to enable Bluedroid");
    return;
  }

  // --- Register Callbacks ---
  // Register the GAP callback for handling discovery and other GAP events.
  if (esp_bt_gap_register_callback(esp_gap_cb) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to register GAP callback");
      return;
  }

  // Set the Bluetooth device name that other devices will see during discovery.
  esp_bt_dev_set_device_name("ESP32_TTS_Player_IDF");

  // --- Initialize A2DP Source ---
  // Initialize the A2DP Source profile.
  if (esp_a2d_source_init() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize A2DP source");
    return;
  }
  // Register the A2DP event callback for connection/audio state changes.
  if (esp_a2d_source_register_callback(a2d_source_cb) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register A2DP source callback");
    return;
  }
  // Register the A2DP data callback, which will provide audio samples.
  if (esp_a2d_source_register_data_callback(a2d_data_cb) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register A2DP source data callback");
    return;
  }

  // --- Start Bluetooth Discovery ---
  // Set the scan mode to connectable and general discoverable.
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
  // Start the discovery process for 10 seconds to find nearby Bluetooth speakers.
  // If your speaker's MAC is hardcoded in peer_bd_addr, you might try a direct connect here.
  ESP_LOGI(TAG, "Starting Bluetooth discovery (duration: 10s)...");
  esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_UNLIMITED_LIAC, 10);

  Serial.println("Bluetooth stack initialized. Attempting to connect to speaker...");
}

void loop() {
  // In this direct IDF approach, most of the Bluetooth and audio streaming logic
  // runs in the background via FreeRTOS tasks and callbacks managed by the ESP-IDF.
  // The loop can be used for other application tasks or simple connection monitoring.
  if (is_connected && start_playing) {
      Serial.println("Connected and playing audio...");
  } else {
      Serial.println("Waiting for connection or discovery in progress...");
  }
  delay(2000); // Small delay to prevent busy-looping in the main loop
}
