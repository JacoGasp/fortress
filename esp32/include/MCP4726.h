#ifndef __MCP4726
#define __MCP4726

#include "Arduino.h"
#include <Wire.h>

#define MCP4726_DEFAULT_ADDR  0x60
#define MCP4726_DEFAULT_CONFIG 0x00

//V reference
#define MCP4726_VREF_MASK       0xE7 
#define MCP4726_VREF_VDD        0x00
#define MCP4726_VREF_VREFPIN    0x10
#define MCP4726_VREF_VREFPIN_BUFF 0x18
//Gain
#define MCP4726_GAIN_MASK   0xE1
#define MCP4726_GAIN_1X     0x00
#define MCP4726_GAIN_2X     0x01
//powerOn
#define MCP4726_PWR_MASK    0xF9
//command
#define MCP4726_CMD_MASK    0x1F
#define MCP4726_VOLALL  0x40





class MCP4726{
    public:
        MCP4726();
        void begin(TwoWire * w);
        void begin(uint8_t a, TwoWire * w);  
        void setOutputValue( uint16_t output);
        void setVref(uint8_t vref);
        void setGain(uint8_t gain);
        bool testConnection();

    private:
        uint8_t i2caddr;
        TwoWire * wire;
        uint8_t configRegister;
};


#endif