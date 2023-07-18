#include "hexlight.h"
#pragma once
// static HexlightCluster Cluster;

enum MODES
{
    NONE = 0,
    SOLID_L = 1,
    HORIZONTAL = 2,
    TREE = 3,

};
 void modeToStr(MODES mode, char *str)
{
    switch (mode)
    {
    case NONE:
        strcpy(str, "NONE");
        break;
    case SOLID_L:
        strcpy(str, "SOLID_L");
        break;
    case HORIZONTAL:
        strcpy(str, "HORIZONTAL");
        break;
    case TREE:
        strcpy(str, "TREE");
        break;
    default:
        strcpy(str, "INVALID");
        break;
    }
}

static constexpr uint8_t poo_offset[14] = {0, 7, 14, 28, 21, 14, 7, 14, 21, 28, 35, 42, 35, 28};
static Hexlight *HEX_LIGHTS[14] = {
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
    CRGB color = CRGB::White;
    bool horizontal_animation();
    void makePretty();
    bool tree_animation();
    bool solid_animation();

public:
    HexlightCluster();
    ~HexlightCluster();
    void setMode(MODES mode);
    MODES getMode() { return mode; }

    /**
     * Returns a pointer to the Hexlight object at the specified index.
     *
     * @param index The index of the Hexlight object to retrieve.
     * @return A pointer to the Hexlight object at the specified index, or nullptr if the index is out of range.
     */
    const Hexlight *getHexlight(uint16_t index);
    bool loop();
};
