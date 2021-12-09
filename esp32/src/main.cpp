#include <Arduino.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <SPI.h>
#include <Esp.h>
#include <Wire.h>

#include "../../include/networking/message.h"
#include "TCPServer.h"

#include "pinMapping.h"
#include "ADS8332.h"
#include "ACF2101.h"
#include "MCP4726.h"


//BUZZER pwm properties
const int PWM_FREQ = 4000;
const int BUZZER_PWM_CHAN = 0;
const int PWM_RESOLUTION = 8;

//battery level properties
const int VBATT_ADC_MIN = 3030; 

//ADC 
ADS8332 ADC(ADS8332_CS, ADS8332_CONVST, ADS8332_EOC_INT);
const float Vref = 4.096;   // Volt
std::array<uint16_t, fortress::consts::N_CHANNELS> sensorReadings = {0, 0, 0, 0};

// charge integrators
ACF2101 chargeIntegrator(ACF2101_SEL, ACF2101_HLD, ACF2101_RST);
const uint16_t integratorThreshold = 65500;  //threshold on ADC reading (16 bit)

//Sensor HV
TwoWire I2CHV = TwoWire(0);
MCP4726 HVDAC;
uint16_t sensorHV = 0;    //HV level 0-4095
const float DACVref = 2.048;   //Volt
const double DAC_OPAMP_GAIN = 24.66796875;  

//SPI
SPIClass * hspi = NULL;

//TCP
const char *ssid = "SSID";
const char *password = "PASSWORD";

const uint16_t port = 60000;
TCPServer tcp_server(port);
AsyncClient *tcp_client;  // Caveat: only one client which respond to

//reading variables
bool isUpdating = false;
unsigned long previousMicros = 0;
long samplingInterval = 1000;
unsigned long prevInfoMicros = 0;
long diplayInfoInterval = 5 * 1E6;
unsigned long totalReadings;


//------------hardware functions-----------------
void bipSpeaker(int bipNum){
    for (int i=0; i <= bipNum; i++){
        ledcWrite(BUZZER_PWM_CHAN, 50);
        delay(100);
        ledcWrite(BUZZER_PWM_CHAN, 0);
        delay(100);
    }
}

void blinkLed(uint8_t ledpin, int tmillis, int blinkNum){
    for (int i = 0; i <= blinkNum; i++){
        digitalWrite(ledpin, HIGH);
        delay(tmillis);
        digitalWrite(ledpin, LOW);
        delay(tmillis);
    }
}

void enableAnalog(){
    digitalWrite(VAN_EN, HIGH);
}


void disableAnalog(){
    digitalWrite(VAN_EN, LOW);
}


//RTOS task to check if button power is pressed, priority 1(?)
static void checkPowerOffTask(void* pvParameters){    
    for ( ;; ) {
        if (digitalRead(LTC3101_PBSTAT)){
            //tasto non premuto
            digitalWrite(LTC3101_PWRON, HIGH);
        }
        else {
            //tasto premuto
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_BLUE, HIGH);
            bipSpeaker(5);
            digitalWrite(LTC3101_PWRON, LOW);
        }
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}


/*void IRAM_ATTR Ext_INT_ISR(){
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis(); 
    if (interrupt_time - last_interrupt_time > 100){
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_BLUE, HIGH);
        bipSpeaker(5);
        digitalWrite(LTC3101_PWRON, LOW);
    }
    last_interrupt_time = interrupt_time;
}
*/
//battery level task
static void checkBatteryLevel(void* pvParameters){
    for( ;; ){
        Serial.print("battery ");
        Serial.println(analogRead(VBATT_ADC));
        if (analogRead(VBATT_ADC) <= VBATT_ADC_MIN){
            bipSpeaker(3);
        }
    vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}


//----------------ON MESSAGE--------------- 

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

void setSensorHV(Message &msg){
    msg >> sensorHV;
    HVDAC.setOutputValue(sensorHV);
    std::cout << "Sensor HV set to: " << sensorHV << std::endl;
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

        case MsgTypes::ClientSetSensorHV:
            setSensorHV(msg);
            break;

        default:
            break;
    }
}



