#ifndef HOMELED_REWRITE_MQTT_COM_H
#define HOMELED_REWRITE_MQTT_COM_H

#include "ArduinoJson.h"
#include "EspMQTTClient.h"

// WIFI STUFF

#ifdef WIFI_PASSWORD
const char *password = STR(WIFI_PASSWORD);
#endif

#ifdef WIFI_SSID
static const char *ssid = STR(WIFI_SSID);
#endif

#ifdef MQTT_SERVER
static const char *mqtt_server = STR(MQTT_SERVER);
#else
#pragma error1 - Please supply mqtt server as build flag : MQTT_SERVER
#endif

#ifdef MQTT_USERNAME
static const char *mqtt_username = STR(MQTT_USERNAME);
#else
static const char *mqtt_username = "";
#endif

#ifdef MQTT_PASSWORD
static const char *mqtt_password = STR(MQTT_PASSWORD);
#else
static const char *mqtt_password = "";
#endif

#ifdef LIGHT_STATE_TOPIC
static const char *light_state_topic = STR(LIGHT_STATE_TOPIC);
#endif
#ifdef LIGHT_SET_TOPIC
static const char *light_set_topic = STR(LIGHT_SET_TOPIC); // "home/houselet_light1/set";
#endif
#ifdef LIGHT_SET_TOPIC_GROUP
static const char *light_set_topic_group = STR(LIGHT_SET_TOPIC_GROUP); // "home/LED_GROUP1/set";
#endif

// Incoming strings are copied into the document (String input is read-only), so leave room for them.
static const int BUFFER_SIZE = 384;

EspMQTTClient client(
        ssid,
        password,
        mqtt_server,
        mqtt_username,
        mqtt_password,
        STR(SENSORNAME),
        1883);

static const char *on_cmd = "ON";
static const char *off_cmd = "OFF";

void sendState() {
    StaticJsonDocument<BUFFER_SIZE> root;

    root[c_STR_State] = (desiredState.powerStateOn) ? on_cmd : off_cmd;
    JsonObject color = root.createNestedObject("color");
    color["r"] = desiredState.red;
    color["g"] = desiredState.green;
    color["b"] = desiredState.blue;

    root["brightness"] = desiredState.brightness;
    root["effect"] = cluster.effectName();
    root[cs_Speed] = cluster.speed;
#ifdef HA_DISCOVERY
    root["color_mode"] = "rgb";
#endif
    char buffer[measureJson(root) + 1];
    serializeJson(root, buffer, sizeof(buffer));
    if (!client.publish(light_state_topic, buffer, false)) {
        DEBUG("failed to publish state update");
    }
}

void callBack(const String &message) {
    DEBUG("callback triggered: \n %s", message.c_str());

    StaticJsonDocument<BUFFER_SIZE> jsonBuffer;
    auto error = ArduinoJson::deserializeJson(jsonBuffer, message);
    if (error) {
        DEBUG("deserializeJson() failed with code %s", error.c_str());
        return;
    }

    if (jsonBuffer.containsKey(c_STR_State)) {
        if (strcmp(jsonBuffer[c_STR_State], on_cmd) == 0) {
            desiredState.powerStateOn = true;
        } else if (strcmp(jsonBuffer[c_STR_State], off_cmd) == 0) {
            desiredState.powerStateOn = false;
        }
    }

    if (jsonBuffer.containsKey(c_STR_Effect) && !cluster.setMode(jsonBuffer[c_STR_Effect].as<const char *>())) {
        DEBUG("unknown effect %s", jsonBuffer[c_STR_Effect].as<const char *>());
    }

    if (jsonBuffer.containsKey(cs_Brightness)) {
        desiredState.brightness = jsonBuffer[cs_Brightness];
        FastLED.setBrightness(desiredState.brightness);
    }

    // "speed" sets the animation rate: 128 = normal, 255 = ~2x, 64 = half. HA's "transition" (seconds) is ignored.
    if (jsonBuffer.containsKey(cs_Speed)) {
        cluster.speed = constrain(jsonBuffer[cs_Speed].as<int>(), 1, 255);
    }

    if (jsonBuffer.containsKey(c_STR_Color)) {
        desiredState.red = jsonBuffer[c_STR_Color]["r"];
        desiredState.green = jsonBuffer[c_STR_Color]["g"];
        desiredState.blue = jsonBuffer[c_STR_Color]["b"];
        cluster.color = CRGB(desiredState.red, desiredState.green, desiredState.blue);
    }

    sendState();
}

