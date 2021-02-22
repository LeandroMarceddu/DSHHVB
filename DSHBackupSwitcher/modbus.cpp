#include "src/modbus.h"
#include <ModbusMaster.h>

#define MAX485_RE_NEG      9


ModbusMaster toestelOud;
ModbusMaster toestelNieuw;
uint32_t t3_5 = (1000000 * 39) / 9600 + 500;
void preTransmission()
{
  delayMicroseconds(t3_5);
  digitalWrite(MAX485_RE_NEG, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  delayMicroseconds(t3_5);
}
void setupModbus()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  Serial.begin(9600);
  toestelOud.begin(1, Serial);
  toestelOud.preTransmission(preTransmission);
  toestelOud.postTransmission(postTransmission);
  toestelNieuw.begin(2, Serial);
  toestelNieuw.preTransmission(preTransmission);
  toestelNieuw.postTransmission(postTransmission); 
}
