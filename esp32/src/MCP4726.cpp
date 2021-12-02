#include "MCP4726.h"


MCP4726::MCP4726() {
}

void MCP4726::begin(TwoWire *theWire){
  i2caddr = MCP4726_DEFAULT_ADDR;
  wire = theWire;
  configRegister = MCP4726_DEFAULT_CONFIG;
  
}

void MCP4726::begin(uint8_t addr, TwoWire *theWire) {
  i2caddr = addr;
  wire = theWire;
  configRegister = MCP4726_DEFAULT_CONFIG;
  //return init();
}
 
bool MCP4726::testConnection() {
    wire->beginTransmission(i2caddr);
    return (wire->endTransmission() == 0);
}

void MCP4726::setOutputValue( uint16_t output)
{
  wire->beginTransmission(i2caddr);
  wire->write((configRegister | MCP4726_VOLALL ) & MCP4726_PWR_MASK);
  wire->write((uint8_t) ((output >> 4) & 0xFF));    
  wire->write((uint8_t) ((output << 4) & 0xF0));  
  wire->endTransmission();
}

void MCP4726::setVref(uint8_t Vref){
  configRegister = (configRegister & MCP4726_VREF_MASK) | (Vref & !MCP4726_VREF_MASK);
}

void MCP4726::setGain(uint8_t gain){
  configRegister = (configRegister & MCP4726_GAIN_MASK) | (gain & !MCP4726_GAIN_MASK);
}
