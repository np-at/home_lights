


// FASTLED DEFINITIONS

//class State {
//public: LightEffect Effect;
//    boolean powerStateOn;
//    byte red;
//    byte green;
//    byte blue;
//    byte brightness;
//    byte speed;
//
//    byte desiredBrightness;
//protected:
//    byte getCalculatedBrightness();
//    byte brightness;
//};

#ifndef HOMELED_REWRITE_GLOBALS_H
#define HOMELED_REWRITE_GLOBALS_H
#pragma once
#include "State.h"

class LightEffectType {
public:
    virtual void DrawEffect(State *state) = 0;

    virtual boolean handleDiff_step(State *targetState, State *presentState) = 0;

};


#define NUM_LEDS 35 * 14
// led pin for esp32 is D4
#define LED_PIN 4
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
#define TIME_INCREMENT_SECONDS 10
#define STATE_CHANGE_BUFFER_THRESHOLD_SECONDS 30

#define INCREMENT_COUNT_THRESHOLD STATE
static byte red = 200;
static byte green = 200;
static byte blue = 200;
static boolean isTransitioning = false;
// byte brightness = 64;




#ifdef DEBUGENABLE
#define DBG(fmt, ...) rdebugA(fmt, ##__VA_ARGS__)
#else

#define DBG(fmt, ...) #fmt // fmt
#endif
// Taking 128 to be the "normal" rate, this is user controlled
uint16_t g_TransitionDelay = 128;

// This will be modified by the different configs to interpret the user controlled g_TransitionDelay into the effective rate
float g_DelayMultiplier = 1.00;

// the calculated delay value, derived from g_DelayMultiplier * g_TransitionDelay
// call refreshDelay when either parent changes
uint16_t g_DelayValue = 120;

CRGB g_LEDS[NUM_LEDS];

// Runtime globals

static const int g_c_INCREMENT_COUNT_THRESHOLD = (STATE_CHANGE_BUFFER_THRESHOLD_SECONDS / TIME_INCREMENT_SECONDS);
//static uint8_t g_Brightness = 200;  // 0-255 LED brightness scale
//static uint8_t g_DesiredBrightness = 200;
//static const int g_PowerLimit = 22000; // 22Watt Power Limit for a 300LEDS strip
static State desiredState;
static State currentState;
//float max_Brightness = 255;


// number of TIME_INCREMENT_SECONDS since state was last changed
u_long g_timeIncrementSinceStateChange = 0;


bool g_HasReceivedFirstMessage = true;


// DEBUG STUFF
#ifdef DEBUGENABLE

#include "RemoteDebug.h"

static RemoteDebug Debug;
#else
#ifndef DEBUG_I
#define DEBUG_I(...)
#endif

#endif


#define ST(A) #A
#define STR(A) ST(A)


// String Constants
static const char *twinkle = "twinkle";
static const char *solid = "solid";
static const char *rainbow = "rainbow";
static const char *comet = "comet";
static const char *breathe = "breathe";
static const char *wave = "wave";

static const char *c_STR_Effect = "effect";
static const char *c_STR_State = "state";
static const char *transition = "transition";
static const char *cs_Brightness = "brightness";
static const char *c_STR_Color = "color";


const char *lightEffectToString(LightEffect &lightEffect) {
    switch (lightEffect) {
        case LightEffect::SOLID:
            return solid;
        // case LightEffect::COMET:
        //     return comet;
        // case LightEffect::TWINKLE:
        //     return twinkle;
        // case LightEffect::BREATHE:
        //     return breathe;
        case LightEffect::RAINBOW:
            return "rainbow";
        default:
            return nullptr;
    }
}

static LightEffect effect = LightEffect::SOLID;

void refreshDelay() {
    g_DelayValue = (uint16_t) ((float) g_TransitionDelay * g_DelayMultiplier);
}

void SetupGlobalVars() {
    desiredState = State();
    g_HasReceivedFirstMessage = false;

}
#endif //HOMELED_REWRITE_GLOBALS_H