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

volatile uint8_t touchEvent=0;

static void touchEventISR()
{
	touchEvent=1;	
}

void setKeyHitISR(uint8_t pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH); // pull high, CHANGE is open drain
  attachInterrupt(pin-2,touchEventISR,FALLING); // on active falls low
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
	setKeyHitISR(2);
  	qTouch.reset();
	qTouch.maxOnDuration(0); // no stuck keys (we want full fingers)
	qTouch.lowPowerMode(4); // full power, let's save a bit if gasoline
  	qTouch.calibrate();
  	while(qTouch.isCalibrating())
  	{
  		Serial.print(".");
  		delay(10);
  	}
}


#define BLINKPERIOD 100
static uint8_t blnkCount=0;
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
	if (touchEvent)
	{
		touchEvent=0;
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

