#include "hexlight.h"
#include "utils.h"

Hexlight::Hexlight(uint16_t startIdx, uint16_t endIdx, uint8_t orientation)
{
    this->startIndex = startIdx;
    this->endIndex = endIdx;
    this->rotationOffset = orientation;
    this->setOrder(0, false);
}

void Hexlight::setAll(CRGB color)
{
    for (auto i = 0; i < LIGHT_LED_COUNT; i++)
    {
        this->hexLights[i] = color;
        // FastLED.leds()[this->normalizedOrder[i]] = color;
    }
}
void Hexlight::syncAll()
{
    for (auto i = 0; i < LIGHT_LED_COUNT; i++)
    {
        FastLED.leds()[this->normalizedOrder[i]] = this->hexLights[i];
    }
}

void Hexlight::setLight(uint16_t angle, CRGB color, uint8_t brightness)
{
    // normalize angle
    auto normalizedAngle = angle % 360;
    auto ledIndex = (normalizedAngle * LIGHT_LED_COUNT) / 360;
    this->hexLights[ledIndex] = color;
    this->hexLights[ledIndex].fadeToBlackBy(255 - brightness);
    this->syncLightIdxHard(ledIndex);
}

void Hexlight::setLight(uint16_t a, CRGB color)
{
    // normalize angle
    auto normalizedAngle = a % 360;
    auto ledIndex = (normalizedAngle * LIGHT_LED_COUNT) / 360;
    this->hexLights[ledIndex] = color;
    this->syncLightIdxHard(ledIndex);
}

void Hexlight::setOrder(uint8_t rotation, bool useRandomness = false)
{
    auto rotationRandomness = useRandomness ? random(0, 6) : 0;
    switch ((this->rotationOffset + rotation + rotationRandomness) % 6)
    {
    case 0:
        for (int i = 0; i < LIGHT_LED_COUNT; i++)
        {
            this->normalizedOrder[i] = i + this->startIndex;
        }
        break;
    case 1:
        for (int i = 0; i < LIGHT_LED_COUNT; i++)
        {
            this->normalizedOrder[i] = ((i + 5) % LIGHT_LED_COUNT) + this->startIndex;
        }
        break;
    case 2:
        for (int i = 0; i < LIGHT_LED_COUNT; i++)
        {
            this->normalizedOrder[i] = ((i + 11) % LIGHT_LED_COUNT) + this->startIndex;
        }
        break;
    case 3:
        for (int i = 0; i < LIGHT_LED_COUNT; i++)
        {
            this->normalizedOrder[i] = ((i + 17) % LIGHT_LED_COUNT) + this->startIndex;
        }
        break;
    case 4:
        for (int i = 0; i < LIGHT_LED_COUNT; i++)
        {
            this->normalizedOrder[i] = ((i + 23) % LIGHT_LED_COUNT) + this->startIndex;
        }
        break;
    case 5:
        for (int i = 0; i < LIGHT_LED_COUNT; i++)
        {
            this->normalizedOrder[i] = ((i + 29) % LIGHT_LED_COUNT) + this->startIndex;
        }
        break;
    default:
        break;
    }
}
void Hexlight::horizontalLightStep(uint8_t pos, CRGB color, CRGB backgroundColor)
{

    // static const CRGB color = CRGB::White;
    static const CRGB black = CRGB::Black;
    // pos++;
    // if (pos > 32)
    //     pos = 0;
     for (auto i = 0; i < LIGHT_LED_COUNT; i++)
    {
        stepShift<5>(this->hexLights[i], backgroundColor);

        // this->hexLights[i].fadeToBlackBy(20);
    }
    this->syncAll();
    switch (pos)
    {
    case 0:
        this->setLight(0, color);
        break;
    case 1:
        // this->setBottomColors(color);
        this->setLight(11, color);
        this->setLight(-11, color);
        break;
    case 2:
        // this->setBottomColors(black);
        this->setLight(22, color);
        this->setLight(-22, color);
        break;
    case 3:
        // this->setLight(22, black);
        // this->setLight(-22, black);
        this->setLight(33, color);
        this->setLight(-33, color);
        break;
    case 4:
        // this->setLight(-33, black);
        // this->setLight(33, black);
        this->setLight(44, color);
        this->setLight(-44, color);
        break;
    case 5:
        // this->setLight(44, black);
        // this->setLight(-44, black);
        this->setLight(55, color);
        this->setLight(-55, color);
        break;
    case 6:

        // this->setLight(55, black);
        // this->setLight(-55, black);
        this->setLight(66, color);
        this->setLight(-66, color);
        break;
    case 7:
        // this->setLight(66, black);
        // this->setLight(-66, black);

        this->setLight(77, color);
        this->setLight(-77, color);
        break;
    case 8:
        // this->setLight(77, black);
        // this->setLight(-77, black);
        this->setLight(88, color);
        this->setLight(-88, color);
        break;
    case 9:
        // this->setLight(88, black);
        // this->setLight(-88, black);
        this->setLight(99, color);
        this->setLight(-99, color);
        break;
    case 10:
        // this->setLight(99, black);
        // this->setLight(-99, black);
        this->setLight(110, color);
        this->setLight(-110, color);
        break;
    case 11:
        // this->setLight(110, black);
        // this->setLight(-110, black);
        this->setLight(121, color);
        this->setLight(-121, color);
        break;
    case 12:
        // this->setLight(121, black);
        // this->setLight(-121, black);
        this->setLight(132, color);
        this->setLight(-132, color);
        break;
    case 13:
        // this->setLight(132, black);
        // this->setLight(-132, black);
        this->setLight(143, color);
        this->setLight(-143, color);
        break;
    case 14:
        // this->setLight(143, black);
        // this->setLight(-143, black);
        this->setLight(154, color);
        this->setLight(-154, color);
        break;
    case 15:
        // this->setLight(154, black);
        // this->setLight(-154, black);
        // this->setTopColors(color);
        this->setLight(165, color);
        this->setLight(-165, color);
        break;
    default:
        // this->setAll(black);
        // this->syncAll();
        // pos = 0;
        break;
    }
   
}
void Hexlight::horizontalLightStep(uint8_t pos, CRGB color)
{
    this->horizontalLightStep(pos, color, CRGB::Black);
}
bool Hexlight::sychronizeHexlights()
{
    auto changed = false;
    for (int i = 0; i < LIGHT_LED_COUNT; i++)
    {
        if (FastLED.leds()[this->normalizedOrder[i]] != this->hexLights[i])
        {
            FastLED.leds()[this->normalizedOrder[i]] = this->hexLights[i];
            changed = true;
        }
    }
    return changed;
}

Hexlight::~Hexlight(){};