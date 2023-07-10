

//struct SolidParams {
//    byte red;
//    byte green;
//    byte blue;
//    byte brightness;
//
//};
//static SolidParams *solidParams;
//
#ifndef HOMELED_REWRITE_SOLID_H
#define HOMELED_REWRITE_SOLID_H
#pragma once

class SolidEffect : public LightEffectType {
protected:
    static const byte changeIncrement = 1;
public:
    boolean handleDiff_step(State *targetState, State *presentState) final;

    void DrawEffect(State *state) final;
};

void SolidEffect::DrawEffect(State *state) {

    for (int i = 0; i < NUM_LEDS - 1; i++) {
        g_LEDS[i].setRGB(state->red, state->green, state->blue);
    }
}

bool SolidEffect::handleDiff_step(State *targetState, State *presentState) {
    auto changeCount = 0;

    // power state handling can shortcircuit function execution so run first
    if (presentState->powerStateOn != targetState->powerStateOn) {
        if (targetState->powerStateOn) {
            presentState->powerStateOn = true;
            presentState->brightness = 0;
            changeCount++;
        } else {
            if (presentState->brightness > 1) {
                presentState->brightness--;
                if (presentState->brightness < 2)
                {
                    presentState->powerStateOn = false;
                    return false;
                }
                changeCount++;
                return true;
            } else {
                // short circuit
                presentState->powerStateOn = false;
                return false;
            }

        }
    }

    const auto blueDiff = (targetState->blue - presentState->blue);
    if (blueDiff != 0) {
        targetState->blue > presentState->blue ? presentState->blue += changeIncrement
                                               : presentState->blue -= changeIncrement;
        changeCount++;
    }
    const auto redDiff = (targetState->red - presentState->red);
    if (redDiff != 0) {
        DBG("reddiff: %i \n", redDiff);
        targetState->red > presentState->red ? presentState->red += changeIncrement
                                             : presentState->red -= changeIncrement;
        changeCount++;
    }
    const auto greenDiff = targetState->green - presentState->green;
    if (greenDiff != 0) {
        targetState->green > presentState->green ? presentState->green += changeIncrement
                                                 : presentState->green -= changeIncrement;
        changeCount++;
    }
    const auto brightDiff = targetState->brightness - presentState->brightness;
    if (brightDiff != 0) {
        targetState->brightness > presentState->brightness ? presentState->brightness += changeIncrement
                                                           : presentState->brightness -= changeIncrement;
        changeCount++;
    }
    if (targetState->desiredBrightness != presentState->desiredBrightness) {
        presentState->desiredBrightness = targetState->desiredBrightness;
    }

#ifdef DEBUGENABLE
    Debug.printf("diffstuff:\n brightness: %i vs %i", presentState->brightness, targetState->brightness);
#endif
    DBG("handling diff: b: %i \n", presentState->brightness);

    return (changeCount > 0);
}

void setupSolid(bool refresh = false) {
    for (int i = 0; i < NUM_LEDS - 1; i++) {
        g_LEDS[i].setRGB(red, green, blue);
    }
    if (!refresh) {
        g_DelayMultiplier = (float) 2;
    }
    refreshDelay();

}

#endif //HOMELED_REWRITE_SOLID_H



//static const byte changeIncrement = 1;
//
//void SetSolid(byte redValue, byte greenValue, byte blueValue, byte brightnessValue) {
//    solidParams->brightness = brightnessValue;
//    solidParams->red = redValue;
//    solidParams->blue = blueValue;
//    solidParams->green = greenValue;
//
//    isTransitioning = true;
//    effect = LightEffect::SOLID;
//}


//bool changeSolidParams_Step(State *targetSolidParams) {
//    auto changeCount = 0;
//    const auto blueDiff = (targetSolidParams->blue - blue);
//    if (blueDiff != 0) {
//        targetSolidParams->blue > blue ? blue += changeIncrement : blue -= changeIncrement;
//        changeCount++;
//    }
//    const auto redDiff = (targetSolidParams->red - red);
//    if (redDiff != 0) {
////#ifdef DEBUGENABLE
////        Debug.printf("reddiff: %i \n", redDiff);
////#endif
//        targetSolidParams->red > red ? red += changeIncrement : red -= changeIncrement;
//        changeCount++;
//    }
//    const auto greenDiff = targetSolidParams->green - green;
//    if (greenDiff != 0) {
//        targetSolidParams->green > green ? green += changeIncrement : green -= changeIncrement;
//        changeCount++;
//    }
//    const auto brightDiff = targetSolidParams->brightness - g_Brightness;
//    if (brightDiff != 0) {
//        targetSolidParams->brightness > g_Brightness ? g_Brightness += changeIncrement
//                                                     : g_Brightness -= changeIncrement;
//        changeCount++;
//    }
//#ifdef DEBUGENABLE
//    //    Debug.printf("change count is %i  \n\n", changeCount);
//        Debug.printf("diffs; r: %i, b: %i, g: %i, bright: %i \n", redDiff, blueDiff, greenDiff, brightDiff);
//#endif
//    return (changeCount > 0);
//
//}
//
//void DrawSolid() {
//    if (isTransitioning) {
//        DBG("is in transitioning");
//        isTransitioning = changeSolidParams_Step(solidParams);
//        for (int i = 0; i < NUM_LEDS - 1; i++) {
//            g_LEDS[i].setRGB(red, green, blue);
//        }
//    }
//}
