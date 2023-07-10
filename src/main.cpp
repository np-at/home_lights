
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

#include "hexlight.h"
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
static Hexlight HEX_LIGHTS[14] = {
    Hexlight(0, 34, 0),
    Hexlight(35, 69, 1),
    Hexlight(70, 104, 1),
    Hexlight(105, 139, 0),
    Hexlight(140, 174, 4),
    Hexlight(175, 209, 4),
    Hexlight(210, 244, 4),
    Hexlight(245, 279, 5),
    Hexlight(280, 314, 1),
    Hexlight(315, 349, 1),
    Hexlight(uint16_t(350), uint16_t(384), uint8_t(1)),
    Hexlight(uint16_t(385), uint16_t(419), uint8_t(5)),
    Hexlight((uint16_t)420, uint16_t(454), uint8_t(4)),
    Hexlight(uint16_t(455), uint16_t(489), uint8_t(4)),

};
void makePretty()
{
    // setInnerRing(CRGB::Red);
    // setOuterRing(CRGB::Blue);
    for (uint8_t i = 0; i < 14; i++)
    {
        HEX_LIGHTS[i].setOrder(0, true);
        // HEX_LIGHTS[i].applyNormalizedColorMapping(pallete);
        HEX_LIGHTS[i].setTopColors(CRGB::DarkOrange);
        HEX_LIGHTS[i].setTopLeftColors(CRGB::Black);
        HEX_LIGHTS[i].setTopRightColors(CRGB::Black);
        HEX_LIGHTS[i].setBottomColors(CRGB::BlueViolet);
        HEX_LIGHTS[i].setBottomLeftColors(CRGB::Green);
        HEX_LIGHTS[i].setBottomRightColors(CRGB::Green);
    }
}
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
    currentState.Effect = LightEffect::COMET;
    currentState.powerLimit = MAX_POWER_LIMIT;
    currentState.brightness = 255;
    currentState.powerStateOn = true;
    desiredState = State();
    desiredState.blue = 144;
    desiredState.red = 144;
    desiredState.green = 144;

    desiredState.powerLimit = MAX_POWER_LIMIT;

    FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(g_LEDS, NUM_LEDS, 0); // Add our LED strip to the FastLED library
    FastLED.setBrightness(100);
    FastLED.showColor(CRGB::Black);
    makePretty();
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

void setInnerRing(CRGB color)
{
    HEX_LIGHTS[4].setBottomLeftColors(color);
    HEX_LIGHTS[4].setTopLeftColors(color);
    HEX_LIGHTS[5].setTopColors(color);
    HEX_LIGHTS[8].setTopRightColors(color);
    HEX_LIGHTS[8].setBottomRightColors(color);
    HEX_LIGHTS[9].setBottomColors(color);
}
void setOuterRing(CRGB color)
{
    HEX_LIGHTS[4].setBottomColors(color);
    HEX_LIGHTS[4].setBottomRightColors(color);
    HEX_LIGHTS[4].setTopRightColors(color);
    HEX_LIGHTS[4].setTopColors(color);

    HEX_LIGHTS[5].setBottomColors(color);
    HEX_LIGHTS[5].setBottomRightColors(color);
    HEX_LIGHTS[5].setBottomLeftColors(color);

    HEX_LIGHTS[8].setBottomColors(color);
    HEX_LIGHTS[8].setBottomColors(color);
    HEX_LIGHTS[8].setBottomLeftColors(color);
    HEX_LIGHTS[8].setTopLeftColors(color);
    HEX_LIGHTS[8].setTopColors(color);

    HEX_LIGHTS[9].setTopLeftColors(color);
    HEX_LIGHTS[9].setTopColors(color);
    HEX_LIGHTS[9].setTopRightColors(color);
}


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
        delay(300);
    }
    else
    {
        for (auto i = 0; i < 14; i++)
        {
            HEX_LIGHTS[i].stepHexlights<1000>();
        }

    
        FastLED.show();

        static bool flipFlop = false;
        EVERY_N_SECONDS(10)
        {
            if (flipFlop)
            {
              
                makePretty();

            }
            else
            {
                for (auto i = 0; i < 14; i++)
                {
                    HEX_LIGHTS[i].setAll(RainbowColors_p[random(0, 16)]);
                }

            }
            flipFlop = !flipFlop;
        }
    }

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