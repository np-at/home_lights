//
// Created by noahp on 6/11/22.
//

#ifndef HOMELED_REWRITE_STATE_H
#define HOMELED_REWRITE_STATE_H
#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "power_mgt.h"


#define MAX_POWER_LIMIT 22000
enum LightEffect {
    SOLID,
    TWINKLE,
    COMET,
    BREATHE,
};

class State {
public:
    LightEffect Effect{};
    boolean powerStateOn{};
    byte red{};
    byte green{};
    byte blue{};

    byte getBrightness();

    void setBrightness(byte newDesiredBrightness);

    byte speed = 128;

    int powerLimit = MAX_POWER_LIMIT;
    byte brightness{};
    byte desiredBrightness{};

private:

    byte getCalculatedBrightness(int desiredPowerLimit) const;
};


#endif //HOMELED_REWRITE_STATE_H
