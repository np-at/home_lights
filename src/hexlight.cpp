#include "hexlight.h"

  Hexlight::Hexlight(uint16_t startIdx, uint16_t endIdx, uint8_t orientation)
    {
        this->startIndex = startIdx;
        this->endIndex = endIdx;
        this->rotationOffset = orientation;
        this->setOrder(0, true);
    }

void Hexlight::setAll(CRGB color)
{
    for (auto i = 0; i < LIGHT_LED_COUNT; i++)
    {
        this->hexLights[i] = color;
        // FastLED.leds()[this->normalizedOrder[i]] = color;
    }
}



    void Hexlight::setOrder(uint8_t rotation, bool useRandomness = false)
    {
        
        auto rotationRandomness =  useRandomness ? random(0, 6) : 0 ;
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

    Hexlight::~Hexlight() {};
    
