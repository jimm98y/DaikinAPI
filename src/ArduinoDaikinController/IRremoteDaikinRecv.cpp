// Copyright 2019 Jimm98y
// Copyright 2015 danny
// Arduino IRremote Daikin 2015

#include "IRremoteDaikinRecv.h"
#include <util/delay.h>

// decode
#define SAMPLE_DELAY_TIME 10//uS
#define IDLE_TIMER_COUNT ((1000*13)/SAMPLE_DELAY_TIME)//SAMPLE_DELAY_TIME*100*13
#define BUFFER_SIZE 310

//#define DEBUG_IR_PRINT
//#define DEBUG_IR_PRINT_DECODED_DATA

uint8_t irPin = 2;
uint8_t irState = 1;
uint8_t irLastState = 1;
uint16_t stopBitCounter = 0;
uint16_t timeoutCounter = 0;
uint16_t timeCounter = 0;
uint16_t packetCounter = 0;
uint8_t	packetLength = 3;
uint8_t packetNumber = 0;
uint8_t irStateBuf[2]= {0};
uint8_t	hasPacket = 0;
unsigned long irDurationBuf[2] = {0};
uint16_t idx = 0;
uint16_t bitCounter = 0;
uint8_t	irPatternStateMachine = 0;
uint8_t	irRawSateMachine = 0;
uint8_t	wakePatternCounter = 0;
uint8_t	irReceiveDataLen = 0;
uint8_t irReceiveDataBufferSize = 0;

uint8_t IRDaikinRecv::begin(uint8_t pin, uint8_t *buffer, uint8_t buffer_size) {
	irPin = pin;
	irReceiveDataP0 = buffer;
	memset(irReceiveDataP0,0,buffer_size);
	hasPacket = 0;
	packetCounter = 0;
	bitCounter = 0;
	irPatternStateMachine = 0;
	packetCounter = 0;
	wakePatternCounter = 0;
	packetLength = 3;
	irState = irLastState = digitalRead(irPin);
	irReceiveDataLen = 0;
	irReceiveDataBufferSize = buffer_size;
	return 1;
}

uint8_t IRDaikinRecv::decode() {
  for (;;) {
    irState = digitalRead(irPin);
    if (irState!=irLastState) {
      break;
    }
    irLastState = irState;
    return 0; // polling
  }

  // start sniffer
  timeCounter = 1;
  stopBitCounter = 0;
  timeoutCounter = 0;
  hasPacket = 0;
  irLastState = irState;
  idx = 0;

  // searching ir data
  while (1) {
    _delay_us(SAMPLE_DELAY_TIME);
    irState = digitalRead(irPin);

    if (irState!=irLastState) {
		if (irDataStoreBuffer() == 2) {
	      decodeIR(false);
		}
        timeCounter = 0;
        stopBitCounter = 0;
    }

    stopBitCounter ++;
    timeCounter++;
    timeoutCounter;
    irLastState = irState;

    if (stopBitCounter > IDLE_TIMER_COUNT) {
		if (irDataStoreBuffer() == 2) {
			if (decodeIR(true) > 0) {
				if (checkSum(irReceiveDataP0,irReceiveDataLen) == 0) {
#ifdef DEBUG_IR_PRINT
						Serial.println("===CRCFAIL===");
						break;
#endif
				} else {
					hasPacket = 1;
					break;
				}
			} else {
#ifdef DEBUG_IR_PRINT
				Serial.println("===FAIL===");
#endif
			}
		}
		hasPacket = 0;
		packetCounter =0;
		bitCounter = 0;
		irPatternStateMachine = 0;
		packetCounter = 0;
		wakePatternCounter = 0;
		packetLength = 3;
		irState = irLastState = digitalRead(irPin);
		memset(irReceiveDataP0, 0, 15);
		irReceiveDataLen = 0;
		break;
	}
  }

  if (hasPacket == 1) {
#ifdef DEBUG_IR_PRINT
      for (uint16_t i = 0;i < irReceiveDataBufferSize; i++) {
		  Serial.print(irReceiveDataP0[i], HEX);
		  Serial.print(" ");
      }
      Serial.println();
#endif

     if (irReceiveDataLen > 10) {
       printARCState(irReceiveDataP0);
	 }
		
	 hasPacket = 0;
     bitCounter = 0;
     irPatternStateMachine = 0;
     packetCounter = 0;
     wakePatternCounter = 0;
     packetCounter =0;
     return irReceiveDataLen;
   }
   return 0;
}

