#ifndef WIRE_MASTER_H
#define WIRE_MASTER_H
#include <Wire.h>

#define SLAVE_ADDR 69

class WireMaster
{
public:
    void begin();
    void transmit(char *payload);
    void transmit(String payload);
    String receive();
};
#endif