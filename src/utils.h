 #include <Arduino.h>
 #include <FastLED.h>

#pragma once

// branchless signum
// see https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
/**
 * Returns the sign of the given integer value, as an integer.
 * This implementation is branchless, meaning it avoids branching instructions
 * and is therefore faster than a traditional implementation using if/else statements.
 *
 * @param v The integer value to determine the sign of.
 * @return An integer representing the sign of the input value: -1 if v < 0, 0 if v == 0, and 1 if v > 0.
 */
static inline int branchlessSignum(int v)
{
    return (v != 0) | -(int)((unsigned int)((int)v) >> (sizeof(int) * CHAR_BIT - 1));
}



 template <uint8_t SHIFT>
    inline void stepShift(CRGB &current, CRGB &target)
    {
        auto blueDiff = target.b - current.b;
        // current.b += (branchlessSignum(blueDiff) * SHIFT);
        // current.b += (blueDiff / (SHIFT));
        current.b = scale8(current.b, (blueDiff / SHIFT) + 1);
        auto greenDiff = target.green - current.green;
        // current.green += (branchlessSignum(greenDiff) * SHIFT);
        // current.green += (greenDiff / SHIFT);
        current.green = scale8(current.green, (greenDiff / SHIFT)+1);
        auto redDiff = target.r - current.r;
        // current.red += (branchlessSignum(redDiff) * SHIFT);
        // current.red += (redDiff / (SHIFT));
        current.red = scale8(current.red,( redDiff / SHIFT)+1);
    }
    
    inline void stepShift(CRGB &current, CRGB &target)
    {
        auto blueDiff = target.b - current.b;
        current.b += (branchlessSignum(blueDiff));
        auto greenDiff = target.green - current.green;
        current.green += (branchlessSignum(greenDiff));
        auto redDiff = target.r - current.r;
        current.red += (branchlessSignum(redDiff));
    }
    