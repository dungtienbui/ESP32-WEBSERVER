#include <Arduino.h>

// import some libraries

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "RelayStatus.h"

// control relay
#define TXD 8
#define RXD 9
#define BAUD_RATE 9600

// Replace with your network credentials
const char *ssid = "D.Wifi";
const char *password = "Dung12345";
// const char *ssid = "ACLAB";
// const char *password = "ACLAB2023";

// Adafruit MQTT info
const char *mqtt_server = "io.adafruit.com";
const int port = 1883;
const char *adafruit_username = "DungBui";
const char *adafruit_password = "";
const char *adafruit_client = "SERVER_ESP32";

WiFiClient espClient;
PubSubClient client(espClient);

// define function for MQTT
char msg[50];

void callback(char *topic, byte *message, unsigned int length);
void subAllTopicAdafruitMQTT();
void reconnect();
// void TaskCheckMQTTConnect(void *pvParameters);

// initial preferences to load setting from memory

// load an array of relay
// Preferences preferences;





// struct for relay
struct Relay
{
  bool state;  // on / off
  bool isUsed; // assign to a device
  String relay_name;
};

Relay relay_arr[31];

void init_relay()
{
  Serial2.begin(BAUD_RATE, SERIAL_8N1, TXD, RXD);

  for (int i = 0; i < 31; ++i)
  {
    relay_arr[i].relay_name = "Device " + String(i);
    relay_arr[i].state = 0;
    relay_arr[i].isUsed = i % 9  % 2;
  }
}

void sendModbusCommand(const uint8_t command[], size_t length)
{
  for (size_t i = 0; i < length; i++)
  {
    Serial2.write(command[i]);
  }
}

// Set LED GPIO
const int ledPin = 48;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /monitor_env
AsyncEventSource events("/monitor_env");

// Create an Websocket
AsyncWebSocket ws("/ws");

// websocket functions
void notifyClients(String str)
{
  ws.textAll(str);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;

    JsonDocument doc;
    deserializeJson(doc, (char *)data);

    Serial.print("Websocket: ");
    serializeJson(doc, Serial);

    int index = int(doc["id"]) - 1;

    if (doc["action"] == "toggle")
    {
      relay_arr[index].state = !relay_arr[index].state;

      if (relay_arr[index].state)
      {
        sendModbusCommand(relay_ON[index], sizeof(relay_ON[0]));
      }
      else
      {
        sendModbusCommand(relay_OFF[index], sizeof(relay_OFF[0]));
      }

      Serial.print("RELAY: ");
      Serial.print(index);
      Serial.print("; STATE: ");
      Serial.print(relay_arr[index].state);

      String str = "{\"id\":" + String(int(doc["id"])) + ", \"state\":" + String(relay_arr[index].state) + "}";
      notifyClients(str);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void initWebServerEvent()
{
  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000); });
  server.addHandler(&events);
}

// template
// R"rawliteral(
//       <div class="toggle">
//           <div class="toggle_head">
//               <div class="name">Lighter</div>
//               <div class="relay">Relay 1</div>
//           </div>
//           <input class="switch" type="checkbox">
//       </div>
//     )rawliteral";

//-------define processor-------------//
// process for control relay
String processor_control_relay(const String &var)
{
  if (var == "TOGGLES")
  {
    String relay_list_str = "";
    int i = 0;
    for (Relay relay : relay_arr)
    {
      if (relay.isUsed)
      {
        relay_list_str += R"rawliteral(<div class="control_relay_block"><div class="relay_name"><div>)rawliteral" + relay.relay_name + R"rawliteral(</div><div>Relay )rawliteral" + i + R"rawliteral(</div></div><div class="state"><span>State of device: </span><span id="relay_state)rawliteral" + i + "\"" + R"rawliteral(style="color:)rawliteral" + (relay.state ? "blue" : "red") + R"rawliteral(;">)rawliteral" + (relay.state ? "ON" : "OFF") + R"rawliteral(</span></div><button id=")rawliteral" + i + R"rawliteral(">ToggLe</button></div>)rawliteral";
      }
      i = i + 1;
    }
    return relay_list_str;
  }
  return String();
}

// processor for monitor page (home page)
String processor_monitor(const String &var)
{
  // Serial.println(var);
  // if (var == "TEMPERATURE")
  // {
  //   return String("loading...");
  // }
  // else if (var == "HUMIDITY")
  // {
  //   return String("loading...");
  // }
  // else if (var == "LIGHT")
  // {
  //   return String("loading...");
  // }
  // else if (var == "MOISTURE")
  // {
  //   return String("loading...");
  // }
  return String();
}