#ifdef HA_DISCOVERY
#ifndef HA_DISCOVERY_PREFIX
#define HA_DISCOVERY_PREFIX homeassistant
#endif
// Retained "online" on connect; broker publishes retained "offline" (last will) when we drop.
static const char *availability_topic = STR(LIGHT_STATE_TOPIC) "/availability";
// Home Assistant MQTT device discovery: https://www.home-assistant.io/integrations/mqtt/#device-discovery-payload
// Retained, so HA picks it up whenever it (re)starts. Needs setMaxPacketSize() > payload size.
void publishDiscovery() {
    DynamicJsonDocument doc(2048);
    JsonObject dev = doc.createNestedObject("dev");
    dev["ids"] = STR(SENSORNAME);
    dev["name"] = STR(DEVICE_NAME);
    dev["mdl"] = "Hexlight";
    doc["o"]["name"] = "homelights";

    JsonObject light = doc.createNestedObject("cmps").createNestedObject("light");
    light["p"] = "light";
    light["unique_id"] = STR(SENSORNAME) "_light";
    light["name"] = nullptr; // entity takes the device name
    light["schema"] = "json";
    light["command_topic"] = light_set_topic;
    light["state_topic"] = light_state_topic;
    light["availability_topic"] = availability_topic;
    light["brightness"] = true;
    light["effect"] = true;
    JsonArray fx = light.createNestedArray("effect_list");
    for (uint8_t i = 0; i < HexlightCluster::effectCount(); i++) fx.add(HexlightCluster::effectName(i));
    light.createNestedArray("supported_color_modes").add("rgb");

    // Speed slider on the same device, as a multiplier (1x = speed 128). Rides the light's own topics.
    JsonObject spd = doc["cmps"].createNestedObject("speed");
    spd["p"] = "number";
    spd["unique_id"] = STR(SENSORNAME) "_speed";
    spd["name"] = "Speed";
    spd["command_topic"] = light_set_topic;
    spd["command_template"] = "{\"speed\": {{ [[(value | float * 128) | round | int, 1] | max, 255] | min }} }";
    spd["state_topic"] = light_state_topic;
    spd["value_template"] = "{{ (value_json.speed / 128) | round(2) }}";
    spd["availability_topic"] = availability_topic;
    spd["min"] = 0.1;
    spd["max"] = 2;
    spd["step"] = 0.05;
    spd["unit_of_measurement"] = "x";
    spd["mode"] = "slider";
    spd["entity_category"] = "config";

    String payload;
    serializeJson(doc, payload);
    DEBUG("HA discovery payload: %d bytes", payload.length());
    if (doc.overflowed() || !client.publish(STR(HA_DISCOVERY_PREFIX) "/device/" STR(SENSORNAME) "/config", payload, true)) {
        DEBUG("failed to publish HA discovery (%d bytes)", payload.length());
    }
}
#endif

void onConnectionEstablished() {
#ifdef DEBUGENABLE
    Debug.println("Connected");
#endif
    client.subscribe(light_set_topic, callBack);
    client.subscribe(light_set_topic_group, callBack);
#ifdef HA_DISCOVERY
    publishDiscovery();
    client.publish(availability_topic, "online", true);
    sendState(); // give HA an initial state
#endif
}

#endif // HOMELED_REWRITE_MQTT_COM_H
