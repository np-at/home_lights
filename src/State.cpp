//
// Created by noahp on 6/11/22.
//

#include <Arduino.h>
#include "State.h"
#include "FastLED.h"
#include "power_mgt.h"


byte State::getCalculatedBrightness(int desiredPowerLimit) const {
    auto mb = (float) calculate_max_brightness_for_power_mW(255, desiredPowerLimit);
    return uint8_t(((float) desiredBrightness / (float) 255) * mb);
}

byte State::getBrightness() {
    if (!brightness) {
        brightness = getCalculatedBrightness(powerLimit);
    }
    return brightness;
}

void State::setBrightness(byte newDesiredBrightness) {
    desiredBrightness = newDesiredBrightness;
    brightness = getCalculatedBrightness(powerLimit);

}
