#ifndef FASTLED_INTERNAL
#define FASTLED_INTERNAL
#endif
#ifndef DEBUG_ENABLE
#define DEBUG_DISABLED
#endif

#include <Arduino.h>
// #include "power_mgt.h"

#include <FastLED.h>
#include "globals.h"
#include "utils.h"
#include "hexlight.h"
#include "HexlightCluster.h"
static HexlightCluster cluster = HexlightCluster();

#include "State.h"
#include "solid.h"
#include "breathe.h"
#include "comet.h"
#include "twinkle.h"
#include "mqtt_com.h"

#include <ArduinoOTA.h>

// #define DBG(fmt,...) if (DEBUGENABLE) DEBUG(fmt, ##__VA_ARGS__)
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
const char *sensorName = "sensor";
#endif

static SolidEffect *solidEffect;
static TwinkleLightEffect *twinkleLightEffect;
static CometLightEffect *cometLightEffect;
static BreatheEffect *breatheEffect;

void setup()
{
    // Setup LEDs
    static auto comets = CometLightEffect();
    cometLightEffect = &comets;
    static auto solid = SolidEffect();
    solidEffect = &solid;
    static auto twinkle = TwinkleLightEffect();
    twinkleLightEffect = &twinkle;
    static auto breathe = BreatheEffect();
    breatheEffect = &breathe;
    // pinMode(LED_PIN, OUTPUT);
#ifdef DEBUGENABLE

#ifdef USE_SERIAL
    Serial.begin(115200);
    while (!Serial)
    {
    }
    Serial.println("setup beginning");
    Serial.println(STR(SENSORNAME));
#endif

#endif
    currentState = State();
    currentState.blue = 144;
    currentState.red = 144;
    currentState.green = 144;
    currentState.Effect = LightEffect::RAINBOW;
    currentState.powerLimit = MAX_POWER_LIMIT;
    currentState.brightness = 255;
    currentState.powerStateOn = true;
    desiredState = State();
    desiredState.blue = 144;
    desiredState.red = 144;
    desiredState.green = 144;

    desiredState.powerLimit = MAX_POWER_LIMIT;

    FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(g_LEDS, NUM_LEDS, 0); // Add our LED strip to the FastLED library
    // FastLED.setBrightness(100);
    FastLED.showColor(CRGB::Black);
    // makePretty();
    // set_max_power_indicator_LED(LED_BUILTIN);

    //    max_Brightness = calculate_max_brightness_for_power_mW(FastLED.leds(), NUM_LEDS, 255, MAX_POWER_LIMIT);
    //    g_DesiredBrightness = (u_int8_t) max_Brightness;
    // FastLED.setMaxPowerInMilliWatts(MAX_POWER_LIMIT);

    // / Light the builtin LED if we power throttle
    //    FastLED.setBrightness(g_Brightness);

    // OTA SETUP
    ArduinoOTA.setPort(OTAport);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(STR(SENSORNAME));

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
    randomSeed(analogRead(0));
    ArduinoOTA.setRebootOnSuccess(true);

    // SETUP mqtt
    // initEspMQTT(&client);

    while (!client.isConnected())
    {
        client.loop();
        delay(300);
        DBG("waiting for client to connect before initializing ArduinoOTA");
    }

    ArduinoOTA.begin();
#ifdef DEBUGENABLE
#ifdef USE_SERIAL
    Debug.setSerialEnabled(true);
#else
    Debug.setSerialEnabled(false);
#endif

    Debug.setResetCmdEnabled(true); // Enable the reset command
    Debug.showProfiler(true);       // To show profiler - time between messages of Debug
    Debug.begin((String *)sensorName, RemoteDebug::DEBUG);

#endif
    cluster.setMode(MODES::TREE);
    //     while (g_HasReceivedFirstMessage == false)
    //     {
    //         client.loop();
    //         ArduinoOTA.handle();
    //         delay(300);
    //         DBG("waiting for first message before starting loop");
    // #if DEBUGENABLE
    //         Debug.handle();
    // #endif
    //     }

    // FastLED.clear();
    // if (effect == LightEffect::SOLID)
    //     setupSolid();

    //    auto a = SolidParams();
    //    a.brightness=g_Brightness;
    //    a.blue=blue;
    //    a.red=red;
    //    a.green=green;
    //    solidParams=&a;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"

const int innerRingIdx[4] = {4, 5, 8, 9};
//  Layout of hex
//         11
//     12      10
//  13      9       3
//      8       4
//  7       5       2
//      6       1
//          0


// static const CRGB pallete[8] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, CRGB::Orange, CRGB::Purple, CRGB::White, CRGB::Black};
static uint16_t idx = 0;
static bool stateDiff()
{
    if (currentState.powerStateOn != desiredState.powerStateOn)
        return true;
    if (currentState.brightness != desiredState.brightness)
        return true;
}

void loop()
{
    static uint16_t pos = 0;

    ArduinoOTA.handle();
    client.loop();
    // DBG("loop");
    // DBG("{#%d}", FastLED.leds()->getAverageLight());
    // auto randHex = random(0, 14);
    // HEX_LIGHTS[randHex].setTopColors(CRGB::DarkOrange);
    currentState.powerStateOn = desiredState.powerStateOn;
    if (!currentState.powerStateOn)
    {
        FastLED.clear();
        FastLED.show();
    }
    else
    {
        cluster.loop();
    }
    // else
    // {
    //     static uint8_t idx = 0;
    //     static int8_t up = 1;
    //     static CRGB color = CRGB::White;
    //     static auto background_color = CRGB::Black;
    //     // const auto white = CRGB::

    //     EVERY_N_MILLIS(25)
    //     {
    //         // up = idx & 0b10010110;
    //         // up = -1 * branchlessSignum(idx - (15 * 7));
    //         // idx += up;
    //         // if (idx > (15 * 7))
    //         //     idx = 0;
    //         if (idx == 0)
    //         {
    //             up = 1;
    //             color = RainbowColors_p[random(0, 16)];
    //         } // RainbowColors_p[random(0, 16)]
    //         else if (idx >= 15 * 4)
    //         {
    //             up = -1;
    //             color = RainbowColors_p[random(0, 16)];
    //         }
    //         idx += up;

    //         for (auto i = 0; i < 14; i++)
    //         {
    //             HEX_LIGHTS[i].horizontalLightStep(idx - poo_offset[i], color, CRGB::Black);
    //             HEX_LIGHTS[i].syncAll();
    //         }
    //     }
    //     FastLED.show();
    // }
    // else
    // {
    //     for (auto i = 0; i < 14; i++)
    //     {
    //         HEX_LIGHTS[i].stepHexlights<50>();
    //     }

    //     FastLED.show();

    //     static bool flipFlop = true;
    //     if (flipFlop)
    //     {
    //         for (auto i = 0; i < 14; i++)
    //         {
    //             HEX_LIGHTS[i].setAll(CRGB::Black);
    //             HEX_LIGHTS[i].syncAll();
    //         }
    //         pos += increment;
    //         pos %= 360;
    //         DBG("pos %d", pos);
    //         for (auto i = 0; i < 14; i++)
    //         {
    //             HEX_LIGHTS[i].setLight(pos, CRGB::White);
    //         }
    //     }

    //     EVERY_N_SECONDS(10)
    //     {
    //         if (flipFlop)
    //         {

    //             // makePretty();
    //         }
    //         else
    //         {
    //             for (auto i = 0; i < 14; i++)
    //             {
    //                 HEX_LIGHTS[i].setAll(RainbowColors_p[random(0, 16)]);
    //             }
    //         }
    //         // flipFlop = !flipFlop;
    //     }
    // }

#if defined(DEBUG_ENABLE)

    Debug.handle();

#endif // DEBUG

#ifndef USE_SERIAL
#else
    // Or

    // debugHandle(); // Equal to SerialDebug
#endif
}

#pragma clang diagnostic pop