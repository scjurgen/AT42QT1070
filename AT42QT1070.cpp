
#include <avr/pgmspace.h>

#include "AT42QT1070.h"


AT42QT1070* AT42QT1070::mySingleSelf = 0;

AT42QT1070::AT42QT1070()
{
	mySingleSelf=this; // we need this to create a variable accessbile by the interrupt routine
	AT42QTchangeEvent=0;
	_errorCallback=0;
	Wire.begin();
}

// callback function for interrupt, replicate if you want another behaviour on interrupt, see test
void AT42QT_CHANGEISR()
{
	AT42QT1070::mySingleSelf->AT42QTchangeEvent++;	
}

void AT42QT1070::setInternalISR(uint8_t pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH); // pull high, CHANGE is open drain
  attachInterrupt(pin-2,AT42QT_CHANGEISR,FALLING); // on active falls low
}

void AT42QT1070::setErrorCallback(void (*cb)(uint8_t))
{
	_errorCallback=cb;
}
uint8_t AT42QT1070::isCalibrating()
{
	return 0x80==(0x80&getRegValue(DETECTIONSTATUS));
}


uint8_t AT42QT1070::setActiveAddress(uint8_t adr)
{
	Wire.beginTransmission(AT42I2CAdr);
	Wire.write(adr);
	return checkResult(Wire.endTransmission());
}

uint16_t AT42QT1070::readKeyValues(uint8_t key)
{
	uint16_t retval=0;
	if (!setActiveAddress(KEYSIGNAL+key*2))
		return 0;
	Wire.requestFrom(AT42I2CAdr, (uint8_t)2);
	if(Wire.available()==2)
	{
	  
		uint8_t msb = Wire.read();
		uint8_t lsb = Wire.read();
		retval=((uint16_t)msb<<8)+lsb;
	}
	return retval;
}


uint8_t AT42QT1070::getRegValue(uint8_t adr)
{
	uint8_t val=0;
	if (!setActiveAddress(adr))
		return 0;

	Wire.requestFrom(AT42I2CAdr, (uint8_t)1);
	if (Wire.available()==1)
	{
		val = Wire.read();
	}
	return val;
}


void AT42QT1070::setRegValuePreserved(uint8_t regAdr, uint8_t val, uint8_t maskPreserve)
{
    uint8_t preserve=getRegValue(regAdr);
    uint8_t orgval=preserve & maskPreserve;
    Wire.beginTransmission(AT42I2CAdr);
    Wire.write(regAdr);
    Wire.write(orgval|val);
    //int result = Wire.endTransmission();
    //checkResult(result);
}



void AT42QT1070::setRegValue(uint8_t regAdr, uint8_t val)
{
    Wire.beginTransmission(AT42I2CAdr);
    Wire.write(regAdr);
    Wire.write(val);
    uint8_t result = Wire.endTransmission();
    checkResult(result);
}

uint8_t AT42QT1070::changedDiffSet(int8_t *delta)
{
	uint16_t currentVal[AT42QT1070_MAXKEYS];
	
	if (!setActiveAddress(KEYSIGNAL))
		return 0;
	Wire.requestFrom(AT42I2CAdr, (uint8_t)(AT42QT1070_MAXKEYS*2));
	if(Wire.available()==AT42QT1070_MAXKEYS*2)
	{
		for (uint8_t i=0; i < AT42QT1070_MAXKEYS; i++)
		{
			uint8_t msb = Wire.read();
			uint8_t lsb = Wire.read();
			currentVal[i]=((uint16_t)msb<<8)+lsb;
			int16_t dt = currentVal[i]-refSet[i];
			if (dt >=127) dt=127;
			else
				if (dt <=-127) dt=-127; // actually we could do -128.. but what the heck
			delta[i]=dt;
		}
	}	
	return 1;
}


uint8_t AT42QT1070::saveReferenceSet()
{
	uint16_t currentVal[AT42QT1070_MAXKEYS];
	
	if (!setActiveAddress(KEYSIGNAL))
		return 0;

	Wire.requestFrom(AT42I2CAdr, (uint8_t)(AT42QT1070_MAXKEYS*2));
	if(Wire.available()==AT42QT1070_MAXKEYS*2)
	{
		for (uint8_t i=0; i < AT42QT1070_MAXKEYS; i++)
		{
			uint8_t msb = Wire.read();
			uint8_t lsb = Wire.read();
			refSet[i]=((uint16_t)msb<<8)+lsb;
		}
	}

	return 1;
}


void AT42QT1070::reset() // all values will be default
{
    setRegValue(RESET,0x1);
	delay(200+125+30); 
}
void AT42QT1070::calibrate() // all values will be default
{
    setRegValue(CALIBRATE,0x1);
    delay(30);
}

void AT42QT1070::lowPowerMode(uint8_t val)
{
	setRegValue(LOWPOWERMODE,val);
}

void AT42QT1070::maxOnDuration(uint8_t val)
{
	setRegValue(MAXONDURATION,val);
}


uint8_t AT42QT1070::checkResult(uint8_t result)
{
	if(result >= 1)
	{
		if (this->_errorCallback)
			this->_errorCallback(result);
		return 0;
	}
	return 1;
}    


