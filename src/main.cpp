#ifndef FASTLED_INTERNAL
#define FASTLED_INTERNAL
#endif
#ifndef DEBUG_ENABLE
#define DEBUG_DISABLED
#endif

#include <Arduino.h>
#include <FastLED.h>
#include <globals.h>
#include "HexlightCluster.h"
static HexlightCluster cluster;

#include "mqtt_com.h"

#include <ArduinoOTA.h>

#ifdef OTAPORT
const uint16_t OTAport = OTAPORT;
#else
const uint16_t OTAport = 8266;
#endif

#ifdef SENSORNAME
const char *sensorName = STR(SENSORNAME);
#else
const char *sensorName = "sensor";
#endif

void setup()
{
#if defined(DEBUGENABLE) && defined(USE_SERIAL)
    Serial.begin(115200);
    while (!Serial)
    {
    }
    Serial.println("setup beginning");
    Serial.println(STR(SENSORNAME));
#endif

    CFastLED::addLeds<WS2812B, LED_PIN, COLOR_ORDER>(g_LEDS, NUM_LEDS, 0);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, PSU_MA);
    FastLED.showColor(CRGB::Black);

    ArduinoOTA.setPort(OTAport);
    ArduinoOTA.setHostname(STR(SENSORNAME));
    ArduinoOTA.setPassword(STR(OTA_PASSWORD));
    ArduinoOTA.setMdnsEnabled(true);
    ArduinoOTA.setRebootOnSuccess(true);

    random16_add_entropy(esp_random());
    cluster.begin();
    cluster.color = CRGB(desiredState.red, desiredState.green, desiredState.blue);
    cluster.setMode("tree"); // before MQTT connects, so the first reported state is right
#ifdef HA_DISCOVERY
    client.setMaxPacketSize(2048); // discovery payload exceeds PubSubClient's 256-byte default
    client.enableLastWillMessage(availability_topic, "offline", true); // must precede first client.loop()
#endif

    while (!client.isConnected())
    {
        client.loop();
        delay(300);
        DEBUG("waiting for client to connect before initializing ArduinoOTA");
    }

    ArduinoOTA.begin();
#ifdef DEBUGENABLE
#ifdef USE_SERIAL
    Debug.setSerialEnabled(true);
#else
    Debug.setSerialEnabled(false);
#endif
    Debug.setResetCmdEnabled(true);
    Debug.showProfiler(true);
    static String debugHost = sensorName; // begin() takes String*
    Debug.begin(&debugHost, RemoteDebug::DEBUG);
#endif
}

void loop()
{
    ArduinoOTA.handle();
    client.loop();
    currentState.powerStateOn = desiredState.powerStateOn;
    if (!currentState.powerStateOn)
    {
        FastLED.clear();
        FastLED.show();
    }
    else if (cluster.loop())
    {
        sendState(); // the effect ended itself (alert), report the mode it went back to
    }

#if defined(DEBUG_ENABLE)
    Debug.handle();
#endif
}
