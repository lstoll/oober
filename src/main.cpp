#include <Arduino.h>
#include <Adafruit_HDC1000.h>
#include <Adafruit_FONA.h>
#include <Wire.h>
#include <Adafruit_SleepyDog.h>

#include "config.h"

SoftwareSerial gsmSerial(gsmTxPin, gsmRxPin);
Adafruit_FONA fona(gsmResetPin);
Adafruit_HDC1000 hdc;

// this is a large buffer for replies
char replybuffer[160];
char inbuffer[255];

bool tempAlerted = false;

void powerOff();
void powerOn();

void setup() {
  // // should be 8000ms
  Watchdog.enable();

  // On board light
  pinMode(13, OUTPUT);

  for (int i = 0; i<5; i++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }

  // Console
  Serial.begin(115200);

  // Relay
  pinMode(relayPin, OUTPUT);

  // hum interface
  Wire.begin();

  // Start modem serial
  gsmSerial.begin(9600);

  Watchdog.reset();

  // start modem
  Serial.print(F("Starting modem..."));
  if (! fona.begin(gsmSerial)) {
    Serial.println(F("Couldn't find Modem"));
  } else {
    Serial.println(F("OK"));
  }
  Watchdog.reset();

  // Start humid sensor
  Serial.print(F("Starting humidity sensor..."));
  if (!hdc.begin()) {
    Serial.println(F("Couldn't find sensor!"));
  } else {
    Serial.println(F("OK"));
  }

  // Flag that we're ready
  digitalWrite(13, HIGH);
  Watchdog.reset();
}

void loop() {
  Watchdog.reset();

  digitalWrite(13, LOW);
  delay(50);
  digitalWrite(13, HIGH);

  float humid = hdc.readHumidity();
  float temp = hdc.readTemperature();

  Serial.print(F("humid: "));
  Serial.println(humid);
  Serial.print(F("temp: "));
  Serial.println(temp);

  Watchdog.reset();

  uint8_t networkStatus = fona.getNetworkStatus();
  if (networkStatus != 1 && networkStatus != 5) {
    Serial.print(F("Waiting for cell connection, status: "));
    Serial.println(networkStatus);
    Watchdog.reset();
    delay(2000);
    return;
  } else {
    Serial.print(F("Signal strength: "));
    Serial.println(fona.getRSSI());

    if (tempAlerted && temp > (alertAtTemp + 2.5)) {
      // reset the alert, we're above the temp by far enough that things are improved
      Serial.println(F("Temp raised above threshold, resetting alert"));
      tempAlerted = false;
    }  else if (!tempAlerted && temp < alertAtTemp) {
      // ruh roh
      Serial.println(F("Temp dropped below threshold, alerting"));
      tempAlerted = true;

      char temp_temp[6];
      dtostrf(temp, 4, 2, temp_temp);
      sprintf(replybuffer, "Temperature is below threshold. temp: %s", temp_temp);
      fona.sendSMS(alertNumber, replybuffer);
    }

    Watchdog.reset();

    int8_t smsnum = fona.getNumSMS();
    if (smsnum < 0) {
      Serial.println(F("Could not read # SMS"));
      Watchdog.reset();
      delay(2000);
      return;
    } else {
      Serial.print(smsnum);
      Serial.println(F(" SMS's on SIM card!"));
    }

    if (smsnum == 0) {
      Watchdog.reset();
      delay(5000);
      return;
    }

     // there's an SMS!
    uint8_t n = 1;
    while (true) {
      Watchdog.reset();

      uint16_t smslen;
      char sender[25];

      Serial.print(F("\n\rReading SMS #")); Serial.println(n);
      uint8_t len = fona.readSMS(n, inbuffer, 250, &smslen); // pass in buffer and max len!
      // if the length is zero, its a special case where the index number is higher
      // so increase the max we'll look at!
      if (len == 0) {
        Serial.println(F("[empty slot]"));
        n++;
        continue;
      }
      if (! fona.getSMSSender(n, sender, sizeof(sender))) {
	// failed to get the sender?
	sender[0] = 0;
      }

      Watchdog.reset();

      Serial.print(F("***** SMS #")); Serial.print(n);
      Serial.print(" ("); Serial.print(len); Serial.println(F(") bytes *****"));
      Serial.println(inbuffer);
      Serial.print(F("From: ")); Serial.println(sender);
      Serial.println(F("*****"));

      if (strcasecmp(inbuffer, "temp") == 0) {
	// what's it like inside.
	Serial.println(F("Sending environment info"));
	char temp_temp[6];
	char humid_temp[6];
	dtostrf(hdc.readTemperature(), 4, 2, temp_temp);
	dtostrf(hdc.readHumidity(), 4, 2, humid_temp);
	sprintf(replybuffer, "temp: %s humid: %s", temp_temp, humid_temp);
	fona.sendSMS(sender, replybuffer);
      }
      if (strcasecmp(inbuffer, "power off") == 0) {
	Serial.println(F("Powering off"));
	powerOff();
	fona.sendSMS(sender, "Powered off");
      }
      if (strcasecmp(inbuffer, "power on") == 0) {
	Serial.println(F("Powering on"));
	powerOn();
	fona.sendSMS(sender, "Powered on");
      }
      if (strcasecmp(inbuffer, "power cycle") == 0) {
	Serial.println(F("Power cycling"));
	powerOff();
	Watchdog.disable();
	delay(10000);
	Watchdog.enable();
	powerOn();
	fona.sendSMS(sender, "Power cycled");
      }

      Watchdog.reset();

      delay(3000);
      break;
    }
    fona.deleteSMS(n);

    Watchdog.reset();

    delay(2000);
    return;
  }
  delay(2000);
}

void powerOff() {
  digitalWrite(relayPin, HIGH);
}

void powerOn() {
  digitalWrite(relayPin, LOW);
}
