#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "yourSSID";
const char* password = "yourPassword";
String phoneNumber = "yourPhoneNumber";
String apiKey = "yourApiKey";

void setup() {
Serial.begin(115200);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(1000);
Serial.println("Connecting to WiFi...");
}
Serial.println("Connected!");
}

void loop() {
if (WiFi.status() == WL_CONNECTED) {
HTTPClient http;
String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&text=Motion+Detected!&apikey=" + apiKey;
http.begin(url);
int httpCode = http.GET();
if (httpCode == 200) {
Serial.println("Message sent!");
} else {
Serial.println("Error: " + String(httpCode));
}
http.end();
}
delay(60000); // Check every minute
}
