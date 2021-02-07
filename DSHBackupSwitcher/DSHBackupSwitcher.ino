#include <GEM_u8g2.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Indio.h>
#include <ModbusMaster.h>

#include "src/splash.h"
#include "src/modbus.h"

extern void setupModbus();
extern ModbusMaster toestelOud;
extern ModbusMaster toestelNieuw;

int period = 1000;
unsigned long time_now = 0;
bool alarmOudInAlarm, alarmNieuwInAlarm;
int demandpct;
int counter = 0;


#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_UC1701_MINI12864_F_2ND_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/ 19, /* dc=*/ 22);
int number = -512;
boolean enablePrint = false;
byte byteNumber = 1;
GEMPage menuPageMain("Menu");
SelectOptionByte optionsArray[] = {{"Oud", 1}, {"Nieuw", 2}};
GEMSelect myByteSelect(2, optionsArray);
GEMItem menuItemByteNumber("Forceer:", byteNumber, myByteSelect);

GEM_u8g2 menu(u8g2);


void setup() {
  SerialUSB.begin(9600);
  analogWrite(26, 255);
  u8g2.begin(/*Select*/ 24, U8X8_PIN_NONE, U8X8_PIN_NONE, /*Up*/ 25,/* Down */ 23, U8X8_PIN_NONE);

  menu.setSplash(splash_width, splash_height, splash_bits);
  menu.hideVersion();

  menu.init();
  setupMenu();
  menu.drawMenu();

  setupModbus();

  // digital outs zetten
  Indio.digitalMode(1, OUTPUT);
  Indio.digitalMode(2, INPUT);
  Indio.digitalMode(3, OUTPUT);
  Indio.digitalMode(4, INPUT);
  Indio.digitalMode(7, OUTPUT);
  Indio.digitalMode(8, OUTPUT);

}
void setupMenu() {
  // Add menu items to menu page
  menuPageMain.addMenuItem(menuItemByteNumber);
  // Add menu page to menu and set it as current
  menu.setMenuPageCurrent(menuPageMain);
}

