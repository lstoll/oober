#include <Arduino.h>
#include <Adafruit_HDC1000.h>
#include <Adafruit_FONA.h>
#include <Wire.h>

#include "config.h"

SoftwareSerial gsmSerial(gsmTxPin, gsmRxPin);
Adafruit_FONA fona(gsmResetPin);
Adafruit_HDC1000 hdc;

// this is a large buffer for replies
char replybuffer[160];
char inbuffer[255];

void powerOff();
void powerOn();

void setup() {
  // Console
  Serial.begin(115200);

  // Relay
  pinMode(relayPin, OUTPUT);

  // On board light
  pinMode(13, OUTPUT);

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
  Serial.print("Starting humidity sensor...");
  if (!hdc.begin()) {
    Serial.println("Couldn't find sensor!");
  } else {
    Serial.println("OK");
  }

  // Flag that we're ready
  digitalWrite(13, HIGH);
}

void loop() {
  digitalWrite(13, LOW);
  delay(50);
  digitalWrite(13, HIGH);

  Serial.print("humid: ");
  Serial.println(hdc.readHumidity());
  Serial.print("temp: ");
  Serial.println(hdc.readTemperature());

  uint8_t networkStatus = fona.getNetworkStatus();
  if (networkStatus != 1 && networkStatus != 5) {
    Serial.println("Waiting for cell connection");
    delay(2000);
    return;
  } else {
    Serial.print("Signal strength: ");
    Serial.println(fona.getRSSI());
    int8_t smsnum = fona.getNumSMS();
    if (smsnum < 0) {
      Serial.println(F("Could not read # SMS"));
      delay(2000);
      return;
    } else {
      Serial.print(smsnum);
      Serial.println(F(" SMS's on SIM card!"));
    }

    if (smsnum == 0) {
      delay(5000);
      return;
    }

     // there's an SMS!
    uint8_t n = 1;
    while (true) {
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

      Serial.print(F("***** SMS #")); Serial.print(n);
      Serial.print(" ("); Serial.print(len); Serial.println(F(") bytes *****"));
      Serial.println(inbuffer);
      Serial.print(F("From: ")); Serial.println(sender);
      Serial.println(F("*****"));

      if (strcasecmp(inbuffer, "temp") == 0) {
	// what's it like inside.
	Serial.println("Sending environment info");
	char temp_temp[6];
	char humid_temp[6];
	dtostrf(hdc.readTemperature(), 4, 2, temp_temp);
	dtostrf(hdc.readHumidity(), 4, 2, humid_temp);
	sprintf(replybuffer, "temp: %s humid: %s", temp_temp, humid_temp);
	fona.sendSMS(sender, replybuffer);
      }
      if (strcasecmp(inbuffer, "power off") == 0) {
	Serial.println("Powering off");
	powerOff();
	fona.sendSMS(sender, "Powered off");
      }
      if (strcasecmp(inbuffer, "power on") == 0) {
	Serial.println("Powering on");
	powerOn();
	fona.sendSMS(sender, "Powered on");
      }
      if (strcasecmp(inbuffer, "power cycle") == 0) {
	Serial.println("Power cycling");
	powerOff();
	delay(10000);
	powerOn();
	fona.sendSMS(sender, "Power cycled");
      }

      delay(3000);
      break;
    }
    fona.deleteSMS(n);

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
