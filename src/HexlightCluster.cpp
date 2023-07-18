#include "HexlightCluster.h"

HexlightCluster::HexlightCluster()
{
}

HexlightCluster::~HexlightCluster()
{
}

void HexlightCluster::setMode(MODES mode)
{
    this->mode = mode;
    // blank all lights
    // (this->HEX_LIGHTS)->setAll(CRGB::Black);
}

const Hexlight *HexlightCluster::getHexlight(uint16_t index)
{
    return HEX_LIGHTS[index];
}
inline bool HexlightCluster::horizontal_animation()
{

    static uint8_t idx = 0;
    static int8_t up = 1;
    static CRGB color = CRGB::White;
    static auto background_color = CRGB::Black;
    EVERY_N_MILLIS(25)
    {

        // up = idx & 0b10010110;
        // up = -1 * branchlessSignum(idx - (15 * 7));
        // idx += up;
        // if (idx > (15 * 7))

        //     idx = 0;
        if (idx == 0)
        {
            up = 1;
            color = RainbowColors_p[random(0, 16)];
        } // RainbowColors_p[random(0, 16)]
        else if (idx >= 15 * 4)
        {
            up = -1;
            color = RainbowColors_p[random(0, 16)];
        }
        idx += up;

        for (auto i = 0; i < 14; i++)
        {
            HEX_LIGHTS[i]->horizontalLightStep(idx - poo_offset[i], color, CRGB::Black);
            HEX_LIGHTS[i]->syncAll();
        }
    };
    return false;
}
//  Layout of hex cluster
//         11
//     12      10
//  13      9       3
//      8       4
//  7       5       2
//      6       1
//          0
void HexlightCluster::makePretty()
{
    // void makePretty()
    // {
    // setInnerRing(CRGB::Red);
    // setOuterRing(CRGB::Blue);
    for (uint8_t i = 0; i < 14; i++)
    {
        HEX_LIGHTS[i]->setOrder(0, true);
        // HEX_LIGHTS[i].applyNormalizedColorMapping(pallete);
        HEX_LIGHTS[i]->setTopColors(CRGB::DarkOrange);
        HEX_LIGHTS[i]->setTopLeftColors(CRGB::Black);
        HEX_LIGHTS[i]->setTopRightColors(CRGB::Black);
        HEX_LIGHTS[i]->setBottomColors(CRGB::BlueViolet);
        HEX_LIGHTS[i]->setBottomLeftColors(CRGB::Green);
        HEX_LIGHTS[i]->setBottomRightColors(CRGB::Green);
    }
    // }
}
inline bool HexlightCluster::tree_animation()
{
    // for (auto i = 0; i < 14; i++)
    // {
    //     HEX_LIGHTS[i]->stepHexlights<50>();
    // }

    // EVERY_N_SECONDS(10)
    // {

    //     for (auto i = 0; i < 14; i++)
    //     {
    //         HEX_LIGHTS[i]->setAll(RainbowColors_p[random(0, 16)]);
    //     }
    // }
    for (auto i = 0; i < 14; i++)
    {
        HEX_LIGHTS[i]->stepHexlights<50>();
    }

    FastLED.show();

    static bool flipFlop = true;

    EVERY_N_SECONDS(10)
    {
        if (flipFlop)
        {

            this->makePretty();
        }
        else
        {
            for (auto i = 0; i < 14; i++)
            {
                HEX_LIGHTS[i]->setAll(RainbowColors_p[random(0, 16)]);
            }
        }
        flipFlop = !flipFlop;
    }
    return false;
}

bool HexlightCluster::solid_animation()
{
    for (auto i = 0; i < 14; i++)
    {
        HEX_LIGHTS[i]->setAll(this->color);
    }
    return false;
}

bool HexlightCluster::loop()
{
    // static auto flipflop = false;

    // EVERY_N_BSECONDS(3)
    // {
    //     if (flipflop)
    //         FastLED.leds()[42].setRGB(255, 0, 0);
    //     else
    //         FastLED.leds()[42].setRGB(0, 0, 255);
    // }
    FastLED.show();
    switch (this->mode)
    {
    case NONE:
        break;
    case SOLID_L:
        return this->solid_animation();
    case HORIZONTAL:
        return this->horizontal_animation();
        break;
    case TREE:
        return this->tree_animation();
        break;

    default:
        throw "Invalid mode";
        break;
    }
    // FastLED.show();
    return false;
}
