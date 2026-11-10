#include <Arduino.h>
#include <FastLED.h>
#ifndef HEXLIGHT_H
#define HEXLIGHT_H
#include "utils.h"

#define LIGHT_LED_COUNT 35

// One 35-LED panel. hexLights[] is the canvas in logical order (index 0 = bottom centre, counterclockwise);
// syncAll() copies it to the physical LEDs, undoing the panel's mounting rotation.
class Hexlight
{
    uint16_t normalizedOrder[LIGHT_LED_COUNT]{}; // logical index -> physical LED index

public:
    CRGB hexLights[LIGHT_LED_COUNT]{};

    // orientation: sides the panel is rotated counterclockwise from "input wire at the bottom"
    Hexlight(uint16_t startIdx, uint8_t orientation);
    void setAll(CRGB color);
    void syncAll();
    // angle in degrees from LED 0; any int, negatives wrap
    void setLight(int angle, CRGB color);
    // horizontal line at height pos (0 = bottom, 15 = top), mirrored on both sides
    void waveLightStep(uint8_t pos, CRGB color, uint8_t rot = 0);
};
#endif
