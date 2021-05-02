/*
 * Malcolm Stanley
 * Pool Heater Temperature Sensor
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiNINA.h>
#include <ThingSpeak.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
// #include <DHT_U.h>
#include "arduino_secrets.h"

// Data wire is plugged into port 4 on the Arduino
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

//DHT 
#define DHTPIN 6     // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT21 // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

// these next items are in a password file I have gitignored. 
// referenced here so you can see you need to declare them
// tutorial for this is at https://www.andreagrandi.it/2020/12/16/how-to-safely-store-arduino-secrets/
// note the include syntax above is pretty specific

// Secrets
unsigned long myChannelNumber = SECRET_CH_ID;
const char * apiKey = SECRET_API_KEY_FROM_THINGSPEAK; // api from ThingSpeak

char ssid[] = SECRET_SSID_FOR_WIFI_AP; //  your network SSID (name)
char pass[] = SECRET_WIFI_PASSWORD;    //your network password


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
  
  // Start up all the things
  Serial.begin(9600);
  sensors.begin();
  sensors.requestTemperatures();
  ThingSpeak.begin(client);  //Initialize ThingSpeak
  dht.begin();
  
}

void loop(void) { 
  
  // Print the data from the DS18B20 Sensors
  float tempIntake = sensors.getTempC(intakeAddress);
    Serial.println("Intake Sensor ");
    Serial.print(": Temp C: ");
    Serial.print(tempIntake);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempIntake)); // Converts tempC to Fahrenheit
  
  float tempMidpoint = sensors.getTempC(midpointAddress);
    Serial.println("Midpoint Sensor ");
    Serial.print(": Temp C: ");
    Serial.print(tempMidpoint);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempMidpoint)); // Converts tempC to Fahrenheit
  
  float tempOutlet = sensors.getTempC(outletAddress);
    Serial.println("Outlet Sensor ");
    Serial.print(": Temp C: ");
    Serial.print(tempOutlet);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempOutlet)); // Converts tempC to Fahrenheit

  // Print the data from the DHT 21 Sensor
  // Get temperature event and print its value.
   // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

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
   

// init data string to send to ThingSpeak
// not this way --  String data = String("field1=" + String(DallasTemperature::toFahrenheit(tempIntake), DEC) + "&field2=" + String(DallasTemperature::toFahrenheit(tempMidpoint), DEC) + "&field3=" + String(DallasTemperature::toFahrenheit(tempOutlet), DEC)); 

ThingSpeak.setField(1, DallasTemperature::toFahrenheit(tempIntake));
ThingSpeak.setField(2, DallasTemperature::toFahrenheit(tempMidpoint));
ThingSpeak.setField(3, DallasTemperature::toFahrenheit(tempOutlet));
ThingSpeak.setField(4, f);
ThingSpeak.setField(5, h);

// write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, apiKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
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
  
