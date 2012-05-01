/** Copyright Jurgen Schwietering
 * This library contains functions for the AT42QT1070
 * 
 * with some modifications you should make it run for 
 * AT42QT1060 (i2c address 0x12), see section 6.1 in the datasheet for the Address values

 */

#ifndef _AT42QT1070_h
#define _AT42QT1070_h

#include <Arduino.h>

#include <Wire.h>

#define POLLING_USED 1 //!< use this only if you are going to poll (it will need  a bit of sram memory)

/** you can also install your own handler to trigger what ever you want.
 but you should read registers 2 and 3 soon after
*/
extern "C" void AT42QT_CHANGEISR(void) __attribute__ ((signal));

#define AT42QT1070_MAXKEYS 7


class AT42QT1070
{
public: // some public definitions first
	typedef enum
	{
		CHIPID = 0, //!< should be 0x2E
		FIRMWARE= 1,  //!< could be 0x15 ;)
		DETECTIONSTATUS=2, //!<Bit7=calibrate, Bit6=Overflow, Bit0=Touch
		KEYSTATUS=3, //!< Bit=Key
		KEYSIGNAL=4, //!< MSB,LSB of signal 
		REFERENCE=18,//!<MSB,LSB of reference value (after calibration these change)
		// subsequent are R/W
		NEGTHRESHOLD=32, //!< don't set 0, default 20
		AVERAGE_ADJACENTKEYSUPPRESSIONLEVEl=39, 
			//  BIT7-BIT2 average of samples (allowed 1,2,4,8,16,32) default 8 
			// BIT2-BIT0=Define AKS group (0..3)
		DETECTIONINTEGRATORCOUNTER=46, // min is 2, number of consecutive threshold
		FOMODI_MAXCAL_GUARDCHANNEL=53, 
			// Bit5 Fas out DI , 
			//Bit4 calibrate all when Max On Reached if bit 0
			// Bit3-Bit0 Guard channel (0..6)
		LOWPOWERMODE=54, // 0=8ms,1=8ms,2=16ms,3=24ms, 4=32ms, 254=2.032,255=2.040 (default 2!)
		MAXONDURATION=55, // 0 = off 1=160ms, 2=320ms... 255=40.8 seconds
		CALIBRATE=56, // write non zero for calibration, check status register
		RESET=57, // write non zero for reset
	} QT1070ADR;

	typedef void (*ErrorCallback)(const uint8_t);

private:
    static const uint8_t AT42I2CAdr=0x1B; //!< this is constant for this chip
	
    ErrorCallback _errorCallback;
    uint8_t checkResult(uint8_t result);
    uint8_t setActiveAddress(uint8_t adr);

public: 
    static AT42QT1070 *mySingleSelf;
    volatile uint8_t AT42QTchangeEvent;
 
public:
    AT42QT1070();
    
    void setInternalISR(uint8_t pin); //!< pin 2 or 3 of arduino, sets the internal interrupt handler, you can set your own one if you need, the qt1070 does alwaus pull LOW the CHANGE line.
    
    void setErrorCallback(void (*cb)(uint8_t)); //!< you can set a callback function if I2C fails to work
    
    void reset(void);
    void calibrate(void);
    uint8_t isCalibrating(void);
    
    uint16_t readKeyValues(uint8_t key);
   
    void readKeySet();

    void setRegValue(uint8_t regAdr, uint8_t val);
    void setRegValuePreserved(uint8_t regAdr, uint8_t val, uint8_t maskPreserve);
    uint8_t getRegValue(uint8_t adr);
    void lowPowerMode(uint8_t val);
    void maxOnDuration(uint8_t val);
    
    /**! the next set of functions is for polling operations,
    these are only needed if you require more control on the buttons (multiple press at same time i.e.)
    they are intensive in means of cpu time
    after a calibration the refSet needs to be refreshed using completeDiffSet, if you fail to do that the changedDiffSet
    could be off.
    To understand that a calibration has been done you need to read the DETECTIONSTATUS registry or use the wrapper isCalibrating()
    */
#if POLLINGUSED
    uint16_t refSet[AT42QT1070_MAXKEYS]; //!< internal copy of ref set only used if using POLLING mode
    void completeDiffSet(int8_t *delta); //!< get difference of keyvalues delta[i]=actual[i]-reference[i]
    void changedDiffSet(int8_t *delta); //!< as completeDiffSet but suppose that reference did not change so it is not loaded
#endif

};


#endif //_AT42QT1070_h
