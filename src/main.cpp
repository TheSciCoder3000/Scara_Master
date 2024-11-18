#include <Wire_Master.h>
#include "Arduino.h"

#define LM1 17
#define LM2 16
#define LM3 33
#define LM4 32

#define SLAVE_ADD 69
WireMaster comm;

void setup()
{
  comm.begin();
  pinMode(LM1, INPUT_PULLUP);
  pinMode(LM2, INPUT_PULLUP);
  pinMode(LM3, INPUT_PULLUP);
  pinMode(LM4, INPUT_PULLUP);
  Serial.begin(9600); // start serial for output
}

void loop()
{

  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');

    if (command == "who u?")
    {
      Serial.println("Ako si: Master");
    }
    else if (command == "calibrate")
    {
      comm.transmit("STEP3CC300");
      delay(2000);
      comm.transmit("STEP3CL300");
    }
    else
    {
      comm.transmit(command);
    }
  }
}

void calibrate()
{
}

// SERV
// 	SERV
// 		SERV90

// STEP
// 	STEPCL
// 	STEPCC
// 		STEPCC100

// RESO
// 	RESO111