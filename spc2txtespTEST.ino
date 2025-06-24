// ESP32 Speech-to-Text Client
// Connects to Wi-Fi, makes an HTTP POST request to a Flask server
// with audio data, and processes the transcription response.

#include <WiFi.h> // For ESP32 Wi-Fi connectivity
#include <HTTPClient.h> // For making HTTP requests
#include <ArduinoJson.h> // For parsing JSON responses (install via Arduino IDE Library Manager)

// -------------- Wi-Fi Configuration --------------
const char* ssid = "YOUR_WIFI_SSID";         // Replace with your Wi-Fi SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your Wi-Fi Password

// -------------- Flask Server Configuration --------------
// IMPORTANT: Replace with the actual IP address of the machine running your Flask server.
// Use 'ipconfig' (Windows) or 'ifconfig'/'ip a' (Linux/macOS) to find your computer's IP.
const char* serverAddress = "192.168.1.100"; // Example: Replace with your PC's IP address
const int serverPort = 5000;
const char* serverPath = "/upload_audio";

// Placeholder for your audio data.
// In a real application, you would read audio samples from your microphone (e.g., ADC or I2S),
// process them (e.g., resample, add WAV header), and put the byte data here.
// For demonstration, this is a small dummy byte array.
// YOU WILL NEED TO REPLACE THIS WITH ACTUAL AUDIO FILE BYTES (e.g., a short WAV file).
// Example: If you have a 'sample.wav' file, you could convert it to a byte array
// using Python or an online tool.
const unsigned char audioData[] = {
  // Dummy audio data - REPLACE THIS WITH YOUR ACTUAL AUDIO FILE BYTES
  // This array should contain the raw bytes of your audio file (e.g., a WAV file).
  // For a real scenario, you'd likely stream this from an SD card or generate it on the fly.
  0x52, 0x49, 0x46, 0x46, 0x24, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x40, 0x1f, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00,
  0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00,
  // Add more audio bytes here. This is a very small, invalid WAV header as an example.
  // A real WAV file will be much larger.
};
const size_t audioDataSize = sizeof(audioData);
const char* audioFileName = "recording.wav"; // Name of the file as seen by the server

// Function to connect to Wi-Fi
void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    retries++;
    if (retries > 30) { // Try for 30 seconds
      Serial.println("\nFailed to connect to WiFi. Please check credentials and try again.");
      ESP.restart(); // Restart ESP32 to retry connection
    }
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200); // Initialize serial communication
  delay(1000); // Small delay for serial to initialize
  connectToWiFi(); // Connect to Wi-Fi at startup
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    connectToWiFi(); // Reconnect if Wi-Fi drops
  }

  // Delay before sending the next request (adjust as needed)
  // For continuous microphone input, you'd trigger this based on audio buffer fullness
  delay(5000);

  Serial.println("\nAttempting to send audio to Flask server...");

  HTTPClient http;
  http.begin(serverAddress, serverPort, serverPath); // Specify your Flask server's IP and port

  // Set headers for multipart/form-data
  String boundary = "---011000010111000001101001"; // A random unique boundary string
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  // Construct the body of the POST request
  String requestBody = "";
  requestBody += "--" + boundary + "\r\n";
  requestBody += "Content-Disposition: form-data; name=\"audio\"; filename=\"" + String(audioFileName) + "\"\r\n";
  requestBody += "Content-Type: application/octet-stream\r\n"; // Or appropriate audio MIME type
  requestBody += "\r\n";
  // The actual audio data bytes will be inserted here.

  String footer = "\r\n--" + boundary + "--\r\n";

  // Calculate total content length
  int totalContentLength = requestBody.length() + audioDataSize + footer.length();

  http.sendRequest("POST", (const uint8_t*)requestBody.c_str(), requestBody.length(),
                   (const uint8_t*)audioData, audioDataSize,
                   (const uint8_t*)footer.c_str(), footer.length());

  int httpResponseCode = http.GET(); // Use GET to initiate the request after preparing body

  // Check for HTTP response code
  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    String payload = http.getString();
    Serial.println("Server Response:");
    Serial.println(payload);

    // Parse JSON response
    StaticJsonDocument<256> doc; // Adjust size as per your expected JSON response
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      Serial.println("Something went wrong, let me diagnose. JSON parsing failed.");
    } else {
      bool success = doc["success"].as<bool>();
      if (success) {
        String transcribedText = doc["text"].as<String>();
        Serial.print("Transcription: ");
        Serial.println(transcribedText);
      } else {
        String errorMsg = doc["error"].as<String>();
        Serial.print("Error: ");
        Serial.println(errorMsg);
        Serial.println("Talk again, couldn't identify.");
      }
    }
  } else {
    Serial.printf("Error code: %d\n", httpResponseCode);
    Serial.println("Something went wrong, let me diagnose. Server timeout or connection error.");
    Serial.println(http.errorToString(httpResponseCode)); // Print error description
  }

  http.end(); // Free resources
}
