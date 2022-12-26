#include "ArduinoJson.h"
#include "EspMQTTClient.h"

// WIFI STUFF

#ifdef WIFI_PASSWORD
const char *password = STR(WIFI_PASSWORD);
#endif

#ifdef WIFI_SSID
#ifdef DEBUGENABLE
//#pragma message STR(WIFI_SSID)
#endif
const char *ssid = STR(WIFI_SSID);
#endif

#ifdef MQTT_SERVER
// #pragma message STR(MQTT_SERVER)
const char *mqtt_server = STR(MQTT_SERVER);
#else
#pragma error1 - Please supply mqtt server as build flag: MQTT_SERVER
#endif

#ifdef MQTT_PORT
const int mqtt_port = MQTT_PORT;
#else
#pragma warning "Defaulting to mqtt port = 1883"
const int mqtt_port = 1883;
#endif

#ifdef MQTT_USERNAME
const char *mqtt_username = STR(MQTT_USERNAME);
#else
const char *mqtt_username = "";
#endif

#ifdef MQTT_PASSWORD
const char *mqtt_password = STR(MQTT_PASSWORD);
#else
const char *mqtt_password = "";
#endif
#ifndef MQTT_PREFIX
#define MQTT_PREFIX "home"
#endif

#ifdef LIGHT_STATE_TOPIC
const char *light_state_topic = STR(LIGHT_STATE_TOPIC);
#endif
#ifdef LIGHT_SET_TOPIC
const char *light_set_topic = STR(LIGHT_SET_TOPIC); // "home/houselet_light1/set";
#endif
#ifdef LIGHT_SET_TOPIC_GROUP
const char *light_set_topic_group = STR(LIGHT_SET_TOPIC_GROUP); // "home/LED_GROUP1/set";
#endif


// ************ JSON ***//
const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);

EspMQTTClient client(
        ssid,
        password,
        mqtt_server,
        mqtt_username,
        mqtt_password,
        STR(SENSORNAME),
        1883
);

static const char *on_cmd = "ON";
static const char *off_cmd = "OFF";


void initEspMQTT(EspMQTTClient *espClient) {
    client.setWifiCredentials(ssid, password);
    client.setMqttServer(mqtt_server, mqtt_username, mqtt_password);
    client.setMqttClientName(STR(SENSORNAME));
    // client.setOnConnectionEstablishedCallback(onConnectionEstablished);

}

void sendState() {
    StaticJsonDocument<BUFFER_SIZE> root;

    root[c_STR_State] = (desiredState.powerStateOn) ? on_cmd : off_cmd;
    JsonObject color = root.createNestedObject("color");
    color["r"] = desiredState.red;
    color["g"] = desiredState.green;
    color["b"] = desiredState.blue;

    root["brightness"] = desiredState.desiredBrightness;
    root["effect"] = lightEffectToString(desiredState.Effect);
    root["transition"] = g_TransitionDelay;

    char buffer[measureJson(root) + 1];
    serializeJson(root, buffer, sizeof(buffer));
    // Debug.printf("serialized json: %s \n", buffer);
    client.publish(light_state_topic, buffer, false);
//   if (!success) {
    // Debug.println("failed to publish state update");
    // }
}

void callBack(const String &message) {
    DBG("callback triggered: \n %s", message.c_str());


    StaticJsonDocument<BUFFER_SIZE> jsonBuffer;
    auto error = ArduinoJson::deserializeJson(jsonBuffer, message, DeserializationOption::NestingLimit());
    if (error) {
        // Debug.print(F("deserializeJson() failed with code "));
        // Debug.println(error.c_str());

         DBG("deserializeJson() failed with code %s", error.c_str());

        return;
    }
    // assume a new message means a state change, let the diff handlers sort out the rest
    isTransitioning = true;
    if (jsonBuffer.containsKey(c_STR_State)) {
        if (strcmp(jsonBuffer[c_STR_State], on_cmd) == 0) {
            desiredState.powerStateOn = true;
//            currentState.powerStateOn = true;
        } else if (strcmp(jsonBuffer[c_STR_State], off_cmd) == 0) {
            desiredState.powerStateOn = false;
        }
    }
    if (jsonBuffer.containsKey(c_STR_Effect)) {
        if (strcmp(jsonBuffer[c_STR_Effect], twinkle) == 0) {
            desiredState.Effect = LightEffect::TWINKLE;
//            setupTwinkle();
        } else if (strcmp(jsonBuffer[c_STR_Effect], solid) == 0) {
            desiredState.Effect = LightEffect::SOLID;
//            setupSolid();
        } else if (strcmp(jsonBuffer[c_STR_Effect], comet) == 0) {
            desiredState.Effect = LightEffect::COMET;
//            setupComet();
        } else if (strcmp(jsonBuffer[c_STR_Effect], breathe) == 0) {
            desiredState.Effect = LightEffect::BREATHE;
            //            setupBreathe();
        }
    }
    if (jsonBuffer.containsKey(cs_Brightness)) {
        desiredState.setBrightness(jsonBuffer[cs_Brightness]);
//        desiredState.desiredBrightness=jsonBuffer[cs_Brightness];
//        auto mb = (float) calculate_max_brightness_for_power_mW(255, g_PowerLimit);

//        g_DesiredBrightness = jsonBuffer[cs_Brightness];
//        desiredState.desiredBrightness = jsonBuffer[cs_Brightness];
        // Implement the real brightness value as a proportionate equivalent of the effective max brigntness level
//        const byte newBrightness = uint8_t(((float) g_DesiredBrightness / (float) 255) * mb);
//        desiredState.brightness = newBrightness;




    }
    if (jsonBuffer.containsKey(transition)) {
//        g_TransitionDelay = jsonBuffer[transition];
        desiredState.speed = jsonBuffer[transition];
//        refreshDelay();
    }
    if (jsonBuffer.containsKey(c_STR_Color)) {
        desiredState.red = jsonBuffer[c_STR_Color]["r"];
        desiredState.green = jsonBuffer[c_STR_Color]["g"];
        desiredState.blue = jsonBuffer[c_STR_Color]["b"];
    }
//    if (stateChange) {
//
//        switch (effect) {
//
//            case SOLID:
//                isTransitioning = true;
//
////                SetSolid(jsonBuffer[c_STR_Color]["r"], jsonBuffer[c_STR_Color]["g"], jsonBuffer[c_STR_Color]["b"], )
////                setupSolid(true);
//                break;
//            case TWINKLE:
//                setupTwinkle();
//                break;
//            case COMET:
//                setupComet();
//                break;
//        }
//    }

    if (!g_HasReceivedFirstMessage)
        g_HasReceivedFirstMessage = true;
    g_timeIncrementSinceStateChange = 0;
    sendState();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

void onConnectionEstablished() {
#ifdef DEBUGENABLE
    Debug.println("Connected");
#endif
    client.subscribe(light_set_topic, callBack);
    // client.subscribe(light_set_topic_group, callBack);
    delay(1200);
    // Let things settle
}

#pragma clang diagnostic pop
