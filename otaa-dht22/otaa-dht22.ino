/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300 LoRa module.
  Example testing sketch with DHT22 sensors on D0 pin

  REQUIRES the following Arduino libraries:
  - MKRWAN https://github.com/arduino-libraries/MKRWAN
  - CayenneLPP https://github.com/sabas1080/CayenneLPP
  - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library 
  - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
*/

#include <MKRWAN.h>
LoRaModem modem;
#include "arduino_secrets.h" 
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

#include <CayenneLPP.h>
CayenneLPP lpp(51);

#include "DHT.h"
#define DHTPIN 0  // D0
#define DHTTYPE DHT22   // DHT22
DHT dht(DHTPIN, DHTTYPE);
float h, t, f = 0; // sensors global variables

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println(F("DHTxx test!"));
  dht.begin();
  
  // Change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };

  modem.minPollInterval(60);
  modem.setADR(true);
  
  // set datarate not working ?
  //modem.dataRate(3);
  
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

}

void loop() {
  // Wait a few seconds between measurements.
  Serial.println("-- LOOP");
  read_temperature();
  send_message();
  delay(120 * 1000);
}

void read_temperature() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
}

void send_message() {
  lpp.reset();
  lpp.addTemperature(1, t);
  lpp.addRelativeHumidity(2, h);
  
  int err;
  modem.beginPacket();
  modem.write(lpp.getBuffer(), lpp.getSize());
  err = modem.endPacket(false);
  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
    Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
    Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
  }
  delay(1000);
  if (!modem.available()) {
    Serial.println("No downlink message received at this time.");
    return;
  }
  String rcv;
  rcv.reserve(64);
  while (modem.available()) {
    rcv += (char)modem.read();
  }
  Serial.print("Received: " + rcv + " - ");
  for (unsigned int i = 0; i < rcv.length(); i++) {
    Serial.print(rcv[i] >> 4, HEX);
    Serial.print(rcv[i] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();
}
