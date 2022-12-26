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
// History:     Sep-28-2020     davepl      Created
//
//---------------------------------------------------------------------------

#include <Arduino.h>
#include <FastLED.h>

#define NUM_COMETS 3
struct COMET1 {
    unsigned int pos;
    signed char direction;
};



//void DrawComet() {
//    const byte fadeAmt = 64;
////    const int cometSize = 7;
//    const int deltaHue = 4;
//
//    static byte hue = HUE_RED;
////    static int iDirection = 1;
////    static int iDirection2 = 1;
////    static int iPos = 0;
////    static int iPos2 = 200;
////    static byte te[5];
//    hue += deltaHue;
//
//    for (auto &Comet: Comets) {
//        processComets(&Comet, &hue);
//    }
////    iPos += iDirection;
////    if (iPos == (FastLED.size() - cometSize) || iPos == 0)
////        iDirection *= -1;
////    for (int i = 0; i < cometSize; i++) {
////        FastLED.leds()[iPos + i].setHue(hue);
////    }
////
////    iPos2 += iDirection2;
////    if (iPos2 == (FastLED.size() - cometSize) || iPos2 == 0)
////        iDirection2 *= -1;
////    for (int i = 0; i < cometSize; i++) {
////        FastLED.leds()[iPos2 + i].setHue(hue);
////    }
//
//    // Randomly fade the LEDs
//    for (int j = 0; j < FastLED.size(); j++) {
//        if (random(10) > 6)
//            FastLED.leds()[j] = FastLED.leds()[j].fadeToBlackBy(fadeAmt);
//    }
//}
//
//void setupComet() {
//    g_DelayMultiplier = 0.35; // (float)30/(float)128;
//    refreshDelay();
//}

class CometLightEffect : LightEffectType {
public:
    CometLightEffect();

    void DrawEffect(State *state) final;

    boolean handleDiff_step(State *targetState, State *presentState) final;

protected:
    COMET1 Comets[NUM_COMETS] = {
            {5, 1},
            {150, 1},
            {175, -1}
    };
    const static int fadeAmt = 64;
    const static int deltaHue = 4;
    byte hue = HUE_RED;

    void processComets(COMET1 *curComet) const;
};

CometLightEffect::CometLightEffect() =default;
//{
//    hue = HUE_RED;

//    for (auto &Comet: Comets) {
//        Comet = {static_cast<unsigned int>(random(0, NUM_LEDS)),
//                 static_cast<signed char>(random8(0, 254) > 128 ? 1 : -1)};
//    }
//    Comets[0] = {5,1};
//    Comets[1] = {150, 1};
//    Comets[2] = {175,-1};
//k}

void CometLightEffect::processComets(COMET1 *curComet) const {
    const int cometSize = 7;

    unsigned int *pos = &curComet->pos;
    signed char *direction = &curComet->direction;

    if (*pos == (FastLED.size() - cometSize) || *pos == 0)
        *direction *= -1;
    *pos += *direction;

    for (int i = 0; i < cometSize; i++) {
        FastLED.leds()[*pos + i].setHue(hue);
    }
}

void CometLightEffect::DrawEffect(State *state) {
    hue += deltaHue;

    for (auto &Comet: Comets) {
        processComets(&Comet);
    }
//    iPos += iDirection;
//    if (iPos == (FastLED.size() - cometSize) || iPos == 0)
//        iDirection *= -1;
//    for (int i = 0; i < cometSize; i++) {
//        FastLED.leds()[iPos + i].setHue(hue);
//    }
//
//    iPos2 += iDirection2;
//    if (iPos2 == (FastLED.size() - cometSize) || iPos2 == 0)
//        iDirection2 *= -1;
//    for (int i = 0; i < cometSize; i++) {
//        FastLED.leds()[iPos2 + i].setHue(hue);
//    }

    // Randomly fade the LEDs
    for (int j = 0; j < FastLED.size(); j++) {
        if (random(10) > 6)
            FastLED.leds()[j] = FastLED.leds()[j].fadeToBlackBy(fadeAmt);
    }
}

boolean CometLightEffect::handleDiff_step(State *targetState, State *presentState) {
    g_DelayMultiplier = 0.35; // (float)30/(float)128;
    g_TransitionDelay = targetState->speed;
    refreshDelay();

    *presentState = *targetState;
    return false;
}

