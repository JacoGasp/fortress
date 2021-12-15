#include "ACF2101.h"


ACF2101::ACF2101(uint8_t _SelectPin, uint8_t _HoldPin, uint8_t _ResetPin)
{
	SelectPin = _SelectPin;
	HoldPin = _HoldPin;
    ResetPin = _ResetPin;
    
}

void ACF2101::begin()
{
    pinMode(SelectPin, OUTPUT);
	pinMode(HoldPin, OUTPUT);
	pinMode(ResetPin, OUTPUT);
    //LOW = on, HIGH = off
    digitalWrite(SelectPin, LOW);
	digitalWrite(HoldPin, LOW);
    //reset integrators
	digitalWrite(ResetPin, LOW);
}

void ACF2101::reset()
{
    //digitalWrite(HoldPin, HIGH);
    digitalWrite(ResetPin, LOW);
    delayMicroseconds(100);
    digitalWrite(ResetPin, HIGH);
    //digitalWrite(HoldPin, LOW);

}

void ACF2101::stop()
{
    //digitalWrite(HoldPin, HIGH);
    digitalWrite(ResetPin, LOW);
}