// processor for set relay page
String processor_relay_setting(const String &var)
{
  String str = "";
  if (var == "SELECTION_TO_ADD")
  {
    for (int i = 0; i < 31; ++i)
    {
      if (!relay_arr[i].isUsed)
      {
        str += R"rawliteral(<option value=")rawliteral" + String(i) + "\">Relay " + String(i) + R"rawliteral(</option>)rawliteral";
      }
    }
  }
  else if (var == "SELECTION_TO_REMOVE")
  {
    for (int i = 0; i < 31; ++i)
    {
      if (relay_arr[i].isUsed)
      {
        str += R"rawliteral(<option value=")rawliteral" + String(i) + "\">Relay " + String(i) + " - " + String(relay_arr[i].relay_name) + R"rawliteral(</option>)rawliteral";
      }
    }
  }
  return str;
}

// processor
String processor(const String &var)
{
  return String("<h2>UNDEFINED</h2>");
}

//-------end define processor-------------//

void setup()
{
  
  // setup
  // preferences.begin("my-app", false); 
  
  init_relay();

  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(3000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Set MQTT adafruit
  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  // xTaskCreate(TaskCheckMQTTConnect, "Task check MQTT connect (3s).", 4096, NULL, 2, NULL);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", String(), false, processor_monitor); });

  server.on("/monitor_SSE.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/monitor_SSE.js", "text/javascript"); });

  server.on("/control_relay_WS.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/control_relay_WS.js", "text/javascript"); });

  // Route to load style.css file
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/styles.css", "text/css"); });

  // Route to load logo
  server.on("/iot.svg", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/iot.svg", "image/svg+xml"); });

  // Route to load icon
  server.on("/light.svg", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/light.svg", "image/svg+xml"); });
  server.on("/soil-moisture.svg", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/soil-moisture.svg", "image/svg+xml"); });
  server.on("/temperature.svg", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/temperature.svg", "image/svg+xml"); });
  server.on("/humidity.svg", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/humidity.svg", "image/svg+xml"); });

  // Route for control device page
  server.on("/control_device.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/control_device.html", String(), false, processor_control_relay); });

  // Route for set relay page
  server.on("/set_relay.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/set_relay.html", String(), false, processor_relay_setting); });

  server.on("/add-relay", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
        String relaynumber_str;
        String relayname_str;

        if (request->hasParam("relayname") && request->hasParam("relaynumber")) {
          relaynumber_str = request->getParam("relaynumber")->value();
          relayname_str = request->getParam("relayname")->value();
          relay_arr[relaynumber_str.toInt()].isUsed = 1;
          relay_arr[relaynumber_str.toInt()].relay_name = relayname_str;
          relay_arr[relaynumber_str.toInt()].state = 0;
        }
        else {
          request->send(500, "text/plain", "Error");
        }

        request->redirect("/control_device.html"); });

  server.on("/remove-relay", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
        String relaynumber_str;

        if (request->hasParam("relaynumber")) {
          relaynumber_str = request->getParam("relaynumber")->value();
          relay_arr[relaynumber_str.toInt()].isUsed = 0;
          relay_arr[relaynumber_str.toInt()].relay_name = "";
          relay_arr[relaynumber_str.toInt()].state = 0;
        }
        else {
          request->send(500, "text/plain", "Error");
        }

        request->redirect("/control_device.html"); });

  initWebSocket();
  initWebServerEvent();

  // Start server
  server.begin();

  Serial.println("WEB SERVER STARTED");
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// *******************  ******************* //
// ---------define function for subsubclient--------

void callback(char *topic, byte *message, unsigned int length)
{
  for (int i = 0; i < length; i++) {
    msg[i] = (char)message[i];
  }
  msg[length] = '\0';

  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(msg);

  if (String(topic) == "DungBui/feeds/sensor.temperature")
  {
    events.send(msg, "temperature", millis());
  }
  else if (String(topic) == "DungBui/feeds/sensor.light")
  {
    events.send(msg, "light", millis());
  }
  else if (String(topic) == "DungBui/feeds/sensor.moisture")
  {
    events.send(msg, "moisture", millis());
  }
  else if (String(topic) == "DungBui/feeds/sensor.humidity")
  {
    events.send(msg, "humidity", millis());
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(adafruit_client, adafruit_username, adafruit_password))
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
  client.subscribe("DungBui/feeds/sensor.temperature");
  client.subscribe("DungBui/feeds/sensor.humidity");
  client.subscribe("DungBui/feeds/sensor.light");
  client.subscribe("DungBui/feeds/sensor.moisture");
  return;
}

// void TaskCheckMQTTConnect(void *pvParameters)
// {
//   while (1)
//   {
//     if (!client.connected())
//     {
//       reconnect();
//     }
//     client.loop();
//     delay(3000);
//   }
// }