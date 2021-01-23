#include <GEM_u8g2.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Indio.h>
#include <ModbusMaster.h>

#include "src/splash.h"
#include "src/modbus.h"

extern void setupModbus();
extern void leesAlarmen();
extern ModbusMaster toestelOud;
extern ModbusMaster toestelNieuw;

int period = 1000;
unsigned long time_now = 0;

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
    Serial.println("Hello");
    uint8_t result;
    result = toestelOud.readInputRegisters(6, 1);
    SerialUSB.print(result);
    SerialUSB.print(" + ");
    SerialUSB.println(toestelOud.ku8MBSuccess);
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
    }
    leesAlarmen();
  }
  if (menu.readyForKey()) {
    // ...detect key press using U8g2 library
    // and pass pressed button to menu
    menu.registerKeyPress(u8g2.getMenuEvent());
  }
}
