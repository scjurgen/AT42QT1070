/************************************************************************************
* 
* Name : AT42QT test
* Author : Jurgen Schwietering
* Date : 2012-04-30
* Version : 0.0
* /brief : Library for use with AT42QT10x0 Q-touch i2c
* Clock pin 6 of SOIC-14pin package to SCL (analog 5 on arduino uno)
* Data pin 3 of SOIC-14pin package to SDA (analog 4 on arduino uno)
* other pins: (QT1070)
*  1=Vdd
*  2=Mode (should be GND for interfacing with micro)
*  3=SDA
*  4=RESET (leave open)
*  5=CHANGE (Interrupt)
*  6=SCL
*  7=KEY6
*  
*  8=Key5
*  9=Key4
* 10=Key3
* 11=Key2
* 12=Key1
* 13=Key0
* 14=GND (VSS)
*
* check datasheet for 20-pin VQFN package
************************************************************************************/


#include <Wire.h>
#include "AT42QT1070.h"

const uint8_t isrPin = 2; // interrupt vector 0
const uint8_t ledPin = 13;

AT42QT1070 qTouch;


void ErrorCallback(uint8_t errNumber)
{
	Serial.print("An error occured in i2c:");
	Serial.println(errNumber);
	switch(errNumber)
	{
		case 1: Serial.print(F("data too long ")); break;
		case 2: Serial.print(F("received NACK on address ")); break;
		case 3: Serial.print(F("recevied NACK on transmit ")); break;
		case 4:  // fll through
		default:
				Serial.print(F("unknown err. (chip no connecte?)")); break;
	}	
}


void printValue(char *msg, int value)
{
	Serial.print(msg);
	Serial.println(value);
}

void setup()
{
	Serial.begin(9600);
 	// touch.begin(interruptPin,false);
  	pinMode(ledPin,OUTPUT);

  	printValue("ChipID=",qTouch.getRegValue(AT42QT1070::CHIPID));
  	qTouch.setInternalISR(2);
  	qTouch.setErrorCallback(ErrorCallback);

//	setKeyHitISR(2);
  	qTouch.reset();
  
  	qTouch.calibrate();
  	while(qTouch.isCalibrating())
  	{
  		Serial.print(".");
  		delay(10);
  	}

 	qTouch.maxOnDuration(8); // maximum on duration of key (8*160ms)
	qTouch.lowPowerMode(4); // full power, let's save a bit if gasoline
 	Serial.print("max on duration:");
  	Serial.println(qTouch.getRegValue(AT42QT1070::MAXONDURATION));
  	Serial.print("low power mode read:");
  	Serial.println(qTouch.getRegValue(AT42QT1070::LOWPOWERMODE));
}


#define BLINKPERIOD 50 // equals 100 because duty is 50%
static uint8_t blnkCount=2;
static unsigned long nextBlink=10;


/**
blink the number of times of the last key pressed
*/
void blinkNumber(uint8_t cnt)
{
	if (blnkCount) // dont set when blinking
		return;
	blnkCount=cnt*2; 
	nextBlink=millis();
}

/**
check if we need to blink led 13
*/
void checkBlink()
{
	if (nextBlink != 0)
	{
		if (millis() >=nextBlink)
		{
			nextBlink=millis()+BLINKPERIOD;
			blnkCount--;
			if (blnkCount==0)
			{
				nextBlink=0;
			}
			digitalWrite(ledPin,blnkCount & 0x01);
		}
	}
}


void loop()
{
	checkBlink();
	if (qTouch.AT42QTchangeEvent)
	{
		qTouch.AT42QTchangeEvent=0;
		uint8_t status; 
		uint8_t keyset;
		status=qTouch.getRegValue(AT42QT1070::DETECTIONSTATUS);
		if (status & AT42QT1070::OVERFLOWBIT)
			Serial.println("Overflow");
		if (status & AT42QT1070::CALIBRATEBIT)
			Serial.println("Calibrating");
		keyset=qTouch.getRegValue(AT42QT1070::KEYSTATUS);
		uint8_t keyPressed=0;
		for (uint8_t i=0; i < AT42QT1070_MAXKEYS;i++)
		{
			if (keyset & (1<<(AT42QT1070_MAXKEYS-i-1)))
			{
				blinkNumber(i+1); // blink times of keynumber (keys 1..7, not 0..6)
				Serial.print("Key");
				Serial.print(i+1);
				keyPressed=1;
			}
		}
		if (!keyPressed)
		{
		
			blinkNumber(1); // blink once if key released
			Serial.print("No key active");
		}
		Serial.println("");
	}

}

