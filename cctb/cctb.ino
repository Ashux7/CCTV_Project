#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define irPin             12
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid = "IIT_Bhilai";
const char* password = "iitbhilai2023";
const char* discord_webhook = "https://discord.com/api/webhooks/1478363014175527045/6-O2e2QgeBu2ht4PXbzRjpsT5LwF8o7dcFGX3N3yZ_fOffqYcPTwHOzUk5aSdPM9rJa0";

void setup() {
  pinMode(33,OUTPUT); // cam red led
  pinMode(4,OUTPUT); // flashlight
  pinMode(irPin,INPUT_PULLUP); //cam ka gpio pin 12 pr IR ka output lga kr send krenge
  Serial.begin(9600);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA; // 640x480 - Best for Discord
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // esp32cam.setup().vga().highQuality();
}

void sendToDiscord(camera_fb_t *fb) {
  WiFiClientSecure client;
  client.setInsecure();
  // client.setTimeout(10); // 10 second timeout

  if (!client.connect("discord.com", 443)) {
    Serial.println("Connection to Discord failed!");
    return;
  }

  String boundary = "----ESP32CamBoundary";
  String head = "--" + boundary + "\r\n"
                "Content-Disposition: form-data; name=\"file\"; filename=\"capture.jpg\"\r\n"
                "Content-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";

  uint32_t totalLen = head.length() + fb->len + tail.length();

  // Send HTTP headers
  client.println("POST " + String(discord_webhook) + " HTTP/1.1");
  client.println("Host: discord.com");
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println("Content-Length: " + String(totalLen));
  client.println("Connection: close");
  client.println();

  // Send multipart head
  client.print(head);

  // Stream the image in small chunks — no extra malloc needed!
  size_t sent = 0;
  const size_t chunkSize = 1024;  // send 1KB at a time
  while (sent < fb->len) {
    size_t toSend = min(chunkSize, fb->len - sent);
    size_t written = client.write(fb->buf + sent, toSend);
    if (written == 0) {
      Serial.println("Write failed, connection lost");
      client.stop();
      return;
    }
    sent += written;
  }

  // Send multipart tail
  client.print(tail);

  // Read response
  Serial.println("Waiting for Discord response...");
  unsigned long timeout = millis();
  while (!client.available()) {
    if (millis() - timeout > 10000) {
      Serial.println("Response timeout!");
      client.stop();
      return;
    }
    delay(10);
  }

  // Print just the status line
  String statusLine = client.readStringUntil('\n');
  Serial.println("Discord: " + statusLine);

  client.stop();  // close connection and free all socket memory ✅
  Serial.printf("Free heap after send: %d bytes\n", ESP.getFreeHeap());
}


void clicknsend(){
  camera_fb_t *fb = esp_camera_fb_get();

  if(!fb){
    Serial.println("Chud gye guru! :(");
    return;
  }
  Serial.println("Clicked, DC pr milo :)");

  sendToDiscord(fb);

  esp_camera_fb_return(fb);
}

void loop() {
  digitalWrite(33,LOW);
  int objVal = digitalRead(irPin);
  if(!objVal){
    digitalWrite(4,HIGH);
    clicknsend();
    digitalWrite(4,LOW);
    delay(5000);
  }
}
