#include <Arduino.h>
#include <SIM800.h>
#include <Adafruit_HDC1000.h>

#include "config.h"

#define con Serial

SoftwareSerial gsmSerial(gsmTxPin, gsmRxPin);
CGPRS_SIM800 gsm(gsmSerial, gsmResetPin);

void setup() {
  // Console
  con.begin(9600);
  while (!con);

  // Relay
  pinMode(relayPin, OUTPUT);

  // Start modem
  con.print("Resetting...");
  while (!gsm.init());
  con.println("OK");
}

void loop() {

}

void powerOff() {
  digitalWrite(relayPin, HIGH);
}

void powerOn() {
  digitalWrite(relayPin, LOW);
}
