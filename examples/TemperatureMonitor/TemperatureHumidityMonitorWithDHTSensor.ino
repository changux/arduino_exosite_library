/*
 Exosite Arduino Basic Temp Monitor 2 (updated to use Exosite library)
  
 This sketch shows an example of sending data from a connected
 sensor to Exosite. (http://exosite.com) Code was used from various
 public examples including the Arduino Ethernet examples and the DHT
 Library examples found on the Arduino playground. 
  
 This code keeps track of how many milliseconds have passed
 and after the user defined REPORT_TIMEOUT (default 60 seconds)
 reports the temperature from a DHT22 (and 11) temp and humidity sensor.
 The code sets up the Ethernet client connection and connects / disconnects 
 to the Exosite server when sending data.
  
 Assumptions:
 - Tested with Arduino 1.6.6
 - Arduino included Ethernet Library
 - Arduino included SPI Library
 - Using Exosite library https://github.com/exosite-garage/arduino_exosite_library
 - Using Adafruit's DHT Library https://github.com/adafruit/DHT-sensor-library 
 - User has an Exosite account and created a device (CIK needed / https://portals.exosite.com -> Add Device)
 - User has added a device to Exosite account and added a data source with alias 'temp', type 'float' and 'humidity', type 'float'.

 
  
 Hardware:
 - Arduino Uno, Mega, Duemilanove or similiar
 - Arduino Ethernet Shield
 - DHT 22 (11 or 21 also works) temp sensor (on data pin 2, with 4.7k pull-up)
 
Version History:
- 1.0 - Dec 15 2015. First version based on the TemperatureMonitor sketch of Exosite library.

*/


#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"
#include <Exosite.h>

#define DHTPIN 2 // PIN connected

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

//

String cikData = "";  // <-- FILL IN YOUR CIK HERE! (https://portals.exosite.com -> Add Device)
byte macData[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };         // <-- FILL IN YOUR Ethernet shield's MAC address here.

// User defined variables for Exosite reporting period and averaging samples
#define REPORT_TIMEOUT 30000 //milliseconds period for reporting to Exosite.com
#define SENSOR_READ_TIMEOUT 5000 //milliseconds period for reading sensors in loop

class EthernetClient client;
Exosite exosite(cikData, &client);


void setup() {
  Serial.begin(9600);
  Serial.println("Boot");
  dht.begin();
  delay(1000);
  Serial.println("Starting Exosite Temp Monitor");
  Serial.print("DHT Digital Pin Specified: ");
  Serial.println(DHTPIN);
  Ethernet.begin(macData);
  // wait 3 seconds for connection
  delay(3000);

}

void loop() {
  static unsigned long sendPrevTime = 0;
  static unsigned long sensorPrevTime = 0;
  static float tempC;
  static float h;
  char buffer[7];
  String readParam = "";
  String writeParam = "";
  String returnString = "";
  // Wait a few seconds between measurements.

  Serial.print("."); // print to show running

  // Read sensor every defined timeout period
  if (millis() - sensorPrevTime > SENSOR_READ_TIMEOUT) {
    Serial.println();
    Serial.println("Requesting temperature...");
    dht.readTemperature(); // Send the command to get temperatures
    float tempC = dht.readTemperature();
    Serial.print("Celsius:    ");
    Serial.print(tempC);
    Serial.println(" C ..........DONE");
    tempC = dht.readTemperature();
    //
    Serial.println();
    Serial.println("Requesting humidity...");
    dht.readHumidity(); // Send the command to get humidity
    float h = dht.readHumidity();
    Serial.print("Percent:    ");
    Serial.print(h);
    Serial.println(" % ..........DONE");
    h = dht.readHumidity();


    sensorPrevTime = millis();
  }

  // Send to Exosite every defined timeout period
  if (millis() - sendPrevTime > REPORT_TIMEOUT) {
    Serial.println(); //start fresh debug line
    Serial.println("Sending data to Exosite...");
    tempC = dht.readTemperature();
    h = dht.readHumidity();

    readParam = "";        //nothing to read back at this time e.g. 'control&status' if you wanted to read those data sources
    writeParam = "temp="; //parameters to write e.g. 'temp=65.54' or 'temp=65.54&status=on'

    String tempValue = dtostrf(tempC, 1, 2, buffer); // convert float to String, minimum size = 1, decimal places = 2

    writeParam += tempValue;    //add converted temperature String value

    writeParam += "&humidity="; //add humidity

    writeParam += h;

    //Debug
    //Serial.println(writeParam);

    if ( exosite.writeRead(writeParam, readParam, returnString)) {
      Serial.println("Exosite OK");
      if (returnString != "") {
        Serial.println("Response:");
        Serial.println(returnString);
      }
    }
    else {
      Serial.println("Exosite Error");
    }

    sendPrevTime = millis(); //reset report period timer
    Serial.println("done sending.");
  }
  delay(1000); //slow down loop
}
