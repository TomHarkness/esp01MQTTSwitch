#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#define CLIENT_ID "Wilson"

//MQTT server, client & topic settings
const char* mqtt_server = "";      //MQTT server address
const char* mqtt_topic = "home/IOT";          //topic to publish to
const char* mqtt_username = "";   //MQTT username (if required)
const char* mqtt_password = "";   //MQTT password (if required, leave blank for no password)
const char* clientID = "";          //client id identifies the ESP8266 device, This MUST be unique!
const int relayPin = 0;
//MQTT Message buffer
char message_buff[100];
//Wi-Fi Settings
const char* ssid = ""; //YWi-Fi SSID
const char* wifi_password = ""; //Wi-Fi Password
//last MQTT connection attempt time (for auto reconnect)
long lastReconnectAttempt = 0;

// Initialise the WiFi and MQTT Clients
WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
  //WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Wi-Fi Connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else if (WiFi.status() == WL_CONNECT_FAILED) {
        Serial.println("Wi-Fi Failed!");
    }
    delay(500);
}

void callback(char* topic, byte* payload, unsigned int length) {
      Serial.print("Message Arrived");
      Serial.println();
      for (unsigned int i=0;i<length;i++) {
          message_buff[i] = payload[i];
      }
       Serial.println("-----------------------");
       String msgString = String(message_buff);
       Serial.println("Payload: " + msgString);
       Serial.println("-----------------------");

      if(msgString.indexOf("toggleWilson") ==0){
        digitalWrite(relayPin, LOW);
        delay(50);
        digitalWrite(relayPin, HIGH);
        Serial.println("Wilsons switch was Toggled for 50ms");
      }
}

boolean reconnect() {
  if (client.connect(clientID, mqtt_username, mqtt_password)) {     // Once connected, publish an announcement...
      client.publish("home/ConnectionLogs","Wilson has connected...");
    client.subscribe(mqtt_topic);       // ... and resubscribe
  }
  return client.connected();
}
//Function to keep the MQTT connection alive
void keepMQTTAllive() {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
          if (client.connected()) {
            Serial.println("MQTT Connected");
        }
      }
    }
    return;
  }

void setup() {

  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  setup_wifi();
  delay(10);

  //connect to MQTT Server
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
     ArduinoOTA.setHostname("Wilson");
  // No authentication by default
  // ArduinoOTA.setPassword("admin");
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
    ArduinoOTA.begin();
    Serial.println("OTA Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}


void loop() {
  ArduinoOTA.handle();
  //Keep MQTT connection alive
  if (!client.connected()) {
    keepMQTTAllive();
    Serial.println("MQTT Failed.... Retrying");
  }
  else{
    // Client connected

    client.loop();
  }

}
