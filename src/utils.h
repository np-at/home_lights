#include <Arduino.h>
#include <FastLED.h>

#pragma once

// branchless signum, see https://stackoverflow.com/questions/1903954
static inline int branchlessSignum(int v)
{
    return (v != 0) | -(int)((unsigned int)((int)v) >> (sizeof(int) * CHAR_BIT - 1));
}

// Move current one step (per channel) toward target. Linear and exact; 255 calls to cross the full range.
inline void stepShift(CRGB &current, const CRGB &target)
{
    current.b += branchlessSignum(target.b - current.b);
    current.g += branchlessSignum(target.g - current.g);
    current.r += branchlessSignum(target.r - current.r);
}
