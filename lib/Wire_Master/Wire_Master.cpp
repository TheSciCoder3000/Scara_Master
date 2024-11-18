#include <iterator>
#include "Wire_Master.h"

void WireMaster::begin()
{
    Wire.begin();
}

void WireMaster::transmit(char *payload)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.println(payload);
    Wire.endTransmission();
}
void WireMaster::transmit(String payload)
{
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.println(payload);
    Wire.endTransmission();
}

String WireMaster::receive()
{
    String c;
    Wire.requestFrom(SLAVE_ADDR, 9 + 11); // request 6 bytes from peripheral device #8

    while (Wire.available())
    {                           // peripheral may send less than requested
        c += (char)Wire.read(); // receive a byte as character
    }

    return c;
}