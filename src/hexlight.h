#include <Arduino.h>
#include <FastLED.h>

#define LIGHT_LED_COUNT 35
static constexpr int BOTTOM_LEDS[5] = {0, 1, 2, 33, 34};
static constexpr int BOTTOM_RIGHT_LEDS[6] = {3, 4, 5, 6, 7, 8};
static constexpr int TOP_RIGHT_LEDS[6] = {9, 10, 11, 12, 13, 14};
static constexpr int TOP_LEDS[6] = {15, 16, 17, 18, 19, 20};
static constexpr int TOP_LEFT_LEDS[6] = {21, 22, 23, 24, 25, 26};
static constexpr int BOTTOM_LEFT_LEDS[6] = {27, 28, 29, 30, 31, 32};

// static const int ALL_LEDS[5+6+6+6+6+6] = {0}; // initialize to all zeros

// template <uint16_t Start,uint16_t End>
// class Hexlight
// {
//     private:
//     const uint16_t startIdx = Start;
//     const uint16_t endIdx = End;
//     static const int BOTTOM_LEDS[5] = {0, 1, 2, 33, 34};
// };

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

template <uint16_t TRANSITION_PERIOD_MILLIS>
static inline void diffColorStep(CRGB color, CRGB target)
{

    EVERY_N_MILLIS(TRANSITION_PERIOD_MILLIS / 255)
    {
        auto blueDiff = target.blue - color.blue;
        auto greenDiff = target.green - color.green;
        auto redDiff = target.r - color.r;
        color.blue += (branchlessSignum(blueDiff));
        color.green += (branchlessSignum(greenDiff) );
        color.red += (branchlessSignum(redDiff));
    };

    return;
}
/**
 * Converts a CRGB color to a string representation.
 *
 * @param color The CRGB color to convert.
 * @return A dynamically allocated char array containing the string representation of the color.
 *         The caller is responsible for freeing the memory allocated by this function.
 */
static inline char *crgbToString(CRGB color)
{
    char *buffer = new char[20];
    sprintf(buffer, "CRGB(%d,%d,%d)", color.r, color.g, color.b);
    return buffer;
}

class Hexlight
{
private:
    // represents the index of the first led in the hexagon
    // progression is counterclockwise

    uint16_t startIndex;
    uint16_t endIndex;
    // represents the number of increments that the hexagon is rotated counterclockwise,
    // with 0 being the default orientation (the input wire being at the bottommost point of the hexagon)
    // uint16_t normalizedStartIdx;
    // CRGB leds[LIGHT_LED_COUNT];

    // Represents the order of the LEDs after normalization, where the first LED is at the startIndex.
    // The order is determined by the orientation of the hexagon and is used to map colors to the LEDs.
    uint16_t normalizedOrder[LIGHT_LED_COUNT];
    CRGB hexLights[LIGHT_LED_COUNT];
    uint8_t rotationOffset;

    /**
     * Sets the order of the LEDs based on the given rotation.
     *
     * @param rotation The number of increments that the hexagon is rotated counterclockwise,
     *                 with 0 being the default orientation (the input wire being at the bottommost point of the hexagon).
     */
    // void setOrder(uint8_t rotation);

    // int normalizeIndex(uint16_t index)
    // {
    //     return (index + this->normalizedStartIdx) % LIGHT_LED_COUNT;
    // }

    //

public:
    /**
     * Updates the colors of the hexagon's LEDs to be closer to their target colors by incrementing or decrementing
     * each color channel (red, green, blue) by 1, based on the difference between the current
     * color and the target color.
     *
     * @return A boolean indicating whether any LED colors were changed during this step.
     */
template <uint16_t TRANSITION_PERIOD_MILLIS>
    bool stepHexlights()
    {
        auto changed = false;
        for (int i = 0; i < LIGHT_LED_COUNT; i++)
        {
            auto ledIdx = this->normalizedOrder[i];
            auto current = FastLED.leds()[ledIdx];
            auto target = this->hexLights[i];
            if (current != this->hexLights[i])
            {
               
                auto blueDiff = target.b - current.b;
                FastLED.leds()[ledIdx].blue += branchlessSignum(blueDiff);
                auto greenDiff = target.green - current.green;
                FastLED.leds()[ledIdx].green += branchlessSignum(greenDiff);
                auto redDiff = target.r - current.r;
                FastLED.leds()[ledIdx].red += branchlessSignum(redDiff);
                // this->diffColorStep(current, target);
                // DEBUG_I("new current: %s\n", crgbToString(current));

                // DEBUG_I("%s\n", crgbToString(&this->hexLights[i]));
                // DEBUG_I("%s\n", crgbToString(&diff));

                // FastLED.leds()[this->normalizedOrder[i]] = diff;
                changed = true;
            }
        }
        return changed;
    }
    /**
     * Sets the color of the top LEDs of the hexagon to the given color.
     *
     * @param color The color to set the LEDs to.
     */
    void setTopColors(CRGB color)
    {
        for (int i = 0; i < 6; i++)
        {
            this->hexLights[TOP_LEDS[i]] = color;
            // FastLED.leds()[this->normalizedOrder[TOP_LEDS[i]]] = color;
        }
    }
    // Sets the color of the top LEDs of the hexagon to the given colors.
    //
    // @param colors An array of CRGB pointers representing the colors to set the LEDs to.
    void setTopColors(CRGB *colors[6])
    {
        for (int i = 0; i < 6; i++)
        {
            this->hexLights[TOP_LEDS[i]] = *colors[i];
            // FastLED.leds()[this->normalizedOrder[TOP_LEDS[i]]] = *colors[i];
        }
    }
    /**
     * Sets the color of the bottom LEDs of the hexagon.
     *
     * @param color The color to set the LEDs to.
     */
    void setBottomColors(CRGB color)
    {
        for (int i = 0; i < 5; i++)
        {
            this->hexLights[BOTTOM_LEDS[i]] = color;
            // FastLED.leds()[this->normalizedOrder[BOTTOM_LEDS[i]]] = color;
        }
    }
    // Sets the color of the bottom LEDs of the hexagon to the given colors.
    //
    // @param colors An array of CRGB pointers representing the colors to set the LEDs to.
    void setBottomColors(CRGB *colors[5])
    {
        for (auto i = 0; i < 5; i++)
        {
            this->hexLights[BOTTOM_LEDS[i]] = *colors[i];
            // FastLED.leds()[this->normalizedOrder[BOTTOM_LEDS[i]]] = *colors[i];
        }
    }
    /**
     * Sets the color of the bottom right LEDs of the hexagon.
     *
     * @param color The color to set the LEDs to.
     */
    void setBottomRightColors(CRGB color)
    {
        for (int i = 0; i < 6; i++)
        {
            this->hexLights[BOTTOM_RIGHT_LEDS[i]] = color;
            // FastLED.leds()[this->normalizedOrder[BOTTOM_RIGHT_LEDS[i]]] = color;
        }
    }

