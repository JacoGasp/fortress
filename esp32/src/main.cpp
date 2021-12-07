#define configUSE_TRACE_FACILITY 1

#include <Arduino.h>
#include <AsyncTCP.h>
#include <Esp.h>
#include <SPI.h>
#include <WiFi.h>

#include "../../include/SharedParams.h"
#include "../../include/networking/message.h"
#include "ACF2101.h"
#include "ADS8332.h"
#include "TCPServer.h"
#include "pinMapping.h"

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_NAME_HERE"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWD_HERE"
#endif
#ifndef PORT
#define PORT 60000
#endif

TCPServer tcp_server(PORT);
AsyncClient *tcp_client;  // Caveat: only one client which respond to
bool isUpdating = false;

unsigned long previousMicros = 0;
long samplingInterval = 1000;

// Display some info
unsigned long prevInfoMicros = 0;
long displayInfoInterval = 5 * 1E6;

SPIClass *hspi = NULL;
unsigned long totalReadings;

// ADC
ADS8332 ADC(ADS8332_CS, ADS8332_CONVST, ADS8332_EOC_INT);
const float Vref = 4.096;  // Volt
std::array<uint16_t, SharedParams::n_channels> sensorReadings = {0, 0, 0, 0};

// charge integrators
ACF2101 chargeIntegrator(ACF2101_SEL, ACF2101_HLD, ACF2101_RST);

// Test task
xTaskHandle xHandle;

using Message = fortress::net::message<fortress::net::MsgTypes>;

void printFreeMemory() {
    Serial.print("Free heap memory ");
    Serial.println(ESP.getFreeHeap());
}

void startUpdating(Message &msg, AsyncClient *client) {
    if (isUpdating) {
        std::cerr << "Already sending reading" << std::endl;
        return;
    }

    uint16_t frequency;
    msg >> frequency;
    samplingInterval = static_cast<long>(1.0 / frequency * 1'000'000);

    if (samplingInterval > 0) {
        tcp_client = client;
        isUpdating = true;
        totalReadings = 0;
        sensorReadings = {0, 0, 0, 0};
        std::cout << "Start updating every " << samplingInterval << " us" << std::endl;
        previousMicros = micros();
        chargeIntegrator.reset();
    }
}

void stopUpdating() {
    isUpdating = false;
    Message msg;
    msg.header.id = fortress::net::MsgTypes::ServerFinishedUpload;
    tcp_server.sendMessage(msg, tcp_client);
    std::cout << "Stop updating. Sent " << totalReadings + 1 << " readings" << std::endl;
    chargeIntegrator.stop();
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

void UpdateTask(void *args) {
    Serial.println("Start task");

    for (;;) {
        unsigned long currentMicros = micros();
        // Because of async, can happen that currentMicros < previousMicros and
        // since they are unsigned, the difference can overflow
        if (isUpdating && currentMicros > previousMicros && currentMicros - previousMicros >= samplingInterval) {
            /*
             * read all ADC channels
             * important: getSample with no ADC connected adds 100 ms delay due
             * to ADC timeout
             */

#ifdef EMULATE_SAMPLING
            // Dummy data for test
            for (auto it = sensorReadings.begin(); it != sensorReadings.end(); ++it) {
                *it += static_cast<uint16_t>(random(10));
                if (*it > SharedParams::integratorThreshold) *it -= SharedParams::integratorThreshold;
            }
#else
            uint8_t adcstatus = ADC.getSample(&sensorReadings[0], 0);
            ADC.getSample(&sensorReadings[1], 1);
            ADC.getSample(&sensorReadings[2], 2);
            ADC.getSample(&sensorReadings[3], 3);
            Serial.println(sensorReadings[0]);
            Serial.print("ADC status: ");
            Serial.print(adcstatus);
            Serial.println(micros());

            if (std::any_of(sensorReadings.begin(), sensorReadings.end(), [](uint16_t x) { 
                    return x >= SharedParams::integratorThreshold; 
                })) {
                chargeIntegrator.reset();
            }
#endif

            Message msg;
            msg.header.id = fortress::net::MsgTypes::ServerReadings;
            msg << sensorReadings[3]  // Ch. 4
                << sensorReadings[2]  // Ch. 3
                << sensorReadings[1]  // Ch. 2
                << sensorReadings[0]  // Ch. 1
                << static_cast<uint32_t>(currentMicros - previousMicros);

            tcp_server.sendMessage(msg, tcp_client);
            previousMicros = currentMicros;
            ++totalReadings;
        }

        if (currentMicros - prevInfoMicros >= displayInfoInterval) {
            printFreeMemory();
            prevInfoMicros = currentMicros;
        }
    }
}

void setup() {
    hspi = new SPIClass(HSPI);
    hspi->begin();
    Serial.begin(115200);
    delay(10);
    ADC.begin(hspi);

    ADC.setVref(Vref);

    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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

    // xTaskCreate(UpdateTask, "Update", configMINIMAL_STACK_SIZE * 3, NULL, 10, NULL);
}

void loop() { UpdateTask(nullptr); }