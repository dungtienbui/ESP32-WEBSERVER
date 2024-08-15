#include <Arduino.h>

/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <DHT20.h>
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7*3600;
const int   daylightOffset_sec = 3600;


#define leb_pin 48


// Replace the next variables with your SSID/Password combination
const char *ssid = "ACLAB";
const char *password = "ACLAB2023";
// const char *ssid = "D.Wifi";
// const char *password = "Dung12345";

// set up wifi function
void setup_wifi();

// Adafruit MQTT info
const char *mqtt_server = "io.adafruit.com";
const int port = 1883;

#define ADA_CLIENT  "dungbui2392003_0"

#define IO_USERNAME  "DungBui"
#define IO_KEY       ""


// Initialize MQTT 
WiFiClient espClient0;
WiFiClient espClient1;
WiFiClient espClient2;
PubSubClient client0(espClient0);
PubSubClient client1(espClient1);
PubSubClient client2(espClient2);


// define function for MQTT
void callback(char *topic, byte *message, unsigned int length);
void subAllTopicAdafruitMQTT();
void connectMQTT(PubSubClient * client, const char * client_name);



// Initialize DHT20 sensor
DHT20 DHT;
#define SCL 12
#define SDA 11
#define BAUD_RATE 9600


// Define task for RTOS
void TaskBlink(void *pvParameters);
void TaskLightSensor(void *pvParameters);
void TaskDHT20Sensor(void *pvParameters);
void TaskMoistureSensor(void *pvParameters);

// timer
void printLocalTime();
String getCurrentTimeStamp();




// ******************* SETUP ******************* //
void setup()
{
  // A0 -> light
  pinMode(A0, INPUT);
  // A1 -> moisture
  pinMode(A1, INPUT);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  Serial.begin(115200);

  Wire.begin(SDA, SCL);
  DHT.begin();

  setup_wifi();
  client0.setServer(mqtt_server, port);
  client1.setServer(mqtt_server, port);
  client2.setServer(mqtt_server, port);
  client0.setCallback(callback);
  client1.setCallback(callback);
  client2.setCallback(callback);

  connectMQTT(&client0, "light_sensor");
  connectMQTT(&client1, "Moisture_sensor");
  connectMQTT(&client2, "DHT20_sensor");

  xTaskCreate( TaskBlink, "Task Blink" ,2048  ,NULL  ,2 , NULL);

  xTaskCreate(TaskLightSensor, "Task read light sensor and public data to adafruit MQTT broker.", 4096, NULL, 2, NULL);
  xTaskCreate(TaskMoistureSensor, "Task read moisture.", 4096, NULL, 1, NULL);
  xTaskCreate(TaskDHT20Sensor, "Task read temperature and humidity.", 4096, NULL, 1, NULL);
}
// *******************  ******************* //

// ---------set up wifi--------

void setup_wifi()
{
  
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// *******************  ******************* //
// ---------define function for subsubclient--------

void callback(char *topic, byte *message, unsigned int length)
{
}

void connectMQTT(PubSubClient * client, const char * client_name)
{
  while (!client->connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client->connect(client_name, IO_USERNAME, IO_KEY))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client->state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
  client->loop();
}

void subAllTopicAdafruitMQTT()
{
  // client.subscribe("DungBui/feeds/sensor.temperature");
  return;
}

// ------after----------
void loop()
{
}



// *******************  ******************* //
// ------define task for RTOS----------
void TaskLightSensor(void *pvParameters)
{
  String data = "";

  while (1)
  {
    connectMQTT(&client0, "light_sensor");

    data = "{\"timestamp\":\"" + getCurrentTimeStamp() + "\", \"data\":" + String(int(analogRead(A0))) + "}";

    Serial.println("Light sensor upload data: ");
    Serial.println(data.c_str());

    client0.publish("DungBui/feeds/sensor.light", data.c_str());
    delay(10000);
  }
}

void TaskDHT20Sensor(void *pvParameters) {
  String data = "";
  while (1)
  {
    connectMQTT(&client2, "DHT20_sensor");

    DHT.read();

    // public TEMPERATURE VALUE to MQTT
    data = "{\"timestamp\":\"" + getCurrentTimeStamp() + "\", \"data\":" + String(DHT.getTemperature()) + "}";

    // data = "{\"data\":" + String(millis()%100) + "}";

    Serial.println("DHT20 sensor (TEMP) upload data: ");
    Serial.println(data.c_str());

    client2.publish("DungBui/feeds/sensor.temperature", data.c_str());

    // public HUMIDITY VALUE to MQTT
    data = "{\"timestamp\":\"" + getCurrentTimeStamp() + "\", \"data\":" + String(DHT.getHumidity()) + "}";

    Serial.println("DHT20 sensor (HUMI) upload data: ");
    Serial.println(data.c_str());
    client2.publish("DungBui/feeds/sensor.humidity", data.c_str());


    delay(10000);
  }

}

void TaskMoistureSensor(void *pvParameters) {
  String data = "";

  while (1)
  {
    connectMQTT(&client1, "Moisture_sensor");

    data = "{\"timestamp\":\"" + getCurrentTimeStamp() + "\", \"data\":" + String(map(analogRead(A1), 0, 4095, 0, 100)) + "}";

    // data = "{\"data\":" + String(millis()%100) + "}";

    Serial.println("Moisture sensor upload data: ");
    Serial.println(data.c_str());

    client1.publish("DungBui/feeds/sensor.moisture", data.c_str());
    delay(10000);
  }
}



void TaskBlink(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(leb_pin, OUTPUT);
  

  while(1) {                          
    digitalWrite(leb_pin, HIGH);  // turn the LED ON
    delay(2000);
    digitalWrite(leb_pin, LOW);  // turn the LED OFF
    delay(2000);
  }
}


void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");
}


String getCurrentTimeStamp() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "00:00:00 01-01-1900";
  }

  String timeStamp = "";
  timeStamp = String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec) 
                          + " " + String(timeinfo.tm_mday) + "-" + String(timeinfo.tm_mon + 1) + "-" + String(timeinfo.tm_year + 1900);
                          
  return timeStamp;
}
