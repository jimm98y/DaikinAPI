/*
 * Arduino IRremote Daikin 2015
 * Copyright 2015 danny
 *
 *
 * Arduino PWM declare base on  Ken Shirriff's IRremote library.
 * http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 *
 */

#ifndef IRdaikinBRC_h
#define IRdaikinBRC_h

#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy
#include "IRremoteIntDaikin.h"

class IRdaikinBRC
{
public:
  void daikin_on();
	void daikin_off();
	void daikin_setSwing_on();
	void daikin_setSwing_off();
	void daikin_setMode(int mode);//
	void daikin_setFan(int speed);// 0~1 Hi,Low
	void daikin_setTemp(uint8_t temp);//16 ~ 32
	void daikin_sendCommand();
	void dump();

	void setPin(int pin);
	void sendRaw(unsigned int buf[], int len, int hz);
	void sendDaikin(unsigned char buf[], int len, int start);
	void sendDaikinWake();
	void enableIROut(int khz);
  void updateBRC(uint8_t* buffer, int buffer_size);
  void getState(
    uint8_t* temperature,
    uint8_t* fan, 
    uint8_t* powerState,
  	uint8_t* swing, 
    uint8_t* timerOn, 
    uint8_t* timerOnValue, 
    uint8_t* timerOff,
    uint8_t* timerOffValue,
    uint8_t* mode);

private:
	void daikinController_on();
	void daikinController_off();
	void daikinController_setTemp(uint8_t temp);
	void daikinController_setFan(uint8_t fan);
	void daikinController_setMode(uint8_t mode);
	void printARCState(uint8_t *recvData);
	void sendDaikinCommand();
	uint8_t daikinController_checksum();
	uint8_t daikinController_getState();
	void mark(int usec);
	void space(int usec);
};

#endif
