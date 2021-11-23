#include "ADS8332.h"

ADS8332::ADS8332(uint8_t _SelectPin, uint8_t _ConvertPin, uint8_t _EOCPin)
{
	SelectPin = _SelectPin;
	ConvertPin = _ConvertPin;
	EOCPin = _EOCPin;
	pinMode(ConvertPin, OUTPUT);
	digitalWrite(ConvertPin, HIGH);
	pinMode(SelectPin, OUTPUT);
	digitalWrite(SelectPin, HIGH);
	pinMode(EOCPin, INPUT);
	Vref = 2.5;
	EOCTimeout = 100000;
	ConnectionSettings = SPISettings(12000000, MSBFIRST, SPI_MODE1);

	myspi = nullptr;	
}

void ADS8332::setCommandBuffer(CommandRegister Command)
{
	CommandBuffer = 0;
	CommandBuffer = ((uint16_t)static_cast<uint8_t>( Command )) << 12;
}

void ADS8332::begin(SPIClass * spi)
{
    myspi = spi;

	setCommandBuffer(CommandRegister::WriteConfig);
	setConfiguration(ConfigRegisterMap::ChannelSelectMode, false);
	setConfiguration(ConfigRegisterMap::ClockSource, true);
	setConfiguration(ConfigRegisterMap::TriggerMode, true);
	setConfiguration(ConfigRegisterMap::SampleRate, true);
	setConfiguration(ConfigRegisterMap::EOCINTPolarity, true);
	setConfiguration(ConfigRegisterMap::EOCINTMode, true);
	setConfiguration(ConfigRegisterMap::ChainMode, true);
	setConfiguration(ConfigRegisterMap::AutoNap, true);
	setConfiguration(ConfigRegisterMap::Nap, true);
	setConfiguration(ConfigRegisterMap::Sleep, true);
	setConfiguration(ConfigRegisterMap::TAG, true);
	setConfiguration(ConfigRegisterMap::Reset, true);
	//Serial.println(CommandBuffer,BIN);
	sendCommandBuffer(true);
	//sendWriteCommandBuffer();
}

void ADS8332::begin()
{
	setCommandBuffer(CommandRegister::WriteConfig);
	setConfiguration(ConfigRegisterMap::ChannelSelectMode, false);
	setConfiguration(ConfigRegisterMap::ClockSource, true);
	setConfiguration(ConfigRegisterMap::TriggerMode, true);
	setConfiguration(ConfigRegisterMap::SampleRate, true);
	setConfiguration(ConfigRegisterMap::EOCINTPolarity, true);
	setConfiguration(ConfigRegisterMap::EOCINTMode, true);
	setConfiguration(ConfigRegisterMap::ChainMode, true);
	setConfiguration(ConfigRegisterMap::AutoNap, true);
	setConfiguration(ConfigRegisterMap::Nap, true);
	setConfiguration(ConfigRegisterMap::Sleep, true);
	setConfiguration(ConfigRegisterMap::TAG, true);
	setConfiguration(ConfigRegisterMap::Reset, true);
	//Serial.println(CommandBuffer,BIN);
	sendCommandBuffer(true);
	//sendWriteCommandBuffer();
}


void ADS8332::reset()
{
	setCommandBuffer(CommandRegister::WriteConfig);
	setConfiguration(ConfigRegisterMap::ChannelSelectMode, false);
	setConfiguration(ConfigRegisterMap::ClockSource, true);
	setConfiguration(ConfigRegisterMap::TriggerMode, true);
	setConfiguration(ConfigRegisterMap::SampleRate, true);
	setConfiguration(ConfigRegisterMap::EOCINTPolarity, true);
	setConfiguration(ConfigRegisterMap::EOCINTMode, true);
	setConfiguration(ConfigRegisterMap::ChainMode, true);
	setConfiguration(ConfigRegisterMap::AutoNap, true);
	setConfiguration(ConfigRegisterMap::Nap, true);
	setConfiguration(ConfigRegisterMap::Sleep, true);
	setConfiguration(ConfigRegisterMap::TAG, true);
	setConfiguration(ConfigRegisterMap::Reset, false);
	//Serial.println(CommandBuffer,BIN);
	sendCommandBuffer(true);
	//sendWriteCommandBuffer();
}

void ADS8332::setConfiguration(ConfigRegisterMap Option, bool Setting)
{
	bitWrite(CommandBuffer, static_cast<uint8_t>(Option), Setting);
}

void ADS8332::setVref(float NewVref)
{
	Vref = NewVref;
}

float ADS8332::getVref()
{
	return Vref;
}

/*uint32_t ADS8332::getEOCPort()
{
    return EOCPortMask;
}*/

uint16_t ADS8332::getConfig()
{
	setCommandBuffer(CommandRegister::ReadConfig);
	return sendCommandBuffer(true);
	//return sendReadCommandBuffer();
}

void ADS8332::print_binary(uint32_t v)
{
	int mask = 0;
	int n = 0;
	int num_places = 32;
	for (n=1; n<=num_places; n++)
	{
		mask = (mask << 1) | 0x0001;
	}
	v = v & mask;  // truncate v to specified number of places
	while(num_places)
	{
		if (v & (0x0001 << (num_places-1) ))
		{
			Serial.print("1");
		}
		else
		{
			Serial.print("0");
		}
		--num_places;
	}
}