/*
 * 0:none 1:store to buffer 2:pair store to buffer
*/
uint8_t IRDaikinRecv::irDataStoreBuffer()
{
	if (irRawSateMachine == 0) {
		if (irLastState == 0) {
			irStateBuf[0] = irLastState;
			irDurationBuf[0] = timeCounter;
			irRawSateMachine = 1;
			return 1;
		}
		return 0;
	}

	if ( irRawSateMachine == 1) {
		if (irLastState == 1) {
			irStateBuf[1] = irLastState;
			irDurationBuf[1] = timeCounter;
			irRawSateMachine = 0;
			return 2;
		} else {
			irStateBuf[0] = irLastState;
			irDurationBuf[0] = timeCounter;
			irRawSateMachine = 1;
			return 1;
		}
		return 0;
	}
}

uint8_t IRDaikinRecv::decodeIR(bool lastBit) {

	if (irPatternStateMachine == 0) {
		//~ skip wake up pattern
		if (isZeroMatched(irDurationBuf[0], irDurationBuf[1]) == 1) {
			wakePatternCounter++;
		}

		if (lastBit == true) {
			if (wakePatternCounter > 4) {
				irPatternStateMachine = 1;
				wakePatternCounter = 0;
				packetLength = 3;
				packetCounter = -1;
				irReceiveDataLen = 0;
				memset(irReceiveDataP0,0, irReceiveDataBufferSize);
				return 1;
			}
			return 0;
		}

		//start bit
		if (isStartMatched(irDurationBuf[0], irDurationBuf[1]) == 1) {
			//some model no wake up pattern and only 2 packet
			packetLength = 1;
			irPatternStateMachine = 2;
			irReceiveDataLen = 0;
			bitToByteBuffer(irReceiveDataP0, 0, 1, NULL);
			memset(irReceiveDataP0, 0, irReceiveDataBufferSize);
			return 1;
		} else {
			return 0;
		}
	}

	if (irPatternStateMachine == 1) {
	  //~ skip start bit
	  if (isStartMatched(irDurationBuf[0], irDurationBuf[1]) == 1) {
		irPatternStateMachine = 2;
		bitToByteBuffer(irReceiveDataP0, 0, 1, NULL);
		return 1;
	  }
	  return 0;
	}
	if (irPatternStateMachine == 2) {//packet number 1
		if (lastBit == true){
			if (isStopMatched(irDurationBuf[0], irDurationBuf[1]) == 1) {
				irPatternStateMachine = 0;
				packetLength++;
				return 1;
			}else {
				irPatternStateMachine = 0;
				return 0;
			}
		}
		if (isZeroMatched(irDurationBuf[0], irDurationBuf[1]) == 1) {
			bitToByteBuffer(irReceiveDataP0, 0, 0, &irReceiveDataLen);
		}else {
			bitToByteBuffer(irReceiveDataP0, 1, 0, &irReceiveDataLen);
		}
		return 0;
	}
}

uint8_t IRDaikinRecv::checkSum(uint8_t *buffer,uint8_t len)
{
	// error here: len was 14, which is the last index, not the buffer length, therefore (len - 1) resulted in the last byte not being counted in
	uint8_t sum = 0;
	for (uint8_t i = 0; i < len; i++) {
		sum = (uint8_t)(sum + buffer[i]);
	}
	
	if (buffer[len] == sum) {
		return 1;
	}
	return 0;
}

void IRDaikinRecv::bitToByteBuffer(uint8_t *buffer, uint8_t value, int restart,uint8_t *bufferPtr) {
	static uint8_t bitIndex = 0;
	static uint8_t bufferIndex = 0;

	if (restart == 1) {
		bitIndex = 0;
		bufferIndex = 0;
		return;
	}

	if (bitIndex>7) {
		bufferIndex++;
		bitIndex = 0;
	} else {
		buffer[bufferIndex] = buffer[bufferIndex] >> 1;
	}

	if (value == 1) {
		buffer[bufferIndex] = buffer[bufferIndex] | 0x80;
	}

	bitIndex++;

	if (!(bufferPtr == NULL)) {
		*bufferPtr = bufferIndex;
	}
}

