#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "MQTTManager.h"
#include "PinController.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "FontMaker.h"


#define led1 25
#define led2 26
#define DHTPIN 16
#define DHTTYPE DHT11
#define TFT_DC 12    // A0
#define TFT_CS 13    // CS
#define TFT_MOSI 14  // SDA
#define TFT_CLK 27   // SCK
#define TFT_RST 0
#define TFT_MISO 0

const char* mqtt_server = "f9b8a42fa59847f5a39ae3d2d218c107.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "pthuan";
const char* mqtt_password = "thuctap";

MQTTManager mqttManager(mqtt_server, mqtt_port, mqtt_username, mqtt_password);
PinController pinController(led1, led2, DHTPIN, DHTTYPE);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
MakeFont myfont(&setpx);

void setup_wifi() {
  WiFiManager wm;
  bool res = wm.autoConnect("Amaduino Amazing", "amztech9");
  if (!res) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("connected...");
    delay(1000);  // Delay to ensure WiFi is properly connected
  }
}

void setpx(int16_t x, int16_t y, uint16_t color) {
  tft.drawPixel(x, y, color);
}
// MakeFont myfont(&setpx);

void updateDisplay(float temperature = NAN, float humidity = NAN);

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) {
    incommingMessage += (char)payload[i];
  }
  Serial.println("Message arrived [" + String(topic) + "] " + incommingMessage);

  if (String(topic) == "esp32/light1") {
    int lightState = incommingMessage.toInt();
    pinController.setPin1State(lightState == 1);
    updateDisplay();
  }
  if (String(topic) == "esp32/brightnessLight1") {
    int brightnessValue = incommingMessage.toInt();
    Serial.println("Brightness value: " + String(brightnessValue));
  }
}


void updateDisplay(float temperature, float humidity) {
  myfont.set_font(Amazingtime);
  myfont.print(0,0,"Temperature:",ST7735_RED,ST7735_BLACK);
  // tft.setCursor(85, 0);;'
  // tft.setTextColor(ST7735_RED, ST7735_BLACK);
  myfont.print(100, 0, String(temperature) + " °C",ST7735_RED, ST7735_BLACK);
  myfont.print(0,20,"Humidity:",ST7735_CYAN,ST7735_BLACK);
  myfont.print(85, 20,String(humidity) + " %",ST7735_CYAN, ST7735_BLACK);
  tft.drawRect(0,47,160,28,ST7735_WHITE);              //Hien thi khung 
  myfont.print(1,52,"Time LED1:",ST7735_CYAN,ST7735_BLACK);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Setup started"));
  setup_wifi();
  pinController.begin();

  mqttManager.begin();
  mqttManager.setCallback(callback);

  tft.initR(INITR_BLACKTAB);  // Initialize the display
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);  // Set rotation if needed

  xTaskCreate([](void* parameter) {
    for (;;) {
      mqttManager.loop();
      vTaskDelay(100);  // Increase delay to reduce CPU usage
    }
  },
              "MQTTTask", 10000, NULL, 1, NULL);

  Serial.println(F("Setup completed"));
}

unsigned long timeUpdata = millis();
void loop() {
  // read DHT11
  if (millis() - timeUpdata > 5000) {
    float h = pinController.readHumidity();
    float t = pinController.readTemperature();

    // Kiểm tra xem cảm biến có trả về giá trị hợp lệ không
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    DynamicJsonDocument doc(1024);
    doc["humidity"] = h;
    doc["temperature"] = t;
    char mqtt_message[128];
    serializeJson(doc, mqtt_message);
    mqttManager.publishMessage("esp32/dht22", mqtt_message, true);

    updateDisplay(t, h);  // Update the display with new sensor data

    timeUpdata = millis();
  }
  
}
