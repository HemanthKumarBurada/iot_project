#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//--- Wi-Fi Details ---
const char* ssid = "They_call_me_OG";
const char* password = ""; // No password

//--- Twitter API v2 Details ---
// ⚠️ IMPORTANT: Replace with your own valid Bearer Token.
const char* bearerToken = "AAAAAAAAAAAAAAAAAAAAALp14AEAAAAA6Vao8CRrT5Wl6x7xScBX%2FwkIEg4%3DBb8hL2vSYzpywUEZztkgCulVoubVRPFtaqtBCKxMB2TKFzOpPu";

// The user ID for the Twitter handle @HBurad8
const char* twitter_user_id = "1966408269611864064";

//--- Hardware Pin ---
// The LED will be connected to GPIO 2 (the built-in LED on many boards)
const int ledPin = 2;

// Function to connect to Wi-Fi
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to fetch tweets and control the LED
void getTweetsAndControlLed() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String apiUrl = "https://api.twitter.com/2/users/" + String(twitter_user_id) + "/tweets";

    http.begin(apiUrl);
    http.addHeader("Authorization", "Bearer " + String(bearerToken));
    Serial.println("Fetching tweets...");
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          http.end();
          return;
        }

        JsonArray tweets = doc["data"];

        if (tweets.isNull() || tweets.size() == 0) {
          Serial.println("✅ API call successful, but no recent tweets were found.");
          digitalWrite(ledPin, LOW); // Turn LED off if no tweets are found
        } else {
          // Get the very first tweet, which is the most recent one
          JsonObject latestTweet = tweets[0];
          const char* tweet_text = latestTweet["text"];

          Serial.println("--- Latest Tweet ---");
          Serial.println(tweet_text);

          // Check for the hashtag and control the LED
          if (String(tweet_text).indexOf("#ledon") != -1) {
            Serial.println("Hashtag #ledon FOUND! Turning LED ON. 💡");
            digitalWrite(ledPin, HIGH); // Turn LED ON
          } else {
            Serial.println("Hashtag #ledon not found. Turning LED OFF. ⚫");
            digitalWrite(ledPin, LOW); // Turn LED OFF
          }
        }

      } else {
        Serial.printf("HTTP request failed with error: %s\n", http.errorToString(httpCode).c_str());
      }
    } else {
      Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set up the LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with the LED off

  connectToWiFi();
}

void loop() {
  // Fetch tweets and control the LED
  getTweetsAndControlLed();

  // Wait for 1 minute before fetching again to avoid being rate-limited
  Serial.println("\nWaiting for 1 minute before next fetch...");
  delay(65000); // 60,000 milliseconds = 1 minute
}
