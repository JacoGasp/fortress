#include "ACF2101.h"


ACF2101::ACF2101(uint8_t _SelectPin, uint8_t _HoldPin, uint8_t _ResetPin)
{
	SelectPin = _SelectPin;
	HoldPin = _HoldPin;
    ResetPin = _ResetPin;

    pinMode(SelectPin, OUTPUT);
	digitalWrite(SelectPin, HIGH);
	pinMode(HoldPin, OUTPUT);
	digitalWrite(HoldPin, LOW);
	pinMode(ResetPin, OUTPUT);
    digitalWrite(ResetPin, HIGH);
}

void ACF2101::reset()
{
    digitalWrite(HoldPin, HIGH);
    digitalWrite(ResetPin, LOW);
    digitalWrite(ResetPin, HIGH);
    digitalWrite(HoldPin, LOW);

}

void ACF2101::stop()
{
    digitalWrite(HoldPin, HIGH);
}