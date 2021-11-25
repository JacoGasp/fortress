#ifndef __ACF2101
#define __ACF2101

#include <Arduino.h>

class ACF2101
{
    public:
        ACF2101(uint8_t SelectPin, uint8_t HoldPin, uint8_t ResetPin);
        void reset();
        void stop();

    private:
        uint8_t SelectPin;
        uint8_t HoldPin;
        uint8_t ResetPin;
};





#endif