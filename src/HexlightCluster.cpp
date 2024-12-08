#include "HexlightCluster.h"
#include <globals.h>
HexlightCluster::HexlightCluster()
= default;

HexlightCluster::~HexlightCluster()
= default;

void HexlightCluster::setMode(MODES newMode) {
    this->mode = newMode;
    // blank all lights
    // (this->HEX_LIGHTS)->setAll(CRGB::Black);
}

const Hexlight *HexlightCluster::getHexlight(uint16_t index) {
    return HEX_LIGHTS[index];
}

inline bool HexlightCluster::horizontal_animation() {

    static uint8_t idx = 0;
    static int8_t up = 1;
    static auto background_color = CRGB::Black;
    EVERY_N_MILLIS(25)
    {

        // up = idx & 0b10010110;
        // up = -1 * branchlessSignum(idx - (15 * 7));
        // idx += up;
        // if (idx > (15 * 7))

        //     idx = 0;
        if (idx == 0) {
            up = 1;
            color = RainbowColors_p[random(0, 16)];
        } // RainbowColors_p[random(0, 16)]
        else if (idx >= 15 * 4) {
            up = -1;
            color = RainbowColors_p[random(0, 16)];
        }
        idx += up;

        for (auto i = 0; i < HEXLIGHT_CLUSTER_SIZE; i++) {
            HEX_LIGHTS[i]->horizontalLightStep(idx - poo_offset[i], this->color, CRGB::Black);
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
inline void HexlightCluster::makePretty() {
    // void makePretty()
    // {
    // setInnerRing(CRGB::Red);
    // setOuterRing(CRGB::Blue);
    for (auto &i: HEX_LIGHTS) {
        i->setOrder(0, true);
        // HEX_LIGHTS[i].applyNormalizedColorMapping(pallete);
        i->setTopColors(CRGB::DarkOrange);
        i->setTopLeftColors(CRGB::Black);
        i->setTopRightColors(CRGB::Black);
        i->setBottomColors(CRGB::BlueViolet);
        i->setBottomLeftColors(CRGB::Green);
        i->setBottomRightColors(CRGB::Green);
    }
    // }
}

inline bool HexlightCluster::tree_animation() {
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
    for (auto &i: HEX_LIGHTS) {
        i->stepHexlights<50>();
    }

    FastLED.show();

    static bool flipFlop = true;

    EVERY_N_SECONDS(10)
    {
        if (flipFlop) {

            this->makePretty();
        } else {
            for (auto &i: HEX_LIGHTS) {
                i->setAll(RainbowColors_p[random(0, 16)]);
            }
        }
        flipFlop = !flipFlop;
    }
    return false;
}

//bool HexlightCluster::twinkle_animation() {
//    static uint8_t idx = 0;
//    static uint8_t animation_states[HEXLIGHT_CLUSTER_SIZE] = {};
//    for (auto i = 0; i < HEXLIGHT_CLUSTER_SIZE; i++) {
//        if (animation_states[i] > 0) {
//            HEX_LIGHTS[i]->shift(CRGB::Black);
//            HEX_LIGHTS[i]->setLight(random(0, 360), RainbowColors_p[random(0, 16)]);
//            animation_states[i]--;
//        } else if (random(0, 100) > 90) {
//            HEX_LIGHTS[i]->setAll(RainbowColors_p[random(0, 16)]);
//            animation_states[i] = random(0, 10);
//        }
//    }
//}

inline bool HexlightCluster::solid_animation() const {
    for (auto &i: HEX_LIGHTS) {
        i->setAll(this->color);
        i->syncAll();
    }

    return false;
}

inline bool HexlightCluster::twinkle_animation() {
    static uint8_t animation_states[HEXLIGHT_CLUSTER_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    for (auto h:HEX_LIGHTS) {
        h->shift(CRGB::Black);
    }
    EVERY_N_MILLIS(300) {
        for (auto i = 0; i < HEXLIGHT_CLUSTER_SIZE; i++) {
            if (animation_states[i] > 0) {
                rdebugAln("animation_states[%d] = %d", i, animation_states[i]);
                HEX_LIGHTS[i]->setLight(random(0, 360), RainbowColors_p[random(0, 16)]);
                animation_states[i]--;
            } else if (random(0, 100) > 90) {
                HEX_LIGHTS[i]->setAll(RainbowColors_p[random(0, 16)]);
                animation_states[i] = random(0, 10);
            }
        }
    }

    return false;
}

inline bool HexlightCluster::sweep_animation(bool clockwise = false) const {

    const uint16_t sweep_speed = 10;
    static uint16_t angle = 0;
    CRGB backgroundColor = CRGB::Black;
    for (auto &i: HEX_LIGHTS) {
        i->shift(CRGB::Black);
    }

    EVERY_N_MILLIS(500)
    {

        // angle += clockwise ? sweep_speed : -sweep_speed;
        angle += sweep_speed;
        angle %= 360;
        // angle of 0 is 12 o'clock
        for (uint8_t i = 0; i < HEXLIGHT_CLUSTER_SIZE; i++) {
            // for (auto j = 0; j < LIGHT_LED_COUNT; j++)
            // {
            //     stepShift<5>(HEX_LIGHTS[i]->hexLights[j], backgroundColor);

            //     // this->hexLights[i].fadeToBlackBy(20);
            // }
            HEX_LIGHTS[i]->waveLightStep((angle / 360) + poo_offset[i], this->color, 1);
            HEX_LIGHTS[i]->syncAll();
        }
    }

    // HEX_LIGHTS[9]->setLight(angle, this->color);
    // HEX_LIGHTS[9]->setLight(angle + 180, this->color);

    // HEX_LIGHTS[11]->setLight(angle, this->color);
    // HEX_LIGHTS[11]->setLight(angle + 180, this->color);

    // angle of 90 is 3 o'clock
    // angle of 180 is 6 o'clock
    // angle of 270 is 9 o'clock
    // angle of 360 is 12 o'clock
    return false;
}

bool HexlightCluster::loop() {
    // static auto flipflop = false;

    // EVERY_N_BSECONDS(3)
    // {
    //     if (flipflop)
    //         FastLED.leds()[42].setRGB(255, 0, 0);
    //     else
    //         FastLED.leds()[42].setRGB(0, 0, 255);
    // }
    FastLED.show();
    switch (this->mode) {
        case NONE:
            break;
        case SOLID_L:
            return this->solid_animation();
        case HORIZONTAL:
            return this->horizontal_animation();
        case TREE:
            return this->tree_animation();
        case SWEEP:
            return this->sweep_animation();
        case TWINKLE:
            return this->twinkle_animation();
        default:
            // DBG("invalid mode %d", this->mode);
            break;
    }
    // FastLED.show();
    return false;
}
