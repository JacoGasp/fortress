#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "TCPServer.h"

const char *ssid = "SSID";
const char *password = "PASSWORD";
const uint16_t port = 60000;

TCPServer tcp_server(port);


void setup() {
    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    tcp_server.begin();
}


void loop() {

}
