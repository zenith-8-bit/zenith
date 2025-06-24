#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // For JSON handling

// Replace with your network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with your OpenRouter API Key
const char* OPENROUTER_API_KEY = "sk-YOUR_OPENROUTER_API_KEY"; 

// OpenRouter.ai API endpoint for chat completions
const char* openrouterHost = "openrouter.ai";
const char* openrouterEndpoint = "/api/v1/chat/completions";

// The root CA certificate for openrouter.ai (or its underlying CDN/proxy)
// You MUST replace this with the actual root CA certificate you extracted.
// This is an example and likely needs to be updated.
// Search for "Root CA certificate for openrouter.ai" if you have trouble extracting.
const char* openrouter_root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgOfc2HU2gKzFu4W4E9zANBgkqhkiG9w0BAQsFADBL
... (rest of your root CA certificate) ...
-----END CERTIFICATE-----
)EOF";


WiFiClientSecure client; // Declare WiFiClientSecure client globally or in main loop

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set the root CA certificate for HTTPS
  client.setCACert(openrouter_root_ca);
}

void loop() {
  // Only make a request when connected to Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nMaking a request to OpenRouter.ai...");

    HTTPClient http;
    http.begin(client, openrouterHost, 443, openrouterEndpoint, true); // Use HTTPS (port 443)

    // Set HTTP headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(OPENROUTER_API_KEY));
    // Optional: for OpenRouter rankings
    http.addHeader("HTTP-Referer", "YOUR_SITE_URL"); 
    http.addHeader("X-Title", "YOUR_APP_NAME");

    // Construct the JSON request body
    // Use ArduinoJson Assistant (https://arduinojson.org/assistant/) to calculate buffer size
    const size_t CAPACITY = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
    StaticJsonDocument<CAPACITY> doc;

    doc["model"] = "deepseek/deepseek-r1:free"; // Or "microsoft/mai-ds-r1:free"
    
    JsonArray messages = doc.createNestedArray("messages");
    JsonObject message1 = messages.createNestedObject();
    message1["role"] = "user";
    message1["content"] = "Tell me a short story about an adventurous cat.";

    String requestBody;
    serializeJson(doc, requestBody);

    Serial.print("Sending JSON: ");
    Serial.println(requestBody);

    // Send the POST request
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode == HTTP_CODE_OK || httpResponseCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println("Response from OpenRouter.ai:");
        Serial.println(payload);

        // Parse the JSON response
        StaticJsonDocument<2048> responseDoc; // Adjust buffer size as needed based on expected response
        DeserializationError error = deserializeJson(responseDoc, payload);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

        String generatedText = responseDoc["choices"][0]["message"]["content"].as<String>();
        Serial.print("Generated Text: ");
        Serial.println(generatedText);
      }
    } else {
      Serial.print("Error sending POST request: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end(); // Free resources
  } else {
    Serial.println("WiFi not connected. Retrying in 5 seconds...");
  }

  delay(5000); // Wait 5 seconds before next request
}
