#ifndef HexlightCluster_h
#define HexlightCluster_h
#include "hexlight.h"

#define HEXLIGHT_CLUSTER_SIZE 14

// Owns the 14 panels and the effect table (HexlightCluster.cpp: EFFECTS[]). To add an effect, write an fx
// function and add one row to EFFECTS; MQTT and Home Assistant discovery pick it up from there.
class HexlightCluster
{
public:
    CRGB color = CRGB::White;
    uint8_t speed = 128; // animation rate, 128 = normal; set by MQTT "speed"

    void begin();
    bool setMode(const char *name); // false if no effect has that name
    const char *effectName() const { return effectName(mode); }
    static const char *effectName(uint8_t i);
    static uint8_t effectCount();
    // Renders a frame when one is due. Returns true if the effect changed itself (alert finished), so the
    // caller should publish state.
    bool loop();

private:
    uint8_t mode = 0, prevMode = 0;
    bool first = true;
    uint32_t lastFrame = 0, vt = 0; // vt: effect time in ms, scaled by speed, 0 at mode start
    uint16_t acc = 0;
    void setMode(uint8_t m);
};
#endif
