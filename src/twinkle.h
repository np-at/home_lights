//+--------------------------------------------------------------------------
//
// NightDriver - (c) 2020 Dave Plummer.  All Rights Reserved.
//
// File:        
//
// Description:
//
//   
//
// History:     Sep-15-2020     davepl      Created
//
//---------------------------------------------------------------------------

#include <Arduino.h>

#define FASTLED_INTERNAL

#include <FastLED.h>

#define NUM_COLORS 5
static const CRGB TwinkleColors[NUM_COLORS] = // NOLINT(cert-err58-cpp)
        {
                CRGB::Red,
                CRGB::Blue,
                CRGB::Purple,
                CRGB::Green,
                CRGB::Yellow
        };

__attribute__((unused)) void setupTwinkle() {
    g_DelayMultiplier = ((float) 1 / (float) 10);
    refreshDelay();
}

// Doesn't release control, BAD
__attribute__((unused)) void DrawTwinkle() {
    FastLED.clear(false);

    for (int i = 0; i < NUM_LEDS / 4; i++) {
        g_LEDS[random(NUM_LEDS)] = TwinkleColors[random(0, NUM_COLORS)];
        FastLED.show(currentState.brightness);
        delay(200);
    }
}

// Releases control back to main loop in between iterations, much better!
__attribute__((unused)) void DrawTwinkle2() {
    static int passCount = 0;
    if (passCount++ == NUM_LEDS / 4) {
        passCount = 0;
        FastLED.clear(false);
    }
    g_LEDS[random(NUM_LEDS)] = TwinkleColors[random(0, NUM_COLORS)];
}

__attribute__((unused)) void DrawTwinkle3() {
    const int fadeAmt = 64;
    // Randomly fade the LEDs
    g_LEDS[random(NUM_LEDS)] = TwinkleColors[random(0, NUM_COLORS)];
    for (int j = 0; j < FastLED.size(); j++) {
        if (random(10) > 5)
            FastLED.leds()[j] = FastLED.leds()[j].fadeToBlackBy(fadeAmt);

    }
}


class TwinkleLightEffect : LightEffectType {
protected:
    static const int fadeAmt = 64;
public:
    boolean handleDiff_step(State *targetState, State *presentState) final;
    void DrawEffect(State *state) final;
};

boolean TwinkleLightEffect::handleDiff_step(State *targetState, State *presentState) {

    auto changeCount = 0;
    g_DelayMultiplier = 0.4; // (float) 1 / (float) 10;
    refreshDelay();
    
    if (targetState->powerStateOn != presentState->powerStateOn) {
        if (presentState->brightness > 0) {
            presentState->brightness--;
            changeCount++;
        }
    }
    if (targetState->speed != presentState->speed) {
        presentState->speed > targetState->speed ? presentState->speed-- : presentState->speed++;
        g_TransitionDelay = presentState->speed;
        refreshDelay();
        changeCount++;
    }


    return (changeCount > 0);

}

void TwinkleLightEffect::DrawEffect(State *state) {
    // Randomly fade the LEDs
    g_LEDS[random(NUM_LEDS)] = TwinkleColors[random(0, NUM_COLORS)];
    for (int j = 0; j < FastLED.size(); j++) {
        if (random(10) > 5)
            FastLED.leds()[j] = FastLED.leds()[j].fadeToBlackBy(fadeAmt);
    }
}

