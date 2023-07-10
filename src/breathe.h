#ifndef HOMELED_REWRITE_BREATHE_H
#define HOMELED_REWRITE_BREATHE_H
#pragma once

class BreatheEffect : public LightEffectType
{
protected:
    static const byte changeIncrement = 1;
    static constexpr float pulseSpeed = 0.5;

    static const uint8_t hueA = 15;      // Start hue at valueMin.
    uint8_t satA = 230;                  // Start saturation at valueMin.
    static constexpr float valueMin = 120.0; // Pulse minimum value (Should be less then valueMax).

    static const uint8_t hueB = 95;      // End hue at valueMax.
    static const uint8_t satB = 255;     // End saturation at valueMax.
    static constexpr float valueMax = 255.0; // Pulse maximum value (Should be larger then valueMin).

    uint8_t hue = hueA;                                                // Do Not Edit
    uint8_t sat = satA;                                                // Do Not Edit
    float val = valueMin;                                              // Do Not Edit
    uint8_t hueDelta = hueA - hueB;                                    // Do Not Edit
    static constexpr float delta = (valueMax - valueMin) / 2.35040238; // Do Not Edit
public:
    boolean handleDiff_step(State *targetState, State *presentState) final;

    void DrawEffect(State *state) final;
};

void BreatheEffect::DrawEffect(State *state)
{
    float dV = ((exp(sin(pulseSpeed * millis() / 2000.0 * PI)) - 0.36787944) * delta);
    val = valueMin + dV;
    hue = map(val, valueMin, valueMax, hueA, hueB); // Map hue based on current val
    sat = map(val, valueMin, valueMax, satA, satB); // Map sat based on current val
    // DBG("val: %f, hue: %i, sat: %i", val, hue, sat);
    
    for (int i = 0; i < NUM_LEDS; i++)
    {
        g_LEDS[i] = CHSV(hue, sat, val);

        // You can experiment with commenting out these dim8_video lines
        // to get a different sort of look.
        g_LEDS[i].r = dim8_video(g_LEDS[i].r);
        g_LEDS[i].g = dim8_video(g_LEDS[i].g);
        g_LEDS[i].b = dim8_video(g_LEDS[i].b);
    }

    // for (int i = 0; i < NUM_LEDS - 1; i++) {
    //     g_LEDS[i].setRGB(state->red, state->green, state->blue);
    // }
}
bool BreatheEffect::handleDiff_step(State *targetState, State *presentState)
{
    DBG("BreatheEffect::handleDiff_step");
      g_DelayMultiplier = 0.35; // (float)30/(float)128;
    g_TransitionDelay = targetState->speed;
    refreshDelay();
    *presentState = *targetState;
    // presentState->setBrightness(255);

    return false;
}

#endif //HOMELED_REWRITE_BREATHE_H