#include <PubSubClient.h>
#include <WiFiClientSecure.h>

class MQTTManager {
public:
    MQTTManager(const char* server, int port, const char* username, const char* password)
        : mqtt_server(server), mqtt_port(port), mqtt_username(username), mqtt_password(password), client(espClient) {}

    void begin() {
        espClient.setInsecure();
        client.setServer(mqtt_server, mqtt_port);
    }

    void setCallback(MQTT_CALLBACK_SIGNATURE) {
        client.setCallback(callback);
    }

    void loop() {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
    }

    void publishMessage(const char* topic, const char* payload, boolean retained) {
        if (client.publish(topic, payload, retained)) {
            Serial.println("Message published [" + String(topic) + "]: " + payload);
        } else {
            Serial.println("Message publishing failed");
        }
    }

private:
    void reconnect() {
        while (!client.connected()) {
            Serial.print("Attempting MQTT connection...");
            String clientID = "ESPClient-";
            clientID += String(random(0xffff), HEX);
            if (client.connect(clientID.c_str(), mqtt_username, mqtt_password)) {
                Serial.println("connected");
                client.subscribe("esp32/client");
                client.subscribe("esp32/light1");
                client.subscribe("esp32/brightnessLight1");
            } else {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" try again in 10 seconds");
                delay(10000); // Increase delay to 10 seconds
            }
        }
    }

    const char* mqtt_server;
    int mqtt_port;
    const char* mqtt_username;
    const char* mqtt_password;
    WiFiClientSecure espClient;
    PubSubClient client;
};
