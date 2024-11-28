#include <Wire_Master.h>
#include "Arduino.h"

#define LM1 5
#define LM2 17
#define LM3 33
#define LM4 32

#define BUT1 16
#define BUT2 4
#define BUT3 15

#define ROTA 14
#define ROTB 13

#define X2 34
#define X1 39
#define Y2 27
#define Y1 26

#define SLAVE_ADD 69
bool displayLimits = false;
bool enabledJoystick = false;

int upperMinimum = 4095 - 100;
int lowerMinimum = 100;

int rotaryCounter = 0;
int aState;
int aLastState;

WireMaster comm;
void checkLimits();
void checkButtons();
void checkRotary();
void setup()
{
  comm.begin();
  pinMode(LM1, INPUT_PULLUP);
  pinMode(LM2, INPUT_PULLUP);
  pinMode(LM3, INPUT_PULLUP);
  pinMode(LM4, INPUT_PULLUP);

  pinMode(X1, INPUT);
  pinMode(X2, INPUT);
  pinMode(Y1, INPUT);
  pinMode(Y2, INPUT);

  pinMode(BUT1, INPUT);
  pinMode(BUT2, INPUT);

  pinMode(ROTA, INPUT);
  pinMode(ROTB, INPUT);
  aLastState = digitalRead(ROTA);
  Serial.begin(9600); // start serial for output
}

void loop()
{
  if (displayLimits)
    Serial.println("LM Base: " + String(digitalRead(LM1)) + "\t| LM Top: " + String(digitalRead(LM2)) + "\t| LM Arm: " + String(digitalRead(LM3)) + "\t| LM Grip: " + String(digitalRead(LM4)));

  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');

    if (command == "who u?")
    {
      Serial.println("Ako si: Master");
    }
    else if (command == "toggle limit")
    {
      displayLimits = !displayLimits;
    }
    else if (command == "calibrate")
    {
      // comm.transmit("STEP3CC300");

      for (int i = 0; i < 600; i++)
      {
        comm.transmit("STEP3CC1");
      }

      delay(2000);

      for (int i = 0; i < 600; i++)
      {
        comm.transmit("STEP3CL1");
      }

      // comm.transmit("STEP3CL300");
    }
    else if (command == "toggle joystick")
    {
      Serial.println("Joystick Enabled!");
      enabledJoystick = !enabledJoystick;
    }
    else
    {
      Serial.println("Transmit command: " + String(command));
      comm.transmit(command);
    }
  }

  if (enabledJoystick)
  {
    int joystick1_X = analogRead(X1);
    int joystick1_Y = analogRead(X2);
    int joystick2_X = analogRead(Y1);
    int joystick2_Y = analogRead(Y2);

    int joystick1_dirX = map(joystick1_X, 0, 4095, -100, 100) / 100;
    int joystick1_dirY = map(joystick1_Y, 0, 4095, -100, 100) / 100;
    int joystick2_dirX = map(joystick2_X, 0, 4095, -100, 100) / 100;
    int joystick2_dirY = map(joystick2_Y, 0, 4095, -100, 100) / 100;

    if (joystick1_dirX == -1)
      comm.transmit("STEP1CC10");
    if (joystick1_dirX == 1)
      comm.transmit("STEP1CL10");
    if (joystick1_dirY == -1)
      comm.transmit("STEP2CC10");
    if (joystick1_dirY == 1)
      comm.transmit("STEP2CL10");

    if (joystick2_dirX == -1)
      comm.transmit("STEP3CC10"); // ARM
    if (joystick2_dirX == 1)
      comm.transmit("STEP3CL10");
    if (joystick2_dirY == -1)
      comm.transmit("STEP4CC10");
    if (joystick2_dirY == 1)
      comm.transmit("STEP4CL10");

    // Serial.println("J1 X: " + String(joystick1_dirX) + "J1 Y: " + String(joystick1_dirY));
    // Serial.println("J1X: " + String(joystick1_X) + " | J1Y: " + String(joystick1_Y) + " | J2X: " + String(joystick2_X) + " | J2Y: " + String(joystick2_Y));
  }

  // checkLimits();
  // checkButtons();
  checkRotary();
}

void checkRotary()
{
  aState = digitalRead(ROTA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState)
  {
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(ROTB) != aState)
    {
      rotaryCounter++;
    }
    else
    {
      rotaryCounter--;
    }

    if (rotaryCounter > 180)
      rotaryCounter = 180;
    else if (rotaryCounter < 0)
      rotaryCounter = 0;

    comm.transmit("SERV" + String(rotaryCounter));
  }
  aLastState = aState; // Updates the previous state of the outputA with the current state
}

void checkLimits()
{
  if (digitalRead(LM1) == LOW)
  {
    Serial.println("stopping 1: " + String(digitalRead(LM1)));
    // comm.transmit("STOP1");
  }

  // if (digitalRead(LM2) == HIGH)
  // {
  //   Serial.println("stopping 2 " + String(digitalRead(LM2)));
  //   // comm.transmit("STOP2");
  // }

  // if (digitalRead(LM3) == HIGH)
  // {
  //   Serial.println("stopping 3");
  //   // comm.transmit("STOP3");
  // }

  // if (digitalRead(LM4) == HIGH)
  // {
  //   Serial.println("stopping 4");
  //   // comm.transmit("STOP4");
  // }
}

void checkButtons()
{
  if (digitalRead(BUT1) == HIGH)
  {
    Serial.println("button 1");
  }

  if (digitalRead(BUT2) == HIGH)
  {
    Serial.println("button 2");
  }

  if (digitalRead(BUT3) == HIGH)
  {
    Serial.println("button 3");
  }
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

// STEPPER UP DOWN - LIMIT 6000
//    CL - UP
//    CC - DOWN
// STEPPER ARM - LIMIT 2500
// STEPPER GRIPPER - LIMIT 600
