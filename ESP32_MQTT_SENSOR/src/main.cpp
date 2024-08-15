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
WiFiClient espClient;
PubSubClient client(espClient);


// define function for MQTT
void callback(char *topic, byte *message, unsigned int length);
void subAllTopicAdafruitMQTT();
void reconnect();



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

void checkMQTTConnect();


// ******************* SETUP ******************* //
void setup()
{
  // A0 -> light
  pinMode(A0, INPUT);
  // A1 -> moisture
  pinMode(A1, INPUT);

  Serial.begin(115200);

  Wire.begin(SDA, SCL);
  DHT.begin();

  setup_wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  reconnect();

  xTaskCreate( TaskBlink, "Task Blink" ,2048  ,NULL  ,2 , NULL);

  xTaskCreate(TaskLightSensor, "Task read light sensor and public data to adafruit MQTT broker.", 4096, NULL, 2, NULL);
  xTaskCreate(TaskDHT20Sensor, "Task read temperature and humidity.", 4096, NULL, 1, NULL);
  xTaskCreate(TaskMoistureSensor, "Task read moisture.", 4096, NULL, 1, NULL);
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

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(ADA_CLIENT, IO_USERNAME, IO_KEY))
    {
      Serial.println("connected");
      subAllTopicAdafruitMQTT();
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void subAllTopicAdafruitMQTT()
{
  // client.subscribe("DungBui/feeds/sensor.temperature");
  return;
}

void checkMQTTConnect()
{
  if(!client.connected())
    {
      reconnect();
    }
    client.loop();
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
    checkMQTTConnect();

    data = "{\"data\":" + String(int(analogRead(A0))) + "}";
    // data = "{\"data\":" + String(millis()%100) + "}";

    Serial.println("Light sensor upload data: ");
    Serial.println(data.c_str());

    client.publish("DungBui/feeds/sensor.light", data.c_str());
    delay(5000);
  }
}

void TaskDHT20Sensor(void *pvParameters) {
  String data = "";
  while (1)
  {
    checkMQTTConnect();

    DHT.read();

    // public TEMPERATURE VALUE to MQTT
    data = "{\"data\":" + String(DHT.getTemperature()) + "}";

    // data = "{\"data\":" + String(millis()%100) + "}";

    Serial.println("DHT20 sensor (TEMP) upload data: ");
    Serial.println(data.c_str());

    client.publish("DungBui/feeds/sensor.temperature", data.c_str());

    // public HUMIDITY VALUE to MQTT
    data = "{\"data\":" + String(DHT.getHumidity()) + "}";

    Serial.println("DHT20 sensor (HUMI) upload data: ");
    Serial.println(data.c_str());
    client.publish("DungBui/feeds/sensor.humidity", data.c_str());


    delay(13000);
  }

}

void TaskMoistureSensor(void *pvParameters) {
  String data = "";

  while (1)
  {
    checkMQTTConnect(); 

    data = "{\"data\":" + String(int(analogRead(A1))) + "}";

    // data = "{\"data\":" + String(millis()%100) + "}";

    Serial.println("Moisture sensor upload data: ");
    Serial.println(data.c_str());

    client.publish("DungBui/feeds/sensor.moisture", data.c_str());
    delay(9000);
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

