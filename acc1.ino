#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// ADXL345 object
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Data structure to send (only X/Y tilt)
typedef struct struct_message {
  float tiltX;  // roll
  float tiltY;  // pitch
} struct_message;

struct_message angleData;

// Receiver ESP32 MAC address (update this!)
uint8_t receiverMac[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};

void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

bool initADXL() {
  if (!accel.begin(0x53)) {
    Serial.println("No ADXL345 detected!");
    return false;
  }

  // Optional: set range to ±2g for best resolution. [web:40]
  accel.setRange(ADXL345_RANGE_2_G);

  return true;
}

void computeTilt() {
  sensors_event_t event;
  accel.getEvent(&event);   // gives acceleration in m/s^2 on x,y,z [web:40]

  // Convert to "g" units (approx). [web:18]
  const float g = 9.80665;
  float Ax = event.acceleration.x / g;
  float Ay = event.acceleration.y / g;
  float Az = event.acceleration.z / g;

  // Standard accelerometer tilt formulas (static tilt). [web:18][web:19]
  float roll  = atan2(Ay, sqrt(Ax * Ax + Az * Az)) * 180.0 / PI;
  float pitch = atan2(-Ax, sqrt(Ay * Ay + Az * Az)) * 180.0 / PI;

  angleData.tiltX = roll;
  angleData.tiltY = pitch;
  Serial.println(roll);
  Serial.println(pitch);


}

void setup() {
  Serial.begin(115200);
  Wire.begin();  // SDA=21, SCL=22 by default on ESP32. [web:22][web:25]
  delay(1000);

  if (!initADXL()) {
    while (1) {
      delay(1000);
    }
  }

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  computeTilt();
  //esp_now_send(receiverMac, (uint8_t *)&angleData, sizeof(angleData));
  delay(50); // ~20 Hz
}