    /**
     * Sets the color of the bottom right LEDs of the hexagon to the given colors.
     *
     * @param colors An array of CRGB pointers representing the colors to set the LEDs to.
     */
    void setBottomRightColors(CRGB *colors[6])
    {
        for (auto i = 0; i < 6; i++)
        {
            this->hexLights[BOTTOM_RIGHT_LEDS[i]] = *colors[i];
            // FastLED.leds()[this->normalizedOrder[BOTTOM_RIGHT_LEDS[i]]] = *colors[i];
        }
    }
    /**
     * Sets the color of the top right LEDs of the hexagon.
     *
     * @param color The color to set the LEDs to.
     */
    void setTopRightColors(CRGB color)
    {
        for (int i = 0; i < 6; i++)
        {
            this->hexLights[TOP_RIGHT_LEDS[i]] = color;
            // FastLED.leds()[this->normalizedOrder[TOP_RIGHT_LEDS[i]]] = color;
        }
    }
    /**
     * Sets the color of the top right LEDs of the hexagon to the given colors.
     *
     * @param colors An array of CRGB pointers representing the colors to set the LEDs to.
     */
    void setTopRightColors(CRGB *colors[6])
    {
        for (auto i = 0; i < 6; i++)
        {
            this->hexLights[TOP_RIGHT_LEDS[i]] = *colors[i];
            // FastLED.leds()[this->normalizedOrder[TOP_RIGHT_LEDS[i]]] = *colors[i];
        }
    }
    // Sets the color of the top left LEDs of the hexagon to the given color.
    //
    // @param color The color to set the LEDs to.
    void setTopLeftColors(CRGB color)
    {
        for (int i = 0; i < 6; i++)
        {
            this->hexLights[TOP_LEFT_LEDS[i]] = color;
            // FastLED.leds()[this->normalizedOrder[TOP_LEFT_LEDS[i]]] = color;
        }
    }
    /**
     * Sets the color of the top left LEDs of the hexagon to the given colors.
     *
     * @param colors An array of CRGB pointers representing the colors to set the LEDs to.
     */
    void setTopLeftColors(CRGB *colors[6])
    {
        for (auto i = 0; i < 6; i++)
        {
            this->hexLights[TOP_LEFT_LEDS[i]] = *colors[i];
            // FastLED.leds()[this->normalizedOrder[TOP_LEFT_LEDS[i]]] = *colors[i];
        }
    }
    void setBottomLeftColors(CRGB color)
    {
        for (int i = 0; i < 6; i++)
        {
            this->hexLights[BOTTOM_LEFT_LEDS[i]] = color;
            // FastLED.leds()[this->normalizedOrder[BOTTOM_LEFT_LEDS[i]]] = color;
        }
    }
    void setBottomLeftColors(CRGB *colors[6])
    {
        for (auto i = 0; i < 6; i++)
        {
            this->hexLights[BOTTOM_LEFT_LEDS[i]] = *colors[i];
            // FastLED.leds()[this->normalizedOrder[BOTTOM_LEFT_LEDS[i]]] = *colors[i];
        }
    }
    /**
     * Sets the color of all LEDs in the hexagon to the given color.
     *
     * @param color The color to set the LEDs to.
     */
    Hexlight(uint16_t startIdx, uint16_t endIdx, uint8_t orientation);
    void setAll(CRGB color);

    void setOrder(uint8_t rotation, bool useRandomness);

    bool sychronizeHexlights();

    ~Hexlight();

    // void setAll(CRGB color)
    // {
    //     for (int i = startIndex; i < endIndex; i++)
    //     {
    //         this->leds[i] = color;
    //     }
    // }
};

static Hexlight CHexlight[14] = {
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
