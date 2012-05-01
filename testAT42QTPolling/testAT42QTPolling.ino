/************************************************************************************
* 
* Name : AT42QT test
* Author : Jurgen Schwietering
* Date : 2012-04-30
* Version : 0.0

this test program uses polling and interrupt to read pressed keys

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
				Serial.print(F("unknown err. (chip no connecte?)")); 
				break;
	}

}

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
  	pinMode(13,OUTPUT);

  	printValue("ChipID=",qTouch.getRegValue(AT42QT1070::CHIPID));
	setKeyHitISR(2);
  	qTouch.reset();
  	qTouch.calibrate();
  	while(qTouch.isCalibrating())
  	{
  		Serial.print(".");
  		delay(10);
  	}
	qTouch.maxOnDuration(0); // no stuck keys (we want full fingers)
	qTouch.lowPowerMode(0); // full power
 	Serial.print("max on duration:");
  	Serial.println(qTouch.getRegValue(AT42QT1070::MAXONDURATION));
  	Serial.print("low power mode read:");
  	Serial.println(qTouch.getRegValue(AT42QT1070::LOWPOWERMODE));
  	qTouch.saveReferenceSet();	
	delay(100);
}



void printBitSet(uint8_t v)
{     
	char outb[9];
	for (uint8_t i=0; i < 8; i++)
	{
		if (v & (1 <<(7-i)))
		{
			outb[i]='-';
		}
		else
		{
			outb[i]='X';
		}
	}
	outb[8]=0;
	Serial.print(outb);
}

int8_t delta[AT42QT1070_MAXKEYS]; // could be negative

uint8_t lastSet[AT42QT1070_MAXKEYS];
uint8_t currentSet[AT42QT1070_MAXKEYS];
uint16_t t3Last=0; // ternery value

void ShowKeySituation()
{
	for (uint8_t i=0; i < AT42QT1070_MAXKEYS; i++)
	{
		switch (currentSet[i])
		{
			case 2:
				Serial.print("X  ");
				break;
			case 1:
				Serial.print("-  ");
				break;
			case 0:
				Serial.print(".  ");
				break;
		}
	
	}
	
	for (uint8_t i=0; i < AT42QT1070_MAXKEYS; i++)
	{
		char outb[8];
		sprintf(outb,"%+4d ", delta[i]);
		Serial.print(outb);
	
	}
	
	Serial.println("");
}

uint8_t countzero=0;

void CheckForForcedCalibration()
{
	if (t3Last==0)
	{
		countzero++;
	}
	if (countzero>100)
	{
		Serial.println("Calibrating");
	 	qTouch.calibrate();
	  	while(qTouch.isCalibrating())
	  	{
	  		Serial.print(".");
	  		delay(10);
	  	}	
	  	countzero=0;
  	}
}


uint32_t readsDone=0;
unsigned long nextStat=0;

void loop()
{
	readsDone++;
	if (millis() >nextStat)
	{
		nextStat=millis()+10000;
		char outb[32];
		sprintf(outb,"%ul.%d reads per second", readsDone/10,readsDone%10);
		Serial.println(outb);
		readsDone=0;
	}
	if (touchEvent)
	{
		touchEvent=0;
		uint8_t status; 
		uint8_t keyset;
		status=qTouch.getRegValue(AT42QT1070::DETECTIONSTATUS);
		keyset=qTouch.getRegValue(AT42QT1070::KEYSTATUS);
		if (status & AT42QT1070::CALIBRATEBIT)
		{
			Serial.println("Calibrating");
			qTouch.saveReferenceSet();
		}
	}
	qTouch.changedDiffSet(delta);
	uint16_t t3=0;
	uint16_t m=3;
	for (uint8_t i=0; i < AT42QT1070_MAXKEYS; i++)
	{
		if (delta[i]>80)
		{
			currentSet[i]=2;
			t3+=m*2;
		}
		else
		if (delta[i]>40)
		{
			currentSet[i]=1;
			t3+=m;
		}
		else
			currentSet[i]=0;
		m*=3;
	}
	if (t3!=t3Last)
	{
		CheckForForcedCalibration();
		char outb[8];
		sprintf(outb,"%5d:", t3);
		Serial.print(outb);
		ShowKeySituation();
		t3Last=t3;
	}
}