void IRDaikinRecv::printARCState(uint8_t *recvData) {
	/*
	11 DA 17 18 0 73 0 20 0 F E 35 0 20 1F 	// 16C // 14 // MIN
	11 DA 17 18 0 73 0 20 0 F 1C 35 0 20 2D // 23C // 28
	11 DA 17 18 0 73 0 20 0 F 1E 35 0 20 2F // 24C // 30
	11 DA 17 18 0 73 0 20 0 F 2E 35 0 20 3F // 32C // 46 // MAX
	*/
	uint8_t temperature = ((recvData[10] - 14) / 2) + 16;

	/*
	11 DA 17 18 0 73 0 20 0 F 18 15 0 20 9  // small fan
	11 DA 17 18 0 73 0 20 0 F 18 35 0 20 29 // medium fan
	11 DA 17 18 0 73 0 20 0 F 18 55 0 20 49 // large fan
	*/
	uint8_t fan = (recvData[11] & B01110000) >> 4;
	if (fan == 0x1) fan = 1;
	if (fan == 0x3) fan = 2;
	if (fan == 0x5) fan = 3;

	/*
	11 DA 17 18 0 73 0 21 0 F 18 15 0 20 A //on
	11 DA 17 18 0 73 0 20 0 F 18 15 0 20 9 //off
	11 DA 17 18 0 73 0 21 0 F 18 15 0 20 A //on
	*/
	uint8_t powerState = (recvData[7] & 0x01);

	/*
	11 DA 17 18 0 63 0 0 0 F 10 36 0 20 F2 // off
	11 DA 17 18 0 63 0 0 0 F 10 35 0 20 F1 // on
	11 DA 17 18 0 63 0 0 0 F 10 36 0 20 F2 // off
	*/
	uint8_t swing = (recvData[11] & 0x01);

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
	uint8_t timerOn = /* recvData[5] & B00001100 == 0x4; */ recvData[8] >> 7;
	// first bit indicates whether the time os on (1) or off (0)
	// the next 7 bits contain the value
	// on byte|off byte
	uint8_t timerOnValue = recvData[8] & B01111111;
	uint8_t timerOff = /* recvData[5] & B00001100 == 0x8; */ recvData[9] >> 7;
	uint8_t timerOffValue = recvData[9] & B01111111;

	//11 DA 17 18 looks like standard Daikin preamble
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
	uint8_t mode = (recvData[7] & B01110000) >> 4;

    // write currently decoded message to the output
	Serial.println("==DRB==");
	Serial.print("Power:");
	Serial.print(powerState,DEC);
	Serial.println();
	Serial.print("Mode:");
	Serial.print(mode,DEC);
	Serial.println();
	Serial.print("Fan:");
	Serial.print(fan,DEC);
	Serial.println();
	Serial.print("Temperature:");
	Serial.print(temperature,DEC);
	Serial.println();
	Serial.print("Swing:");
	Serial.print(swing,DEC);
	Serial.println();
	Serial.print("TimerOn:");
	Serial.print(timerOn,DEC);
	Serial.println();
	Serial.print("TimerOnValue:");
	Serial.print((timerOnValue),DEC);
	Serial.println();
	Serial.print("TimerOff:");
	Serial.print(timerOff, DEC);
	Serial.println();
	Serial.print("TimerOffValue:");
	Serial.print((timerOffValue),DEC);
    Serial.println();
    Serial.println("==DRE==");
}

uint8_t IRDaikinRecv::isOneMatched(uint16_t lowTimeCounter,uint16_t highTimecounter)
{
	if ((lowTimeCounter > 15 && lowTimeCounter < 50) && (highTimecounter > (lowTimeCounter + lowTimeCounter)  && highTimecounter < 200)) {
		return 1;
	}
	return 0;
}

uint8_t IRDaikinRecv::isZeroMatched(uint16_t lowTimeCounter,uint16_t highTimecounter)
{
	if ((lowTimeCounter > 15 && lowTimeCounter < 50) && (highTimecounter > 15 && highTimecounter < 50)) {
		return 1;
	}
	return 0;
}

uint8_t IRDaikinRecv::isStartMatched(uint16_t lowTimeCounter,uint16_t highTimecounter)
{
	if ((lowTimeCounter > 100 && lowTimeCounter < 400) && (highTimecounter > 70  && highTimecounter < 250)) {
		return 1;
	}
	return 0;
}

uint8_t IRDaikinRecv::isStopMatched(uint16_t lowTimeCounter,uint16_t highTimecounter)
{
	if ((lowTimeCounter > 15 && lowTimeCounter < 50) && (highTimecounter > 200)) {
		return 1;
	}
	return 0;
}
