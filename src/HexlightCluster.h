#ifndef HexlightCluster_h
#define HexlightCluster_h
#include "hexlight.h"

#define HEXLIGHT_CLUSTER_SIZE 14
// #pragma once
// static HexlightCluster Cluster;

enum MODES
{
    NONE = 0,
    SOLID_L = 1,
    HORIZONTAL = 2,
    TREE = 3,
    SWEEP = 4,
    TWINKLE = 5,

};

// const char *modeToStr(MODES mode);
const char *modeToStr(MODES mode);

static constexpr uint8_t poo_offset[14] = {0, 7, 14, 28, 21, 14, 7, 14, 21, 28, 35, 42, 35, 28};
static Hexlight *HEX_LIGHTS[HEXLIGHT_CLUSTER_SIZE] = {
    new Hexlight(0, 34, 0),
    new Hexlight(35, 69, 1),
    new Hexlight(70, 104, 1),
    new Hexlight(105, 139, 0),
    new Hexlight(140, 174, 4),
    new Hexlight(175, 209, 4),
    new Hexlight(210, 244, 4),
    new Hexlight(245, 279, 5),
    new Hexlight(280, 314, 1),
    new Hexlight(315, 349, 1),
    new Hexlight(uint16_t(350), uint16_t(384), (1)),
    new Hexlight(uint16_t(385), uint16_t(419), uint8_t(5)),
    new Hexlight((uint16_t)420, uint16_t(454), uint8_t(4)),
    new Hexlight(uint16_t(455), uint16_t(489), uint8_t(4)),
};

class HexlightCluster
{
private:
    MODES mode = NONE;
    bool horizontal_animation();
    static void makePretty();
    bool tree_animation();
    bool solid_animation() const;
    bool sweep_animation(bool clockwise) const;
    static bool twinkle_animation() ;

public:
    HexlightCluster();
    ~HexlightCluster();
    void setMode(MODES newMode);
    MODES getMode() { return mode; }
    CRGB color = CRGB::White;

    /**
     * Returns a pointer to the Hexlight object at the specified index.
     *
     * @param index The index of the Hexlight object to retrieve.
     * @return A pointer to the Hexlight object at the specified index, or nullptr if the index is out of range.
     */
    const Hexlight *getHexlight(uint16_t index);
    bool loop();
};
#endif
