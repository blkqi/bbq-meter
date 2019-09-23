#include <Arduino.h>
#include <ELClient.h>
#include <ELClientMqtt.h>
#include <TempProbe.h>

#define DEBUG 1
#define READ_PERIOD 1000 //ms
#define READS_PER_PUBLISH 5

ELClient esp(&Serial, &Serial);
ELClientMqtt mqtt(&esp);

struct MetaProbe {
  TempProbe sensor;
  String topicValue;
  String topicStatus;
};

const size_t probesLen = 4;
MetaProbe probes[probesLen] = {
  {TempProbe(A0), "smoker/probe/1/temp", "smoker/probe/1/status"},
  {TempProbe(A1), "smoker/probe/2/temp", "smoker/probe/2/status"},
  {TempProbe(A2), "smoker/probe/3/temp", "smoker/probe/3/status"},
  {TempProbe(A3), "smoker/probe/4/temp", "smoker/probe/4/status"},
};

void publishProbe(MetaProbe *probe) {
  String value;
  String status;
  switch (probe->sensor.getStatus()) {
    case TempProbe::STATUS_OK:
      value = String(probe->sensor.getTempAvg());
      if (DEBUG) {
        Serial.print(probe->topicValue);
        Serial.print(" -> ");
        Serial.print(value);
        Serial.println();
      }
      mqtt.publish(probe->topicValue.c_str(), value.c_str());
      status = "OK";
      break;
    case TempProbe::STATUS_NONE:
      status = "NONE";
      break;
    default:
      status = "ERROR";
      break;
  }
  mqtt.publish(probe->topicStatus.c_str(), status.c_str());
}

void wifiCb(void* response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      Serial.println("WiFi connected");
    } else {
      Serial.print("WiFi not ready: ");
      Serial.println(status);
    }
  }
}

bool connected;

// Callback when MQTT is connected
void mqttConnected(void* response) {
  Serial.println("MQTT connected!");
  mqtt.publish("smoker/availability", "online");
  connected = true;
}

// Callback when MQTT is disconnected
void mqttDisconnected(void* response) {
  Serial.println("MQTT disconnected");
  connected = false;
}

void mqttPublished(void* response) {
  Serial.println("MQTT published");
}

void setup() {
  Serial.begin(38400);
  Serial.println("EL-Client starting!");

  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks to the wifi status change callback. The callback gets called with the initial
  // status right after Sync() below completes.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");

  // Set-up callbacks for events and initialize with es-link.
  mqtt.connectedCb.attach(mqttConnected);
  mqtt.disconnectedCb.attach(mqttDisconnected);
  mqtt.publishedCb.attach(mqttPublished);
  mqtt.setup();
  mqtt.lwt("smoker/availability", "offline");

  Serial.println("EL-MQTT ready");
}

static int count = 0;
static uint32_t last;

void loop() {
  esp.Process();
  if (connected && (millis() - last) > READ_PERIOD) {
    count = (count + 1) % READS_PER_PUBLISH;
    for (size_t i=0; i<probesLen; ++i) {
      probes[i].sensor.Process();
      if (count == 0) {
        publishProbe(&probes[i]);
      }
      delay(50);
    }
    last = millis();
  }
}
