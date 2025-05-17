#include <Arduino.h>

// const int pins[] = {0, 2};
// const int numPins = sizeof(pins) / sizeof(pins[0]);

// void setup() {
//     Serial.begin(115200);
//     delay(200);
//     Serial.println("Setup started at 115200 baud");
    
//     for (int i = 0; i < numPins; i++) {
//         pinMode(pins[i], OUTPUT);
//         digitalWrite(pins[i], HIGH);
//     }
// }

// void loop() {
//     digitalWrite(pins[1], LOW);
//     delay(300);
//     digitalWrite(pins[1], HIGH);
//     delay(300);
// }

/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/
#include "ESP8266WiFi.h"

void setup() {
  Serial.begin(115200);
  delay(200);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("Setup done");
}

void loop() {
  Serial.println("scan SB");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  // Wait a bit before scanning again
  delay(2000);
}
