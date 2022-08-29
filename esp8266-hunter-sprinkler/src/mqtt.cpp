#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include <mqtt.h>
#include <web_server_scheduled.h>
#include <wifi.h>

String device_hostname = HOSTNAME;
String TopicCheckIn = "hunter/" + device_hostname + "/checkIn";
String TopicResult = "hunter/" + device_hostname + "/result";
String TopicZone = "hunter/" + device_hostname + "/zone/";
String TopicProgram = "hunter/" + device_hostname + "/program/";

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_subscribe_topics() {
  for (int i = 1; i <= 48; i++) {
    String zone_number = String(i);
    String TopicZoneFull = TopicZone + zone_number;
    client.subscribe(TopicZoneFull.c_str());
  };
  for (int i = 1; i <= 4; i++) {
    String program_number = String(i);
    String TopicProgramFull = TopicProgram + program_number;
    client.subscribe(TopicProgramFull.c_str());
  };
}

void mqtt_connect(const char *MQTT_USER, const char *MQTT_PASSWORD) {
  bool boot = true;
  // Attempt to connect
  if (client.connect(HOSTNAME, MQTT_USER, MQTT_PASSWORD, TopicCheckIn.c_str(),
                     0, 0, "Dead Somewhere")) {
    // Once connected, publish an announcement...
    if (boot == false) {
      client.publish(TopicCheckIn.c_str(), "Reconnected");
    }
    if (boot == true) {
      client.publish(TopicCheckIn.c_str(), "Rebooted");
      boot = false;
    }
    // ... and resubscribe
    mqtt_subscribe_topics();
  }
}

void mqttPublishResult(const char *toSend) {
  client.publish(TopicResult.c_str(), toSend);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  String newTopic = topic;
  payload[length] = '\0';
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, String((char *)payload));
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  String action = doc["action"];
  byte time = doc["time"];
  for (byte i = 1; i <= 48; i++) {
    String zone_number = String(i);
    String TopicZoneFull = TopicZone + zone_number;
    if (newTopic == TopicZoneFull.c_str()) {
      if (action == "start") {
        startZone(i, time, "");
      }
      if (action == "stop") {
        stopZone(i, "");
      }
    }
  };
  for (byte i = 1; i <= 4; i++) {
    String program_number = String(i);
    String TopicProgramFull = TopicProgram + program_number;
    if (newTopic == TopicProgramFull.c_str()) {
      if (action == "start") {
        startProgram(i, "");
      }
    }
  };
}