#include "hexlight.h"

Hexlight::Hexlight(uint16_t startIdx, uint8_t orientation)
{
    // one side = ~35/6 LEDs; the bottom side has 5, so the first step is 5
    static constexpr uint8_t SIDE_SHIFT[6] = {0, 5, 11, 17, 23, 29};
    for (int i = 0; i < LIGHT_LED_COUNT; i++)
        normalizedOrder[i] = (i + SIDE_SHIFT[orientation % 6]) % LIGHT_LED_COUNT + startIdx;
}

void Hexlight::setAll(CRGB color)
{
    fill_solid(hexLights, LIGHT_LED_COUNT, color);
}

void Hexlight::syncAll()
{
    for (int i = 0; i < LIGHT_LED_COUNT; i++)
        FastLED.leds()[normalizedOrder[i]] = hexLights[i];
}

void Hexlight::setLight(int angle, CRGB color)
{
    hexLights[((angle % 360) + 360) % 360 * LIGHT_LED_COUNT / 360] = color;
}

void Hexlight::waveLightStep(uint8_t pos, CRGB color, uint8_t rot)
{
    if (pos > 15)
        return;
    setLight(11 * pos + rot, color);
    setLight(rot - 11 * pos, color);
}
