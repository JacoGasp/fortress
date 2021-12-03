#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <SPI.h>
#include <Esp.h>

#include "../../include/networking/message.h"
#include "TCPServer.h"

#include "pinMapping.h"
#include "ADS8332.h"
#include "ACF2101.h"

const char *ssid = "SSID";
const char *password = "PASSWORD";

const uint16_t port = 60000;
TCPServer tcp_server(port);
AsyncClient *tcp_client;  // Caveat: only one client which respond to
bool isUpdating = false;

unsigned long previousMicros = 0;
long samplingInterval = 1000;

unsigned long prevInfoMicros = 0;
long diplayInfoInterval = 5 * 1E6;

SPIClass * hspi = NULL;
unsigned long totalReadings;

//ADC 
ADS8332 ADC(ADS8332_CS, ADS8332_CONVST, ADS8332_EOC_INT);
const float Vref = 4.096;   // Volt
std::array<uint16_t, fortress::consts::N_CHANNELS> sensorReadings = {0, 0, 0, 0};

// charge integrators
ACF2101 chargeIntegrator(ACF2101_SEL, ACF2101_HLD, ACF2101_RST);
const uint16_t integratorThreshold = 65500;  //threshold on ADC reading (16 bit)


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

void setup() {
    hspi = new SPIClass(HSPI);
    hspi->begin();
    Serial.begin(115200);
    delay(10);
    ADC.begin(hspi);

    ADC.setVref(Vref);   
    
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
    unsigned long currentMicros = micros();
    
    // Because of async, can happen that currentMicros < previousMicros and since they are unsigned, the difference
    // can overflow
    if (isUpdating && currentMicros > previousMicros && currentMicros - previousMicros >= samplingInterval) {
        
        /*
        * read all ADC channels
        * important: getSample with no ADC connected adds 100 ms delay due to ADC timeout 
        */

        // uint8_t adcstatus = ADC.getSample(&sensorReadings[0], 0);
        //ADC.getSample(&sensorReadings[1], 1);
        //ADC.getSample(&sensorReadings[2], 2);
        //ADC.getSample(&sensorReadings[3], 3);

        //Serial.println(sensorReadings[0]);
        //Serial.print("ADC status: ");
        //Serial.print(adcstatus);
        //Serial.println(micros());
        
        // if (std::any_of(sensorReadings.begin(), sensorReadings.end(), [](uint16_t x){return x >= integratorThreshold;})) {
        //     chargeIntegrator.reset();
        // }
        
        Message msg;
        msg.header.id = fortress::net::MsgTypes::ServerReadings;
        // Insert 4 double channel readings
        msg << static_cast<uint16_t>(random(1024))      // Ch. 4
            << static_cast<uint16_t>(random(1024))      // Ch. 3
            << static_cast<uint16_t>(random(1024))      // Ch. 2
            << static_cast<uint16_t>(random(1024))      // Ch. 1
            << static_cast<uint32_t>(currentMicros - previousMicros);

        /*msg << sensorReadings[3]      // Ch. 4
            << sensorReadings[2]      // Ch. 3
            << sensorReadings[1]      // Ch. 2
            << sensorReadings[0]     // Ch. 1
            << static_cast<uint32_t>(currentMicros - previousMicros);
        */   
        tcp_server.sendMessage(msg, tcp_client);
        previousMicros = currentMicros;
        ++totalReadings;
    }

    if (currentMicros - prevInfoMicros >= diplayInfoInterval) {
        printFreeMemory();
        prevInfoMicros = currentMicros;
    }
}