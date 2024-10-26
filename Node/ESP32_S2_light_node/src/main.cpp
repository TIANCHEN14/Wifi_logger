#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <Adafruit_AS7341.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>

//setup network ssid and password
const char * ssid = "Pi_iot_lan";
const char * pass = "turtleepic";

// setup MQTT broker information
IPAddress       broker(10, 40, 40, 1);
const int       port = 1883;

// setup wifi client and mqtt client
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Delay time 
const long interval = 1000;
unsigned long previousMills = 0;

// setup as7431 sensor
Adafruit_AS7341 as7341;

// function declearing
void mqtt_reconnect();
void wifi_connect();
void publish_spectrum_data();


void setup() {
  // initialized serial and wait for port to open
  Serial.begin(9600);

  // Setting up as7431 as spectrum meter
  while(!as7341.begin()){
    Serial.println("Could not found AS7341, try again");
  }

  // de bugging
  Serial.println("AS7431 begin ");
  Serial.println("Setup Atime, ASTEP, Gain");
  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);
  delay(1000); 

  // connect to wifi
  wifi_connect();

  // connect to mqtt server
  mqtt_reconnect();

}

void loop() {

  // constantly checking making sure we are connected to wifi
  while(WiFi.status() !=  WL_CONNECTED){
    wifi_connect();
  }
  
  // constantly checking making sure we are connected to the mqtt broker
  while(!mqttClient.connected()){
    mqtt_reconnect();
  }
  mqttClient.poll();


 // make sure it is not constantly emiiting
  unsigned long currentMillis = millis();

  //Serial.print("time Interval : ");
  //Serial.println(currentMillis - previousMills);

  if (currentMillis - previousMills >= interval) {
    // save the last time a message was sent
    previousMills = currentMillis;

    // Reading all the light spectrum
    while(!as7341.readAllChannels()){
      Serial.println("Error reading all channels");
    }

    // publish the json object
    publish_spectrum_data();

    }

}


// Mqtt connection function
// This function is used to connect to the mqtt server

void mqtt_reconnect(){
    // Connected to the MQTT server
  Serial.println("Attempt to connect to MQTT server");

  // check to make sure it is connected to the MQTT server
  while (!mqttClient.connect(broker, port)){
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    Serial.println("Try again in 5 second");
    delay(5000);

  }

  // print to show to make sure we connected to the MQTT server successfully
  Serial.println("Successfully connected to MQTT Server");
}


// Wifi Connection function
// This function is used to connect to the wifi
void wifi_connect(){
   // Set rthe hostname before connecting to WIFI
  if (!WiFi.setHostname("esp_light_sensor_1")){
    Serial.println("Failed to set hostname");
  }

  // Start the program
  Serial.print("Trying to connect to SSID :");
  Serial.println(ssid);

  // Connect to WiFi
  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  // print connectivity
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



// publish the spectrum data
// This function is used to publish the spectrum data as an json object
void publish_spectrum_data(){

  // create a json object to store the reading
  JsonDocument spectrum_dict;

  // add reading to the json object
  spectrum_dict["415nm"] = as7341.getChannel(AS7341_CHANNEL_415nm_F1);
  spectrum_dict["445nm"] = as7341.getChannel(AS7341_CHANNEL_445nm_F2);
  spectrum_dict["480nm"] = as7341.getChannel(AS7341_CHANNEL_480nm_F3);
  spectrum_dict["515nm"] = as7341.getChannel(AS7341_CHANNEL_515nm_F4);
  spectrum_dict["555nm"] = as7341.getChannel(AS7341_CHANNEL_555nm_F5);
  spectrum_dict["590nm"] = as7341.getChannel(AS7341_CHANNEL_590nm_F6);
  spectrum_dict["630nm"] = as7341.getChannel(AS7341_CHANNEL_630nm_F7);
  spectrum_dict["680nm"] = as7341.getChannel(AS7341_CHANNEL_680nm_F8);
  spectrum_dict["clear"] = as7341.getChannel(AS7341_CHANNEL_CLEAR);
  spectrum_dict["nir"] = as7341.getChannel(AS7341_CHANNEL_NIR);

  // Serialized JSON to a string
  char jsonBuffer[512];
  serializeJson(spectrum_dict , jsonBuffer); // first arugment is the input object and the second object is an target object

  // publish the json object to MQTT 
  if (mqttClient.beginMessage("Spectrum/node1")){
    
    mqttClient.print(jsonBuffer);
    mqttClient.endMessage();
    Serial.println("Published JSON:");
    Serial.println(jsonBuffer);

  } else{
    
    Serial.println("Fail to publish JSON");
  }


}