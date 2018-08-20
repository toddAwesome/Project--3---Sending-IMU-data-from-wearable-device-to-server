#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // required!
#include <HTTPClient.h>

#define LSM9DS1_SCK 18
#define LSM9DS1_MISO 19
#define LSM9DS1_MOSI 23
#define LSM9DS1_XGCS 5
#define LSM9DS1_MCS 15

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_SCK, LSM9DS1_MISO, LSM9DS1_MOSI, LSM9DS1_XGCS, LSM9DS1_MCS); // software SPI
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_XGCS, LSM9DS1_MCS); // hardware SPI

char ssid[] = "CenturyLink4868"; //  home network SSID (name)
char pass[] = "e33b8733c47cbb";    // home network password (WPA Network)
char univssid[] = "University of Washington"; //  university network SSID (name)
int keyIndex = 0; //incase (WEP Network)


int status = WL_IDLE_STATUS;

void setupSensor() {
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);

  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
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

void setup() {
  Serial.begin(115200);
  Serial.println();

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens.
  }
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin()) {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  //program starts
  Serial.println("LSM9DS1 IMU + ESP32 data writing to server....");
  Serial.println("Found LSM9DS1 9DOF");
  setupSensor();
  status = WiFi.disconnect();
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
    //    Serial.println(univssid);
    //    status = WiFi.begin(univssid); //Open Network
    delay(10000); // wait 10 seconds for connection
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { //check wifi
    lsm.read();  /* ask it to read in the data */
    /* Get a new sensor event */
    sensors_event_t a, m, g, temp;
    lsm.getEvent(&a, &m, &g, &temp);
    StaticJsonBuffer<400> JSONbuffer; //Declaring static JSON buffer
    JsonObject& JSONencoder = JSONbuffer.createObject();

    /* accelerometer */
    JsonArray& accelerometer = JSONencoder.createNestedArray("accelerometer");
    accelerometer.add(a.acceleration.x);
    accelerometer.add(a.acceleration.y);
    accelerometer.add(a.acceleration.z);

    /* magnotometer */
    JsonArray& magnetometer = JSONencoder.createNestedArray("magnetometer");
    magnetometer.add(m.magnetic.x);
    magnetometer.add(m.magnetic.y);
    magnetometer.add(m.magnetic.z);

    /* gyroscope */
    JsonArray& gyroscope = JSONencoder.createNestedArray("gyroscope");
    gyroscope.add(g.gyro.x);
    gyroscope.add(g.gyro.y);
    gyroscope.add(g.gyro.z);

    char JSONmessageBuffer[400];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);

    HTTPClient http;    //Declare object of class HTTPClient
    http.begin();
    IPAddress ipAddress = WiFi.localIP();
    http.connect(); 
    http.addHeader("Content-Type", "application/json");  //Specify
    int httpCode = http.POST(JSONmessageBuffer); //Send the request

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(httpCode);   //Print HTTP return code
      Serial.println(payload);    //Print request response payload
    } else {
      Serial.printf("POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();  //Close connection
  } else {
    Serial.println("Error in WiFi connection");
  }
  delay(10000);
}


