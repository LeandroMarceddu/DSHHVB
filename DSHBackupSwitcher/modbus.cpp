#include "src/modbus.h"
#include <ModbusMaster.h>

#define MAX485_RE_NEG      9


ModbusMaster toestelOud;
ModbusMaster toestelNieuw;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
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
  Serial.begin(9600);
}
