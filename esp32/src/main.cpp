#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>

#include "../../include/networking/message.h"
#include "TCPServer.h"

const char *ssid = "SSID";
const char *password = "PASSWORD";

const uint16_t port = 60000;
TCPServer tcp_server(port);
AsyncClient *tcp_client;  // Caveat: only one client which respond to
bool isUpdating = false;
int delayTime = 0;

using Message = fortress::net::message<fortress::net::MsgTypes>;

void startUpdating(Message &msg, AsyncClient *client) {
    if (isUpdating) {
        std::cerr << "Already sending reading" << std::endl;
        return;
    }

    uint16_t frequency;
    msg >> frequency;
    delayTime = static_cast<int>(1.0 / frequency * 1'000);

    if (delayTime > 0) {
        tcp_client = client;
        isUpdating = true;
        std::cout << "Start updating every " << delayTime << " us" << std::endl;
    }
}

void stopUpdating() {
    isUpdating = false;
    std::cout << "Stop updating" << std::endl;
    tcp_client = nullptr;
}

void onMessage(Message &msg, AsyncClient *client) {
    // std::cout << "A message arrived " << msg << std::endl;

    switch (msg.header.id) {
        using namespace fortress::net;
        case MsgTypes::ClientStartUpdating:
            startUpdating(msg, client);
            break;

        case MsgTypes::ClientStopUpdating:
            stopUpdating();
            break;

        default:
            break;
    }
}

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

    tcp_server.setOnMessageCallback(onMessage);
    tcp_server.begin();
}

void loop() {
    if (isUpdating) {
        Message msg;
        msg.header.id = fortress::net::MsgTypes::ServerReadings;
        // Insert 4 double channel readings
        msg << random(100) / 1000.0 << random(100) / 1000.0
            << random(100) / 1000.0 << random(100) / 1000.0;
        tcp_server.sendMessage(msg, tcp_client);
        delayMicroseconds(delayTime * 1000);
    }
}