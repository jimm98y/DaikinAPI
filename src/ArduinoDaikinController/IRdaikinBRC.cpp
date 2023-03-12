// Copyright 2019 Jimm98y
// Copyright 2015 danny
// Arduino IRremote Daikin 2015

#include "IRdaikinBRC.h"

// http://www.daikin.co.uk/contacts-and-downloads/operation-manuals/air-conditioning/
// http://www.daikin.co.uk/binaries/OM%20-%20BRC4C61~4_3P107422-21S_EN_tcm511-242366.pdf

// # of bytes per command
const int COMMAND_LENGTH_BRC = 22;
unsigned char daikinBRC[COMMAND_LENGTH_BRC] = {
	0x11, 0xDA, 0x17, 0x18,   0x04, 0x00,    0x1E,
	0x11, 0xDA, 0x17, 0x18,   0x00, 0x73, 0x00, 0x20, 0x02, 0x2, 0x2E, 0x36, 0x00, 0x20,    0x35
};

                            // low, medium, high
static byte vFanTableBRC[] = { 0x10, 0x30, 0x50 };

                                // fan, sun, cool, auto, rain
static byte vModeTableBRC12[] = { 0x60, 0x70, 0x70, 0x70, 0x20  };
static byte vModeTableBRC14[] = { 0x00, 0x10, 0x20, 0x30, 0x70  };

                              //16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
static byte vTempTableBRC[] = { 14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46 };

int IRpin;

void IRdaikinBRC::daikin_on() {
	daikinController_on();
}

void IRdaikinBRC::daikin_off() {
	daikinController_off();
}

void IRdaikinBRC::daikin_setSwing_on() {
	daikinBRC[18] &= 0xfc;
	daikinBRC[18] |= 0x01;
	daikinController_checksum();
}

void IRdaikinBRC::daikin_setSwing_off() {
	daikinBRC[18] &= 0xfc;
	daikinBRC[18] |= 0x02;
	daikinController_checksum();
}

void IRdaikinBRC::daikin_setMode(int mode) {
	if (mode >= 0 && mode <= 4) {
		daikinController_setMode(mode);
	}
}

void IRdaikinBRC::daikin_setFan(int speed) {
	if (speed >= 0 && speed <= 2) {
		daikinController_setFan(vFanTableBRC[speed]);
	}
}

void IRdaikinBRC::daikin_setTemp(uint8_t temp) {
	daikinController_setTemp(temp);
}

void IRdaikinBRC::daikin_sendCommand() {
	sendDaikinCommand();
}

uint8_t IRdaikinBRC::daikinController_checksum() {
	uint8_t sum = 0;
	uint8_t i;

	// checksum first part of the message
	for (i = 0; i <= 5; i++) {
		sum += daikinBRC[i];
	}

	daikinBRC[6] = sum & 0xFF;

	// checksum second part of the message
	sum = 0;
	for (i = 7; i <= 20; i++) {
		sum += daikinBRC[i];
	}

	daikinBRC[21] = sum & 0xFF;
}

void IRdaikinBRC::dump() {
	uint8_t i;
	for (i = 0; i <COMMAND_LENGTH_BRC; i++) {
		Serial.print(daikinBRC[i], HEX);
		Serial.print("-");
	}
}

void IRdaikinBRC::daikinController_on() {
	daikinBRC[14] |= 0x01;
	daikinController_checksum();
}

void IRdaikinBRC::daikinController_off() {
	daikinBRC[14] &= 0xFE;
	daikinController_checksum();
}

void IRdaikinBRC::daikinController_setTemp(uint8_t temp) {
	if (temp >= 16 && temp <= 32) 	{
		temp = temp - 16;
		daikinBRC[17] = vTempTableBRC[temp];
		daikinController_checksum();
	}
}

void IRdaikinBRC::daikinController_setFan(uint8_t fan) {
	daikinBRC[18] &= 0x8f;
	daikinBRC[18] |= fan;

	daikinController_checksum();
}

uint8_t IRdaikinBRC::daikinController_getState() {
	return (daikinBRC[14]) & 0x01; // power on/off?
}

void IRdaikinBRC::daikinController_setMode(uint8_t mode) {
	if (mode >= 0 && mode <= 4)
	{
		daikinBRC[12] &= 0x8f;
		daikinBRC[12] |= vModeTableBRC12[mode];
		daikinBRC[13] = 4; // for some reason it is set to 4 when changing the mode
		daikinBRC[14] &= 0x8f;
		daikinBRC[14] |= vModeTableBRC14[mode];
		daikinController_checksum();
	}
}

void IRdaikinBRC::sendDaikinCommand() {
	daikinController_checksum();
	sendDaikin(daikinBRC, 7, 0);
	delay(29);
	sendDaikin(daikinBRC, 15, 7);
	printARCState(daikinBRC + 7);
}