uint16_t ADS8332::sendCommandBuffer(bool SendLong)
{
	union DataConverter
	{
		uint16_t UIntLargeData;
		uint8_t UIntSmallData[2];
	};
	DataConverter TempInput;
	DataConverter TempOutput;
	TempOutput.UIntLargeData = CommandBuffer;
	//Serial.print("COMMAND BUFFER: ");
	//Serial.println(CommandBuffer, BIN);

	myspi->beginTransaction(ConnectionSettings);
	digitalWrite(SelectPin,LOW);
	delayMicroseconds(0);
	myspi->transfer( 0 );
	if (SendLong)
	{
		TempInput.UIntSmallData[1] = myspi->transfer( TempOutput.UIntSmallData[1] );
		TempInput.UIntSmallData[0] = myspi->transfer( TempOutput.UIntSmallData[0] );
	}
	else
	{
		TempInput.UIntSmallData[1] = myspi->transfer( TempOutput.UIntSmallData[1] );
	}
	myspi->endTransaction();
	digitalWrite(SelectPin, HIGH);

/*	Serial.print("O:");
	Serial.print(TempOutput.UIntSmallData[1]);
	Serial.print(":");
	Serial.print(TempOutput.UIntSmallData[0]);
	Serial.print(";\n");*/
	return TempInput.UIntLargeData;
}

uint8_t ADS8332::getSample(float* WriteVariable, uint8_t UseChannel)
{
	uint16_t IntegerValue = 0;
	uint8_t status = getSample(&IntegerValue, UseChannel);
	*WriteVariable = Vref * ( (float)(IntegerValue) / 65535.0);
	return status;
}

uint8_t ADS8332::getSample(uint16_t* WriteVariable, uint8_t UseChannel)
{
	Channel = (uint8_t)( constrain(UseChannel,0,7) );
	setSampleChannel();
	return getSampleInteger(WriteVariable);
}

void ADS8332::setSampleChannel()
{
	switch (Channel)
	{
		case(0):
			setCommandBuffer(CommandRegister::SelectCh0);
			break;
		case(1):
			setCommandBuffer(CommandRegister::SelectCh1);
			break;
		case(2):
			setCommandBuffer(CommandRegister::SelectCh2);
			break;
		case(3):
			setCommandBuffer(CommandRegister::SelectCh3);
			break;
		case(4):
			setCommandBuffer(CommandRegister::SelectCh4);
			break;
		case(5):
			setCommandBuffer(CommandRegister::SelectCh5);
			break;
		case(6):
			setCommandBuffer(CommandRegister::SelectCh6);
			break;
		case(7):
			setCommandBuffer(CommandRegister::SelectCh7);
			break;
		default:
			setCommandBuffer(CommandRegister::SelectCh0);
			break;
	}
	sendCommandBuffer(false);
}

uint8_t ADS8332::getSampleInteger(uint16_t* WriteVariable)
{
	if (!beginsent)
	{
		begin();
		beginsent = true;
	}
	union DataConverter
	{
		uint16_t UIntLargeData;
		uint8_t UIntSmallData[2];
	};
	DataConverter TempInput;
	DataConverter TempOutput;
	setCommandBuffer(CommandRegister::ReadData);
	TempOutput.UIntLargeData = CommandBuffer;
	uint32_t starttime = micros();
	bool keepwaiting = true;
	digitalWrite(ConvertPin, LOW);
	while(keepwaiting)
	{
		if (digitalRead(EOCPin) == 0)    
		{
			keepwaiting = false;
		}
		else
		{
			if ( (micros() - starttime) > EOCTimeout)
			{
				digitalWrite(ConvertPin, HIGH);
				return 1;
			}
		}
	}
	digitalWrite(ConvertPin, HIGH);
	keepwaiting = true;
	starttime = micros();
	myspi->beginTransaction(ConnectionSettings);
	while(keepwaiting)
	{
		if (digitalRead(EOCPin) == 1)   
		{
			keepwaiting = false;
		}
		else
		{
			if ( (micros() - starttime) > EOCTimeout)
			{
				myspi->endTransaction();
				return 2;
			}
		}
	}
	starttime = micros();
	keepwaiting = true;
	uint8_t TAGData = 255;
	bool ChannelCorrect = false;
	bool TagBlank = false;
	uint8_t ChannelTag = 255;
	while(keepwaiting)
	{
		digitalWrite(SelectPin,LOW);
		TempInput.UIntSmallData[1] = myspi->transfer( TempOutput.UIntSmallData[1] );
		TempInput.UIntSmallData[0] = myspi->transfer( TempOutput.UIntSmallData[0] );
		TAGData = myspi->transfer( 0 );
		digitalWrite(SelectPin, HIGH);
		myspi->endTransaction();
		ChannelTag = (uint8_t)(TAGData>>5);
		ChannelCorrect = ( ChannelTag == Channel );
		TagBlank = (uint8_t)(TAGData << 3) == (uint8_t)(0);
		if (ChannelCorrect && TagBlank)
		{
			/*Serial.print("ADCS ");
			Serial.print(ChannelTag);
			Serial.print(",");
			Serial.print(Channel);
			Serial.print(",");
			Serial.print(TempInput.UIntLargeData);
			Serial.print("\n");*/
			*WriteVariable = TempInput.UIntLargeData;
			return 0;
		}
		else
		{
			if ( (micros() - starttime) > EOCTimeout)
			{
				
				//Serial.write("timeout!");
				/*Serial.print("ADCE ");
				Serial.print(ChannelTag);
				Serial.print(",");
				Serial.print(Channel);
				Serial.print(",");
				Serial.print(TempInput.UIntLargeData);
				Serial.print("\n");*/
				
				return 3;
			}
			else
			{
				setSampleChannel();
			}
		}
	}
	return 4;
}

SPISettings* ADS8332::GetSPISettings()
{
	return &ConnectionSettings;
}

