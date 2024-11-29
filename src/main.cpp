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

// ================= CONSTANTS =================
bool displayLimits = false;
bool enabledJoystick = false;

int upperMinimum = 4095 - 100;
int lowerMinimum = 100;

int rotaryCounter = 0;
int aState;
int aLastState;

int minmax_1[2] = {100, 1000};

WireMaster comm;
void checkLimits();
void checkButtons();
void checkRotary();
int mapValue(int value, int min, int max, int rangeStart, int rangeEnd);
int mapAnalog(int analogValue, int range[2]);

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

    int joystick1_rangeX[2] = {1000, 60};
    int joystick1_dirX = mapAnalog(joystick1_X, joystick1_rangeX);

    int joystick1_rangeY[2] = {400, 40};
    int joystick1_dirY = mapAnalog(joystick1_Y, joystick1_rangeY);

    int joystick2_rangeX[2] = {1000, 70};
    int joystick2_dirX = mapAnalog(joystick2_X, joystick2_rangeX);

    int joystick2_rangeY[2] = {1000, 150};
    int joystick2_dirY = mapAnalog(joystick2_Y, joystick2_rangeY);

    comm.transmit("STED1" + String(joystick1_dirX < 0 ? "CC" : "CL") + String(abs(joystick1_dirX)));
    comm.transmit("STED2" + String(joystick1_dirY < 0 ? "CC" : "CL") + String(abs(joystick1_dirY)));
    comm.transmit("STED3" + String(joystick2_dirX < 0 ? "CC" : "CL") + String(abs(joystick2_dirX)));
    comm.transmit("STED4" + String(joystick2_dirY < 0 ? "CC" : "CL") + String(abs(joystick2_dirY)));
    // if (joystick1_dirX == -1)
    //   comm.transmit("STEP1CC10");
    // if (joystick1_dirX == 1)
    //   comm.transmit("STEP1CL10");
    // if (joystick1_dirY == -1)
    //   comm.transmit("STEP2CC10");
    // if (joystick1_dirY == 1)
    //   comm.transmit("STEP2CL10");

    // if (joystick2_dirX == -1)
    //   comm.transmit("STEP3CC10"); // ARM
    // if (joystick2_dirX == 1)
    //   comm.transmit("STEP3CL10");
    // if (joystick2_dirY == -1)
    //   comm.transmit("STEP4CC10");
    // if (joystick2_dirY == 1)
    //   comm.transmit("STEP4CL10");

    // Serial.println("J1 X: " + String(joystick1_dirX));
    // Serial.println("J1X: " + String(joystick1_X) + " | J1Y: " + String(joystick1_Y) + " | J2X: " + String(joystick2_X) + " | J2Y: " + String(joystick2_Y));
  }

  // checkLimits();
  // checkButtons();
  checkRotary();
}

int mapAnalog(int value, int range[2])
{
  int normalized = map(value, 0, 4095, -128, 128);
  if (normalized < 20 && normalized > -20)
    return 0;

  int abs_value = abs(normalized);
  int multipler = normalized / abs_value;

  int converted_delay = map(abs_value, 20, 128, range[0], range[1]);
  return converted_delay * multipler;
}

int mapValue(int value, int min, int max, int rangeStart, int rangeEnd)
{
  if (value == 0)
    return 0;
  int multiplier = value > 0 ? 1 : -1;
  int newvalue = abs(value);

  // Linear interpolation formula with integer output
  return map(newvalue, min, max, rangeStart, rangeEnd) * multiplier;
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