void IRdaikinBRC::sendDaikin(unsigned char buf[], int len, int start) {
	int data2;
	enableIROut(38);
	mark(DAIKIN_HDR_MARK);
	space(DAIKIN_HDR_SPACE);

	for (int i = start; i < start + len; i++) {
		data2 = buf[i];

		for (int j = 0; j < 8; j++) {
			if ((1 << j & data2)) {
				mark(DAIKIN_ONE_MARK);
				space(DAIKIN_ONE_SPACE);
			}
			else {
				mark(DAIKIN_ZERO_MARK);
				space(DAIKIN_ZERO_SPACE);
			}
		}
	}

	mark(DAIKIN_ONE_MARK);
	space(DAIKIN_ZERO_SPACE);
}

void IRdaikinBRC::sendDaikinWake() {
	enableIROut(38);
	space(DAIKIN_ZERO_MARK);

	mark(DAIKIN_ZERO_MARK);
	space(DAIKIN_ZERO_MARK);

	mark(DAIKIN_ZERO_MARK);
	space(DAIKIN_ZERO_MARK);

	mark(DAIKIN_ZERO_MARK);
	space(DAIKIN_ZERO_MARK);

	mark(DAIKIN_ZERO_MARK);
	space(DAIKIN_ZERO_MARK);

	mark(DAIKIN_ZERO_MARK);
	space(DAIKIN_ZERO_MARK);
}

void IRdaikinBRC::sendRaw(unsigned int buf[], int len, int hz) {
	enableIROut(hz);
	for (int i = 0; i < len; i++) {
		if (i & 1) {
			space(buf[i]);
		}
		else {
			mark(buf[i]);
		}
	}
	space(0); // just to be sure
}

void IRdaikinBRC::setPin(int pin) {
	pinMode(IRpin, OUTPUT);
	digitalWrite(IRpin, LOW); // when not sending PWM, we want it low
	IRpin = pin;
}

void IRdaikinBRC::mark(int time) {
	// Sends an IR mark for the specified number of microseconds.
	// The mark output is modulated at the PWM frequency.
	TIMER_ENABLE_PWM; // Enable pin 3 PWM output
	delayMicroseconds(time);
}

/* Leave pin off for time (given in microseconds) */
void IRdaikinBRC::space(int time) {
	// Sends an IR space for the specified number of microseconds.
	// A space is no output, so the PWM output is disabled.
	TIMER_DISABLE_PWM; // Disable pin 3 PWM output
	delayMicroseconds(time);
}

void IRdaikinBRC::enableIROut(int khz) {
	// Enables IR output. The khz value controls the modulation frequency in kilohertz.
	// The IR output will be on pin 3 (OC2B).
	pinMode(TIMER_PWM_PIN, OUTPUT);

	// The top value for the timer. The modulation frequency will be SYSCLOCK / 2 / OCR2A.
	TIMER_CONFIG_KHZ(khz);
}

void IRdaikinBRC::updateBRC(uint8_t* buffer, int buffer_size) {
  uint8_t i = 4;
  for(i = 4; i < buffer_size; i++)   {
	
#ifdef DEBUG_IR_PRINT
     Serial.print(buffer[i], HEX);
     Serial.print(" ");
#endif

     // update the BRC
     daikinBRC[7 + i] = buffer[i];
   }

#ifdef DEBUG_IR_PRINT
   Serial.println();
#endif
}

