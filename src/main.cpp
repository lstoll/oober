#include <Arduino.h>
#include <SIM800.h>
#include <Adafruit_HDC1000.h>
#include <Wire.h>

#include "config.h"

#define con Serial

SoftwareSerial gsmSerial(gsmTxPin, gsmRxPin);
CGPRS_SIM800 gsm(gsmSerial, gsmResetPin);
Adafruit_HDC1000 hdc;

void setup() {
  // Console
  con.begin(9600);
  while (!con);

  // Relay
  pinMode(relayPin, OUTPUT);

  // hum interface
  Wire.begin();

  // Start modem
  con.print("Resetting...");
  while (!gsm.init());
  con.println("OK");

  delay(3000);

  // Start humid sensor
  if (!hdc.begin()) {
    con.println("Couldn't find sensor!");
  }
}

void loop() {
  if (gsm.getOperatorName()) {
    con.print("Operator:");
    con.println(gsm.buffer);
  }
  int ret = gsm.getSignalQuality();
  if (ret) {
    con.print("Signal:");
    con.print(ret);
    con.println("dB");
  }
  con.print("humid: ");
  con.println(hdc.readHumidity());
  con.print("temp: ");
  con.println(hdc.readTemperature());
  delay(3000);
  /*if(started) {
    //Read if there are messages on SIM card and print them.
    if(gsm.readSMS(smsbuffer, 160, n, 20)) {
      Serial.println(n);
      Serial.println(smsbuffer);
    }
    delay(1000);
    }*/
}

void powerOff() {
  digitalWrite(relayPin, HIGH);
}

void powerOn() {
  digitalWrite(relayPin, LOW);
}
