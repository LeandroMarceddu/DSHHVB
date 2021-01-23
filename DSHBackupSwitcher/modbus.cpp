#include "src/modbus.h"
#include <ModbusMaster.h>

#define MAX485_RE_NEG      9

bool alarmOud, alarmNieuw; 
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
void leesAlarmen()
{
  uint8_t oud, nieuw;
  oud = toestelOud.readCoils(0, 1);
  nieuw = toestelNieuw.readCoils(0, 1);
  if ((oud == toestelOud.ku8MBSuccess) && (nieuw == toestelNieuw.ku8MBSuccess)) 
  {
    alarmOud = toestelOud.getResponseBuffer(0);
    alarmNieuw = toestelNieuw.getResponseBuffer(0);
    SerialUSB.print("alarm oud");
    SerialUSB.println(alarmOud);
    SerialUSB.print("alarm nieuw");
    SerialUSB.println(alarmNieuw);
  }
}
