#ifndef HOMELED_REWRITE_STATE_H
#define HOMELED_REWRITE_STATE_H
#pragma once
#include <Arduino.h>

// What MQTT last asked for. Power limiting is FastLED's job (setMaxPowerInVoltsAndMilliamps), not ours.
struct State
{
    bool powerStateOn = false;
    uint8_t red = 144, green = 144, blue = 144;
    uint8_t brightness = 255;
};

#endif //HOMELED_REWRITE_STATE_H
