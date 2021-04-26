/*
 * Malcolm Stanley
 * Pool Heater Temperature Sensor
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi101.h>

#include "arduino_secrets.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Labeled Temperature Sensors are as follows
char intake[]   = "283AD1ED0C0000C0";
char midpoint[] = "2811B0EC0C0000DD";
char outlet[]   = "28315DEC0C00005D";

DeviceAddress intakeAddress = { 0x28, 0x3A, 0xD1, 0xED, 0x0C, 0x00, 0x00, 0xC0 };
DeviceAddress midpointAddress = { 0x28, 0x11, 0xB0, 0xEC, 0x0C, 0x00, 0x00, 0xDD };
DeviceAddress outletAddress = { 0x28, 0x31, 0x5D, 0xEC, 0x0C, 0x00, 0x00, 0x5D };

// these next items are in a password file I have gitignored. 
// referenced here so you can see you need to declare them
// tutorial for this is at https://www.andreagrandi.it/2020/12/16/how-to-safely-store-arduino-secrets/
// note the include syntax above is pretty specific

String apiKey ="SECRET_API_KEY_FROM_THINGSPEAK"; // api from ThingSpeak
char ssid[] = "SECRET_SSID_FOR WIFI_AP"; //  your network SSID (name)
char pass[] = "SECRET_WIFI_PASSWORD";    //your network password


// next #define WEBSITE "api.thingspeak.com"
int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "api.thingspeak.com";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) 
  {
    Serial.println("Attempting to connect to SSID: ");
    Serial.println(ssid);
    //Connect to WPA/WPA2 network.Change this line if using open/WEP network
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  
  Serial.println("Connected to wifi");
  printWifiStatus();

  // start serial port
  Serial.begin(9600);
  
  // Start up the library
  sensors.begin();
}

void loop(void) { 
  
  // Print the data
  float tempIntake = sensors.getTempC(intakeAddress);
    Serial.println("Intake Sensor ");
    Serial.print(intake);
    Serial.print(": Temp C: ");
    Serial.print(tempIntake);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempIntake)); // Converts tempC to Fahrenheit
  
  float tempMidpoint = sensors.getTempC(midpointAddress);
    Serial.println("Midpoint Sensor ");
    Serial.print(midpoint);
    Serial.print(": Temp C: ");
    Serial.print(tempMidpoint);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempMidpoint)); // Converts tempC to Fahrenheit
  
  float tempOutlet = sensors.getTempC(outletAddress);
    Serial.println("Outlet Sensor ");
    Serial.print(outlet);
    Serial.print(": Temp C: ");
    Serial.print(tempOutlet);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempOutlet)); // Converts tempC to Fahrenheit

// create data string to send to ThingSpeak
  String data = String("field1=" + String(DallasTemperature::toFahrenheit(tempIntake), DEC) + "&field2=" + String(DallasTemperature::toFahrenheit(tempMidpoint), DEC) + "&field3=" + String(DallasTemperature::toFahrenheit(tempOutlet), DEC)); 
  
  // close any connection before sending a new request
  client.stop();

  // POST data to ThingSpeak
  if (client.connect(server, 80)) {
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("X-THINGSPEAKAPIKEY: "+apiKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.print(data.length());
    client.print("\n\n");
    client.print(data);
    Serial.println("Data Sent to Thingspeak:");
    Serial.print(data);
    } else {
      Serial.println("*** Connection to Thingspeak Not Established");
    }
  
  delay(60000); 
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  
  }   
  
