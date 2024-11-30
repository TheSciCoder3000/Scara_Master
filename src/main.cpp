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

bool is_limit_1 = false;
bool is_limit_2 = false;
bool is_limit_3 = false;
bool is_limit_4 = false;

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

    int joystick1_rangeX[2] = {3000, 300};
    int joystick1_dirX = mapAnalog(joystick1_X, joystick1_rangeX);

    int joystick1_rangeY[2] = {400, 60};
    int joystick1_dirY = mapAnalog(joystick1_Y, joystick1_rangeY);

    int joystick2_rangeX[2] = {1500, 150};
    int joystick2_dirX = mapAnalog(joystick2_X, joystick2_rangeX);

    int joystick2_rangeY[2] = {2000, 300};
    int joystick2_dirY = mapAnalog(joystick2_Y, joystick2_rangeY);

    if (joystick1_dirX > 0)
    {
      comm.transmit("STED1CL" + String(is_limit_1 ? 0 : (joystick1_dirX)));
    }
    else
    {
      comm.transmit("STED1CC" + String(abs(joystick1_dirX)));
    }

    if (joystick1_dirY > 0)
    {
      comm.transmit("STED2CL" + String(is_limit_2 ? 0 : (joystick1_dirY)));
    }
    else
    {
      comm.transmit("STED2CC" + String(abs(joystick1_dirY)));
    }

    if (joystick2_dirX > 0)
    {
      comm.transmit("STED3CL" + String(is_limit_3 ? 0 : (joystick2_dirX)));
    }
    else
    {
      comm.transmit("STED3CC" + String(abs(joystick2_dirX)));
    }

    if (joystick2_dirY > 0)
    {
      comm.transmit("STED4CC" + String(is_limit_4 ? 0 : (joystick2_dirY)));
    }
    else
    {
      comm.transmit("STED4CL" + String(abs(joystick2_dirY)));
    }
  }

  checkLimits();
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

void checkRotary()
{
  aState = digitalRead(ROTA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState)
  {
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(ROTB) != aState)
    {
      rotaryCounter += 3;
    }
    else
    {
      rotaryCounter -= 3;
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
  is_limit_1 = digitalRead(LM1) == LOW;
  is_limit_2 = digitalRead(LM2) == LOW;
  is_limit_3 = digitalRead(LM3) == LOW;
  is_limit_4 = digitalRead(LM4) == LOW;
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
