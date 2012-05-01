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
  	pinMode(13,OUTPUT);

  	printValue("ChipID=",qTouch.getRegValue(AT42QT1070::CHIPID));
	setKeyHitISR(2);
  	qTouch.reset();
	qTouch.maxOnDuration(0); // no stuck keys (we want full fingers)
	qTouch.lowPowerMode(0); // full power
  	qTouch.calibrate();
  	while(qTouch.isCalibrating())
  	{
  		Serial.print(".");
  		delay(10);
  	}
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


void loop()
{
	if (touchEvent)
	{
		touchEvent=0;
		uint8_t status; 
		uint8_t keyset;
		status=qTouch.getRegValue(AT42QT1070::DETECTIONSTATUS);
		keyset=qTouch.getRegValue(AT42QT1070::KEYSTATUS);
		printBitSet(status);
		Serial.print(" ");	
		printBitSet(keyset);
		Serial.print("     ");	
		int8_t delta[AT42QT1070_MAXKEYS];
		qTouch.completeDiffSet(delta);
		for (uint8_t i=0; i < AT42QT1070_MAXKEYS;i++)
		{
			Serial.print(delta[i]);
			Serial.print(",");
		}
		Serial.println("");
	}
	/*
 		Wire.beginTransmission(0x1B); // transmit to device
		Wire.write(0x3); // want to read detection status // set pointer
		int result=Wire.endTransmission(); // stop transmitting
		switch(result)
		{
			case 0: break;
			case 1: Serial.print(F("data too long ")); break;
			case 2: Serial.print(F("received NACK on address ")); Wire.begin();break;
			case 3: Serial.print(F("recevied NACK on transmit ")); Wire.begin();break;
			case 4: Serial.print(F("unknown err. "));Wire.begin(); break;
				
		}
		Wire.requestFrom(0x1B, 1); 
		if(Wire.available()==1)
		{
			char outb[32];
			int status = Wire.read();
			for (int i=0; i < 8; i++)
			{
				if (status & (1 <<i))
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
 
		Wire.beginTransmission(0x1B); // transmit to device
		Wire.write(0x4); // want to read detection status // set pointer
		result=Wire.endTransmission(); // stop transmitting
		switch(result)
		{
			case 0: break;
			case 1: Serial.print("data too long "); break;
			case 2: Serial.print("received NACK on address "); Wire.begin();break;
			case 3: Serial.print("recevied NACK on transmit "); Wire.begin();break;
			case 4: Serial.print("unknown err. ");Wire.begin(); break;
				
		}
		Wire.requestFrom(0x1B, 28); 
		if(Wire.available()==28)
      	{
			for (int i=0; i < 14; i++)
			{
				char outb[32];
			    int msb = Wire.read();
        		int lsb = Wire.read();
        		unsigned int val= (msb<<8)+lsb;
        		sprintf(outb, "%4x ", val);
        		Serial.print(outb);
			}
			Serial.println("");
		}
		else
		{
			char outb[32];
			sprintf(outb,"%d ", Wire.available());
			Serial.print(outb);
			Serial.println("I2C not ok!");
		}
		result=Wire.endTransmission();
		switch(result)
		{
			case 0: break;
			case 1: Serial.print("data too long "); break;
			case 2: Serial.print("received NACK on address "); Wire.begin();break;
			case 3: Serial.print("recevied NACK on transmit ");Wire.begin(); break;
			case 4: Serial.print("unknown err. "); Wire.begin();break;
				
		}

	delay(200);
	*/
}