void setup() {
    //pin led configuration
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    //power controller LTC3101 configuration
    pinMode(LTC3101_PWRON, OUTPUT);
    digitalWrite(LTC3101_PWRON, HIGH);
    
    //enable analog configuration;
    pinMode(VAN_EN, OUTPUT);
    digitalWrite(VAN_EN, LOW);

    chargeIntegrator.begin();
    
    //pwm configuration and buzzer
    ledcSetup(BUZZER_PWM_CHAN, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(BUZZER, BUZZER_PWM_CHAN);

    //ritardo per evitare autospegnimento LTC3101
    //l'utente quando vede il led verde acceso può rilasciare il pulsante
    delay(200);
    digitalWrite(LED_GREEN, HIGH);
    bipSpeaker(2);

    pinMode(LTC3101_PBSTAT, INPUT);
    //aspetta 1 secondo per evitare che venga letto il tasto come spegnimento
    delay(1000); 

    //battery level
    pinMode(VBATT_ADC, INPUT);

    //button off interrupt
    //attachInterrupt(digitalPinToInterrupt(LTC3101_PBSTAT), Ext_INT_ISR, LOW);

    //brackground tasks:
    xTaskCreate(checkPowerOffTask, "PowerOFF", 1024, NULL, 1, NULL);
    //xTaskCreate(checkBatteryLevel, "batteryLevel", 1024, NULL, 1, NULL);

    //turn on analog circuit
    enableAnalog();

    //serial, i2c, spi init
    Serial.begin(115200);
    delay(10);
    
    Serial.println("Pin set, analog enabled");

    float vbatt = (float)analogRead(VBATT_ADC) * 167 / 120;
    Serial.print("Battery level (V): ");
    Serial.println(vbatt);

    hspi = new SPIClass(HSPI);
    hspi->begin();
    I2CHV.begin(I2C_SDA, I2C_SCL, 100000);
    
    Serial.println("hspi, I2C OK");

    //ADC init
    ADC.begin(hspi);
    ADC.setVref(Vref);
    
    Serial.println("ADC init");

    //DAC HV init
    HVDAC.begin(&I2CHV);
    Serial.print("test i2c connection to DAC: ");
    bool DACConnect = HVDAC.testConnection();
    Serial.println(DACConnect);
    HVDAC.setVref(MCP4726_VREF_VREFPIN_BUFF);
    HVDAC.setGain(MCP4726_GAIN_1X);
    HVDAC.setOutputValue(sensorHV);
    
    Serial.println("HV dac init");
    
    //wifi connection
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        //delay(500);
        //Serial.print(".");
        blinkLed(LED_BLUE, 250, 1);
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

        uint8_t adcstat0 = ADC.getSample(&sensorReadings[0], 0);
        uint8_t adcstat1 = ADC.getSample(&sensorReadings[1], 1);
        uint8_t adcstat2 = ADC.getSample(&sensorReadings[2], 2);
        uint8_t adcstat3 = ADC.getSample(&sensorReadings[3], 3);

        //Serial.println(sensorReadings[0]);
        Serial.print("ADC status: ");
        Serial.print(adcstat0);
        Serial.print(" , ");
        Serial.print(adcstat1);
        Serial.print(" , ");
        Serial.print(adcstat2);
        Serial.print(" , ");
        Serial.println(adcstat3);
        //Serial.println(micros());
        
        // if (std::any_of(sensorReadings.begin(), sensorReadings.end(), [](uint16_t x){return x >= integratorThreshold;})) {
        //     chargeIntegrator.reset();
        // }
        
        Message msg;
        msg.header.id = fortress::net::MsgTypes::ServerReadings;
        // Insert 4 double channel readings
        /*msg << static_cast<uint16_t>(random(1024))      // Ch. 4
            << static_cast<uint16_t>(random(1024))      // Ch. 3
            << static_cast<uint16_t>(random(1024))      // Ch. 2
            << static_cast<uint16_t>(random(1024));     // Ch. 1
       */
        msg << sensorReadings[3]      // Ch. 4
            << sensorReadings[2]      // Ch. 3
            << sensorReadings[1]      // Ch. 2
            << sensorReadings[0];     // Ch. 1
           
        tcp_server.sendMessage(msg, tcp_client);
        previousMicros = currentMicros;
        ++totalReadings;
    }

    if (currentMicros - prevInfoMicros >= diplayInfoInterval) {
        printFreeMemory();
        prevInfoMicros = currentMicros;
    }
}