void IRdaikinBRC::getState(
    uint8_t* temperature,
    uint8_t* fan, 
    uint8_t* powerState,
  	uint8_t* swing, 
    uint8_t* timerOn, 
    uint8_t* timerOnValue, 
    uint8_t* timerOff,
    uint8_t* timerOffValue,
    uint8_t* mode) {
    uint8_t* recvData = daikinBRC + 7;

	/*
	11 DA 17 18 0 73 0 20 0 F E 35 0 20 1F 	// 16C // 14 // MIN
	11 DA 17 18 0 73 0 20 0 F 1C 35 0 20 2D // 23C // 28
	11 DA 17 18 0 73 0 20 0 F 1E 35 0 20 2F // 24C // 30
	11 DA 17 18 0 73 0 20 0 F 2E 35 0 20 3F // 32C // 46 // MAX
	*/
	*temperature = ((recvData[10] - 14) / 2) + 16;

	/*
	11 DA 17 18 0 73 0 20 0 F 18 15 0 20 9  // small fan
	11 DA 17 18 0 73 0 20 0 F 18 35 0 20 29 // medium fan
	11 DA 17 18 0 73 0 20 0 F 18 55 0 20 49 // large fan
	*/
	*fan = (recvData[11] & B01110000) >> 4;
	if (fan == 0x1) fan = 1;
	if (fan == 0x3) fan = 2;
	if (fan == 0x5) fan = 3;

	/*
	11 DA 17 18 0 73 0 21 0 F 18 15 0 20 A //on
	11 DA 17 18 0 73 0 20 0 F 18 15 0 20 9 //off
	11 DA 17 18 0 73 0 21 0 F 18 15 0 20 A //on
	*/
	*powerState = (recvData[7] & 0x01);

	/*
	11 DA 17 18 0 63 0 0 0 F 10 36 0 20 F2 // off
	11 DA 17 18 0 63 0 0 0 F 10 35 0 20 F1 // on
	11 DA 17 18 0 63 0 0 0 F 10 36 0 20 F2 // off
	*/
	*swing = (recvData[11] & 0x01);

	/*
	11 DA 17 18 0 8 0 20 48 81 1A 35 0 20 7A // 1 off
	11 DA 17 18 0 C 0 20 48  1 1A 35 0 20 FE // 1 off cancel
	11 DA 17 18 0 4 0 20 81 81 1A 35 0 20 AF // 1 off 1 on
	11 DA 17 18 0 C 0 20  1  1 1A 35 0 20 B7 // cancel
	11 DA 17 18 0 4 0 20 81  1 1A 35 0 20 2F // 1 on
	11 DA 17 18 0 C 0 20  1  1 1A 35 0 20 B7 // 1 on cancel

	11 DA 17 18 0 8 0 20  1 82 1A 35 0 20 34 // 2 off
	11 DA 17 18 0 C 0 20  1  2 1A 35 0 20 B8 // 2 off cancel
	11 DA 17 18 0 4 0 20 82 82 1A 35 0 20 B1 // 2 off 2 on
	11 DA 17 18 0 C 0 20  2  2 1A 35 0 20 B9 // 2 off 2 on cancel
	11 DA 17 18 0 4 0 20 82  2 1A 35 0 20 31 // 2 on
	11 DA 17 18 0 C 0 20  2  2 1A 35 0 20 B9 // 2 on cancel
	*/

	// the fifth byte is either 0000 0100 (timer on action), 0000 1000 (timer off action), C 0000 1100 (cancel) or 0000 0000 (no timer)
	*timerOn = /* recvData[5] & B00001100 == 0x4; */ recvData[8] >> 7;
	// first bit indicates whether the time os on (1) or off (0)
	// the next 7 bits contain the value

	// on byte|off byte
	*timerOnValue = recvData[8] & B01111111;
	*timerOff = /* recvData[5] & B00001100 == 0x8; */ recvData[9] >> 7;
	*timerOffValue = recvData[9] & B01111111;

	// 11 DA 17 18 looks like standard Daikin preamble
	/*
	11 DA 17 18 0 63 4  0 0 F 10 35 0 20 F5 // fan // 63!!!
	11 DA 17 18 0 73 4 10 0 F 18 35 0 20 1D // sun
	11 DA 17 18 0 73 4 20 0 F 18 35 0 20 2D // snowflake
	11 DA 17 18 0 23 4 70 0 F 10 35 0 20 25 // rain // 23!!!
	11 DA 17 18 0 73 4 10 0 F 18 35 0 20 1D // sun
	11 DA 17 18 0 73 4 30 0 F 1A 35 0 20 3F // Auto M:

	11 DA 17 18 0 73 0 30 2 2 20 35 0 20 36 // H
	11 DA 17 18 0 73 0 30 2 2 1C 35 0 20 32 // H-
	11 DA 17 18 0 73 0 30 2 2 1A 35 0 20 30 // M
	11 DA 17 18 0 73 0 30 2 2 18 35 0 20 2E // M-
	11 DA 17 18 0 73 0 30 2 2 14 35 0 20 2A // L
	*/
	*mode = (recvData[7] & B01110000) >> 4;
}

void IRdaikinBRC::printARCState(uint8_t *recvData) {
	uint8_t temperature = 0;
    uint8_t fan = 0;
    uint8_t powerState = 0;
    uint8_t swing = 0;
    uint8_t timerOn = 0;
    uint8_t timerOnValue = 0;
    uint8_t timerOff = 0;
    uint8_t timerOffValue = 0;
    uint8_t mode = 0;
    getState(&temperature, &fan, &powerState, &swing, &timerOn, &timerOnValue, &timerOff, &timerOffValue, &mode);

	// write currently decoded message to the output
	Serial.println("==DRB==");
	Serial.print("Power:");
	Serial.print(powerState, DEC);
	Serial.println();
	Serial.print("Mode:");
	Serial.print(mode, DEC);
	Serial.println();
	Serial.print("Fan:");
	Serial.print(fan, DEC);
	Serial.println();
	Serial.print("Temperature:");
	Serial.print(temperature, DEC);
	Serial.println();
	Serial.print("Swing:");
	Serial.print(swing, DEC);
	Serial.println();
	Serial.print("TimerOn:");
	Serial.print(timerOn, DEC);
	Serial.println();
	Serial.print("TimerOnValue:");
	Serial.print((timerOnValue), DEC);
	Serial.println();
	Serial.print("TimerOff:");
	Serial.print(timerOff, DEC);
	Serial.println();
	Serial.print("TimerOffValue:");
	Serial.print((timerOffValue), DEC);
	Serial.println();
	Serial.println("==DRE==");
}