void loop() {
  // If menu is ready to accept button press...
  if ((unsigned long)(millis() - time_now) > period) {
    // elke seconde pullen, niet delay gebruiken, dat verneukt modbus (na 2 dagen ~5m vertraging, eindelijk weten we waarom)
    time_now = millis();
    /*uint8_t result;
      result = toestelOud.readInputRegisters(6, 1);
      if (result == toestelOud.ku8MBSuccess)
      {
      SerialUSB.print("Temp: ");
      SerialUSB.println(toestelOud.getResponseBuffer(0));
      }
      result = toestelOud.writeSingleRegister(0, 1);
      if (result == toestelOud.ku8MBSuccess)
      {
      SerialUSB.println("Auto gezet ");
      }
      result = toestelOud.writeSingleRegister(5, 50);
      if (result == toestelOud.ku8MBSuccess)
      {
      SerialUSB.println("50% gezet ");
      }*/
    leesAlarmen();
    leesInputs();
    zetToestel();
  }
  if (menu.readyForKey()) {
    // ...detect key press using U8g2 library
    // and pass pressed button to menu
    menu.registerKeyPress(u8g2.getMenuEvent());
  }
}
void schrijfDemand(int toestel)
{
  uint8_t result;
  //1 = oud 2 = nieuw
  switch (toestel) {
    case 1:
      result = toestelOud.writeSingleRegister(5, demandpct);
      if (result == toestelOud.ku8MBSuccess)
      {
        SerialUSB.println("Demand bij oud geschreven");
      } else {
        SerialUSB.print("Alarm demand bij oud: ");
        SerialUSB.println(result);
      }
      result = toestelNieuw.writeSingleRegister(5, 0);
      if (result == toestelNieuw.ku8MBSuccess)
      {
        SerialUSB.println("Demand bij nieuw geschreven (0)");
      } else {
        SerialUSB.print("Alarm demand bij nieuw: ");
        SerialUSB.println(result);
      }
      toestelOud.clearResponseBuffer();
      toestelNieuw.clearResponseBuffer();
      break;
    case 2:
      result = toestelOud.writeSingleRegister(5, 0);
      if (result == toestelOud.ku8MBSuccess)
      {
        SerialUSB.println("Demand bij oud geschreven (0)");
      } else {
        SerialUSB.print("Alarm demand bij oud: ");
        SerialUSB.println(result);
      }
      result = toestelNieuw.writeSingleRegister(5, demandpct);
      if (result == toestelNieuw.ku8MBSuccess)
      {
        SerialUSB.println("Nieuw demand geschreven");
      } else {
        SerialUSB.print("Alarm bij nieuw demand: ");
        SerialUSB.println(result);
      }
      toestelOud.clearResponseBuffer();
      toestelNieuw.clearResponseBuffer();
      break;
  }
}
void zetToestel()
{
  if ((alarmOudInAlarm) && (!alarmNieuwInAlarm))
  {
    // nieuw niet in alarm oud wel
    klepEnActiveer(2);
    //activeerToestel(2);
  } else if ((!alarmOudInAlarm) && (alarmNieuwInAlarm)) {
    // oud niet in alarm nieuw wel
    klepEnActiveer(1);
    //activeerToestel(1);
  } else if ((!alarmOudInAlarm) && (!alarmNieuwInAlarm)) {
    // beiden niet in alarm
    klepEnActiveer(1);
    //activeerToestel(1);
  } else {
    SerialUSB.println("alarm, beide toestellen in alarm");
  }
}
void klepEnActiveer(int toestel)
{
  switch (toestel) {
    //1 = oud, 2 = nieuw
    case 1:
      Indio.digitalWrite(1, HIGH);
      Indio.digitalWrite(3, LOW);
      if (Indio.digitalRead(2) == HIGH)
      {
        activeerToestel(toestel);
      } else {
        deactiveerBeide();
      }
      break;
    case 2:
      Indio.digitalWrite(3, HIGH);
      Indio.digitalWrite(1, LOW);
      if (Indio.digitalRead(4) == HIGH)
      {
        activeerToestel(toestel);
      } else {
        deactiveerBeide();
      }
      break;
  }
}
void deactiveerBeide()
{
  uint8_t result;
  result = toestelNieuw.writeSingleRegister(0, 2);
  if (result == toestelNieuw.ku8MBSuccess)
  {
    SerialUSB.println("Nieuw in standby (deactiveer beide)");
  } else {
    SerialUSB.print("Alarm bij standby nieuw (deactiveer beide): ");
    SerialUSB.println(result);
  }
  result = toestelOud.writeSingleRegister(0, 2);
  if (result == toestelOud.ku8MBSuccess)
  {
    SerialUSB.println("Oud in standby (deactiveer beide)");
  } else {
    SerialUSB.print("Alarm bij standby Oud (deactiveer beide): ");
    SerialUSB.println(result);
  }
  toestelOud.clearResponseBuffer();
  toestelNieuw.clearResponseBuffer();
}
void activeerToestel(int toestel)
{
  uint8_t result;
  //1 = oud 2 = nieuw
  switch (toestel) {
    case 1:
      result = toestelOud.writeSingleRegister(0, 1);
      if (result == toestelOud.ku8MBSuccess)
      {
        SerialUSB.println("Oud op auto gezet");
      } else {
        SerialUSB.print("Alarm bij oud op auto zetten: ");
        SerialUSB.println(result);
      }
      result = toestelNieuw.writeSingleRegister(0, 2);
      if (result == toestelNieuw.ku8MBSuccess)
      {
        SerialUSB.println("Nieuw op standby gezet");
      } else {
        SerialUSB.print("Alarm bij nieuw op standby zetten: ");
        SerialUSB.println(result);
      }
      toestelOud.clearResponseBuffer();
      toestelNieuw.clearResponseBuffer();
      schrijfDemand(1);
      break;
    case 2:
      result = toestelOud.writeSingleRegister(0, 2);
      if (result == toestelOud.ku8MBSuccess)
      {
        SerialUSB.println("Oud op standby gezet");
      } else {
        SerialUSB.print("Alarm bij oud op standby zetten: ");
        SerialUSB.println(result);
      }
      result = toestelNieuw.writeSingleRegister(0, 1);
      if (result == toestelNieuw.ku8MBSuccess)
      {
        SerialUSB.println("Nieuw op auto gezet");
      } else {
        SerialUSB.print("Alarm bij nieuw op auto zetten: ");
        SerialUSB.println(result);
      }
      toestelOud.clearResponseBuffer();
      toestelNieuw.clearResponseBuffer();
      schrijfDemand(2);
      break;
  }
}
void leesInputs()
{
  Indio.setADCResolution(16);
  Indio.analogReadMode(1, V10_p);
  demandpct = Indio.analogRead(1);
}
void leesAlarmen()
{
  uint8_t oud, nieuw;
  oud = toestelOud.readCoils(0, 1);
  nieuw = toestelNieuw.readCoils(0, 1);
  if ((oud == toestelOud.ku8MBSuccess) && (nieuw == toestelNieuw.ku8MBSuccess))
  {
    alarmOudInAlarm = toestelOud.getResponseBuffer(0);
    alarmNieuwInAlarm = toestelNieuw.getResponseBuffer(0);
    if (alarmOudInAlarm)
    {
      Indio.digitalWrite(7, HIGH);
    } else {
      Indio.digitalWrite(7, LOW);
    }
    if (alarmNieuwInAlarm)
    {
      Indio.digitalWrite(8, HIGH);
    } else {
      Indio.digitalWrite(8, LOW);
    }
    SerialUSB.print("alarm oud ");
    SerialUSB.println(alarmOudInAlarm);
    SerialUSB.print("alarm nieuw ");
    SerialUSB.println(alarmNieuwInAlarm);
  } else {
    SerialUSB.println("Alarm, kan ergens geen waardes opvragen van toestel");
    SerialUSB.print("Oud ");
    SerialUSB.print(oud);
    SerialUSB.print(" nieuw ");
    SerialUSB.println(nieuw);
  }

  toestelOud.clearResponseBuffer();
  toestelNieuw.clearResponseBuffer();
}
