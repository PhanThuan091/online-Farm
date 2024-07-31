#include <Arduino.h>
#include <DHT.h>

class PinController {
public:
    PinController(int pin1, int pin2, int dhtPin, int dhtType) 
        : led1(pin1), led2(pin2), dht(dhtPin, dhtType) {}

    void begin() {
        pinMode(led1, OUTPUT);
        pinMode(led2, OUTPUT);
        dht.begin(); // Khởi tạo cảm biến DHT11
    }

    void setPin1State(bool state) {
        digitalWrite(led1, state ? HIGH : LOW);
        Serial.println(state ? "LED turned ON" : "LED turned OFF");
    }

    float readHumidity() {
        return dht.readHumidity();
    }

    float readTemperature() {
        return dht.readTemperature();
    }

private:
    int led1;
    int led2;
    DHT dht;
};
