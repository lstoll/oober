#include <Arduino.h>
#include <Adafruit_HDC1000.h>
#include <Adafruit_FONA.h>
#include <Wire.h>

#include "config.h"

SoftwareSerial gsmSerial(gsmTxPin, gsmRxPin);
Adafruit_FONA fona(gsmResetPin);
Adafruit_HDC1000 hdc;

void setup() {
  // Console
  Serial.begin(115200);

  // Relay
  pinMode(relayPin, OUTPUT);

  // hum interface
  Wire.begin();

  // Start modem serial
  gsmSerial.begin(9600);

  // start modem
  Serial.print("Starting modem...");
  if (! fona.begin(gsmSerial)) {
    Serial.println(F("Couldn't find Modem"));
  } else {
    Serial.println("OK");
  }

  // Start humid sensor
  if (!hdc.begin()) {
    Serial.println("Couldn't find sensor!");
  }
}

void loop() {
  // https://github.com/adafruit/Open-Sesame/blob/master/OpenSesame.ino
  /* if (gsm.getOperatorName()) {
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
  delay(3000);*/
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
