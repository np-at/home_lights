#ifndef HOMELED_REWRITE_GLOBALS_H
#define HOMELED_REWRITE_GLOBALS_H
#pragma once
#include "State.h"

#define NUM_LEDS 35 * 14
// led pin for esp32 is D4
#define LED_PIN 4
#define CHIPSET WS2812B
#define COLOR_ORDER GRB

// Power cap applied by FastLED on every show(). Supply is 5V/15A; cap at 80% of that.
// Override with -DPSU_MA=... if the supply changes.
#ifndef PSU_MA
#define PSU_MA 12000
#endif

static CRGB g_LEDS[NUM_LEDS];

static State desiredState;
static State currentState;

// DEBUG STUFF
#ifdef DEBUGENABLE
#include <RemoteDebug.h>
static RemoteDebug Debug;
#endif

#define ST(A) #A
#define STR(A) ST(A)

// MQTT JSON keys
static const char *c_STR_Effect = "effect";
static const char *c_STR_State = "state";
static const char *cs_Speed = "speed";
static const char *cs_Brightness = "brightness";
static const char *c_STR_Color = "color";

#endif //HOMELED_REWRITE_GLOBALS_H
