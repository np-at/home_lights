//#include "secrets.h"
#include <Arduino.h>
#include <FastLED.h>
#include "globals.h"
#include "solid.h"
#include "comet.h"
#include "twinkle.h"
#include "mqtt_com.h"
//#include "WiFi.h"
//#include <ESPmDNS.h>
//#include <WiFiUdp.h>
//#include "WiFiClientSecure.h"
#include <ArduinoOTA.h>

//#define DBG(fmt,...) if (DEBUGENABLE) DEBUG(fmt, ##__VA_ARGS__)
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define ST(A) #A
#define STR(A) ST(A)

#define FASTLED_INTERNAL
////// DECLARATIONS
#ifdef OTAPORT
const uint16_t OTAport = OTAPORT
#else
const uint16_t OTAport = 8266;
#endif

#ifdef SENSORNAME
const char *sensorName = STR(SENSORNAME);
#else
const char* sensorName = "sensor";
#endif

static SolidEffect* solidEffect;
static TwinkleLightEffect* twinkleLightEffect;
static CometLightEffect* cometLightEffect;

void setup() {
    // Setup LEDs
    static auto comets = CometLightEffect();
    cometLightEffect = &comets;
    static auto solid = SolidEffect();
    solidEffect = &solid;
    static auto twinkle = TwinkleLightEffect();
    twinkleLightEffect = &twinkle;
    pinMode(LED_PIN, OUTPUT);
#ifdef DEBUGENABLE

#ifdef USE_SERIAL
    Serial.begin(115200);
    while (!Serial) {
    }
    Serial.println("setup beginning");
    Serial.println(STR(SENSORNAME));
#endif

#endif
    currentState = State();
    currentState.powerLimit = MAX_POWER_LIMIT;
    desiredState = State();
    desiredState.powerLimit = MAX_POWER_LIMIT;

    CFastLED::addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(g_LEDS, NUM_LEDS); // Add our LED strip to the FastLED library
    // set_max_power_indicator_LED(LED_BUILTIN);

//    max_Brightness = calculate_max_brightness_for_power_mW(FastLED.leds(), NUM_LEDS, 255, MAX_POWER_LIMIT);
//    g_DesiredBrightness = (u_int8_t) max_Brightness;
    FastLED.setMaxPowerInMilliWatts(MAX_POWER_LIMIT);

    // / Light the builtin LED if we power throttle
//    FastLED.setBrightness(g_Brightness);

    //OTA SETUP
    ArduinoOTA.setPort(OTAport);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(STR(SENSORNAME));

    // No authentication by default

    ArduinoOTA.setPassword(STR(OTA_PASSWORD));
    ArduinoOTA.setMdnsEnabled(true);
#ifdef DEBUGENABLE
    // if (Debug.isActive(RemoteDebug::ANY))
    // {
    //     ArduinoOTA.onStart([]()
    //                        { Debug.println("Starting"); });
    //     ArduinoOTA.onEnd([]()
    //                      { Debug.println("\nEnd"); });
    //     ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    //                           { Debug.printf("Progress: %u%%\r", (progress / (total / 100))); });
    //     ArduinoOTA.onError([](ota_error_t error)
    //                        {
    //                            Serial.printf("Error[%u]: ", error);
    //                            if (error == OTA_AUTH_ERROR)
    //                                Debug.println("Auth Failed");
    //                            else if (error == OTA_BEGIN_ERROR)
    //                                Debug.println("Begin Failed");
    //                            else if (error == OTA_CONNECT_ERROR)
    //                                Debug.println("Connect Failed");
    //                            else if (error == OTA_RECEIVE_ERROR)
    //                                Debug.println("Receive Failed");
    //                            else if (error == OTA_END_ERROR)
    //                                Debug.println("End Failed");
    //                        });
    // }

#endif
    ArduinoOTA.setRebootOnSuccess(true);

    // SETUP mqtt
    // initEspMQTT(&client);

    while (!client.isConnected()) {
        client.loop();
        delay(300);
        DBG("waiting for client to connect before initializing ArduinoOTA");
    }

    ArduinoOTA.begin();
#ifdef DEBUGENABLE
    Debug.begin((String *) sensorName, RemoteDebug::VERBOSE);
    Debug.setResetCmdEnabled(true); // Enable the reset command
    Debug.showProfiler(true); // To show profiler - time between messages of Debug
#endif

    // FastLED.clear();
    if (effect == LightEffect::SOLID)
        setupSolid();

//    auto a = SolidParams();
//    a.brightness=g_Brightness;
//    a.blue=blue;
//    a.red=red;
//    a.green=green;
//    solidParams=&a;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"

void loop() {
    client.loop();
    ArduinoOTA.handle();

    if (currentState.powerStateOn || isTransitioning) {

        // DRAW Stuff
        switch (currentState.Effect) {
            case LightEffect::COMET:
                if (isTransitioning)
                    isTransitioning = cometLightEffect->handleDiff_step(&desiredState, &currentState);
                cometLightEffect->DrawEffect(&currentState);
//                DrawComet();
                break;
            case LightEffect::TWINKLE:
                if (isTransitioning)
                    isTransitioning = twinkleLightEffect->handleDiff_step(&desiredState, &currentState);
                twinkleLightEffect->DrawEffect(&currentState);
//                DrawTwinkle3();
                break;
            case LightEffect::SOLID:
                if (isTransitioning)
                    isTransitioning = solidEffect->handleDiff_step(&desiredState, &currentState);
                solidEffect->DrawEffect(&currentState);
                break;
            default:
                break;
        }
        if (desiredState.Effect != currentState.Effect) {
            currentState.Effect = desiredState.Effect;
            isTransitioning = true;
        }
//        if (isTransitioning) {
//            sendState();
//        }
    } else {
        FastLED.clear();
    }
    EVERY_N_BSECONDS(TIME_INCREMENT_SECONDS)
    {
        g_timeIncrementSinceStateChange++;
        DBG("Time Since State Change = 10 * %lu \n", g_timeIncrementSinceStateChange);
    }
    //    Only apply new LED config to lights for the first X duration after a state change, otherwise just delay and wait

    if (currentState.Effect == LightEffect::SOLID && (g_timeIncrementSinceStateChange > g_c_INCREMENT_COUNT_THRESHOLD)) {

        if (!isTransitioning)
            delay(g_DelayValue * 2);
    } else {
        if (g_HasReceivedFirstMessage)
            FastLED.show(currentState.getBrightness());
    }

#ifdef DEBUGENABLE
    EVERY_N_SECONDS(10)
    {
        Debug.printf("Colors: { r: %i, b: %i, g: %i } \n", currentState.red, currentState.blue, currentState.green);
        Debug.printf("brightness: %i \n", currentState.brightness);
        Debug.printf("desired brightness: %i \n", currentState.desiredBrightness);
        Debug.println("");
        Debug.printf("DelayValue: %i \n", g_DelayValue);
        Debug.printf("TransitionDelay: %i \n", g_TransitionDelay);
        Debug.printf("DelayMultiplier: %f \n", g_DelayMultiplier);
        Debug.println("");
        Debug.printf("Current Effect: %s \n", lightEffectToString(currentState.Effect));
        Debug.printf("power state: %s \n", currentState.powerStateOn ? "on":"off");
        Debug.println(" -------------- ----------------");
    }
//     Remote debug over WiFi
#ifndef USE_SERIAL
    Debug.handle();
#else
    // Or

    // debugHandle(); // Equal to SerialDebug
#endif
#endif

    // HANDLE Transition speed
    if (!isTransitioning)
        delay(g_DelayValue);
}

#pragma clang diagnostic pop