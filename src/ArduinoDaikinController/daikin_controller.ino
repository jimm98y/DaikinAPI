#include <SPI.h>
#include <UIPEthernet.h>

#include "IRremoteDaikinRecv.h"
#include "IRdaikinBRC.h"

#define DBGMSG(A) if (dbg){ Serial.print("DBG: "); Serial.println(A);}
#define IR_RECV_PIN 4
#define RECV_BUFFER_LENGTH 15
#define SERIAL_SPEED 9600
#define INITIAL_DELAY 500
#define MSG_BUFFER_LEN 200

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 55);
EthernetServer server(80);
char readMessage[MSG_BUFFER_LEN];

IRDaikinRecv irrecv;
IRdaikinBRC irsend;
boolean dbg = false;
uint8_t  irReceiveData[15] = { 0 };

void setup() {
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  server.begin();

  Serial.print("Server is listening on IP address: ");
  Serial.println(Ethernet.localIP());

  delay(INITIAL_DELAY);
	irrecv.begin(IR_RECV_PIN, irReceiveData, RECV_BUFFER_LENGTH);

  DBGMSG(F("IR listening"));
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    DBGMSG(F("New client:"));
    boolean currentLineEmpty = true;

    int readMessageIndex = 0;
    memset(readMessage, 0, MSG_BUFFER_LEN * sizeof(char));

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if(dbg) {
           Serial.write(c);
        }

        if(readMessageIndex < (MSG_BUFFER_LEN - 1)) {
           readMessage[readMessageIndex++] = c;
        }
  
        if (c == '\n' && currentLineEmpty) {
          // HTTP header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json;charset=utf-8");
          client.println("Connection: close");
          client.println();

          // get content
          uint8_t temperature = 0;
          uint8_t fan = 0;
          uint8_t powerState = 0;
  	      uint8_t swing = 0;
          uint8_t timerOn = 0;
          uint8_t timerOnValue = 0;
          uint8_t timerOff = 0;
          uint8_t timerOffValue = 0;
          uint8_t mode = 0;
          irsend.getState(&temperature, &fan, &powerState, &swing, &timerOn, &timerOnValue, &timerOff, &timerOffValue, &mode);

          // HTTP content
          client.print("{ ");

          client.print("\"timestamp\": ");
          client.print(millis());
          client.print(",");

          client.print("\"temperature\": ");
          client.print(temperature);
          client.print(",");

          client.print("\"fan\": ");
          client.print(fan);
          client.print(",");

          client.print("\"powerState\": ");
          client.print(powerState);
          client.print(",");

          client.print("\"swing\": ");
          client.print(swing);
          client.print(",");

          client.print("\"timerOn\": ");
          client.print(timerOn);
          client.print(",");

          client.print("\"timerOnValue\": ");
          client.print(timerOnValue);
          client.print(",");

          client.print("\"timerOff\": ");
          client.print(timerOff);
          client.print(",");

          client.print("\"timerOffValue\": ");
          client.print(timerOffValue);
          client.print(",");

          client.print("\"mode\": ");
          client.print(mode);
          
          client.println(" }");
          break;
        }

        if (c == '\n') {
          currentLineEmpty = true;
        } else if (c != '\r') {
          currentLineEmpty = false;
        }
      }
    }

    client.stop();

    DBGMSG(F("Client disconnected."));
    DBGMSG(F("---------------------"));

    int isCommand = 0; 
    char* f = 0;

    f = strstr(readMessage, "power=");
    if (f) {
      if (strncmp(f + 6, "on", 2) == 0) {
        DBGMSG(F("Power on"));
        irsend.daikin_on();
        isCommand++;
      }
      else if (strncmp(f + 6, "off", 3) == 0) {
        DBGMSG(F("Power off"));
        irsend.daikin_off();
        isCommand++;
      }
    }

    f = strstr(readMessage, "swing=");
    if (f) {
      if (strncmp(f + 6, "on", 2) == 0) {
        DBGMSG(F("Swing on"));
        irsend.daikin_setSwing_on();
        isCommand++;
      }
      else if (strncmp(f + 6, "off", 3) == 0) {
        DBGMSG(F("Swing off"));
        irsend.daikin_setSwing_off();
        isCommand++;
      }
    }

    f = strstr(readMessage, "temp=");
    if(f) {
      int temp = atoi(f + 5);
      if (temp >= 16 && temp <= 32) {
        DBGMSG(F("Set temperature"));
        irsend.daikin_setTemp(temp);
        isCommand++;
      }
      else {
        DBGMSG(F("out of range.  16 <= temp <= 32"));
      }
    }

    f = strstr(readMessage, "fan=");
    if(f) {
      int spd = atoi(f + 4);
      if (spd >= 0 && spd <= 2) {
        DBGMSG(F("Set fan"));
        irsend.daikin_setFan(spd);
        isCommand++;
      }
      else {
        DBGMSG(F("out of range.  0 <= speed <= 2"));
      }
    }

    f = strstr(readMessage, "mode=");
    if(f) {
      int mode = atoi(f + 5);
      if (mode >= 0 && mode <= 4) {
        DBGMSG(F("Set mode"));
        irsend.daikin_setMode(mode);
        isCommand++;
      }
      else {
        DBGMSG(F("out of range.  0 <= mode <= 4"));
      }
    }

    if(isCommand > 0) {
      DBGMSG(F("Send command"));
	    irsend.daikin_sendCommand();
    }

    DBGMSG(F("---------------------"));
  }

  // decode received IR
	decode();
}

uint8_t decode() {
	int buffer_size = 0;
	if ((buffer_size = irrecv.decode()) > 10) {
		// we received a new signal from the remote, update the send buffer
		irsend.updateBRC(irrecv.irReceiveDataP0, buffer_size);
		return 1;
	}
	return 0;
}
