// Cluster geometry: where every LED sits on the wall. No FastLED dependency, so it can be checked on the host
// (see test/host/geometry_check.cpp).
//
//  Layout of hex cluster (flat-top hexes, column layout)
//         11
//     12      10
//  13      9       3
//      8       4
//  7       5       2
//      6       1
//          0
#pragma once
#include <math.h>
#include <stdint.h>

#define GEO_PANELS 14
#define GEO_LEDS 35

// {column, row}; a row is half a hex height, so vertically adjacent panels are 2 rows apart.
static constexpr uint8_t PANEL_XY[GEO_PANELS][2] = {
    {2, 0}, {3, 1}, {4, 2}, {4, 4}, {3, 3}, {2, 2}, {1, 1}, {0, 2}, {1, 3}, {2, 4}, {3, 5}, {2, 6}, {1, 5}, {0, 4}};

// Calibration knobs: logical LED 0 is assumed to sit at bottom centre with indices running counterclockwise
// (from the side tables in the old hexlight.h). If shapes look mirrored or rotated on the wall, flip these.
static constexpr float LED_ZERO_DEG = 0;   // where logical LED 0 sits, degrees counterclockwise from bottom
static constexpr int8_t LED_DIR = 1;       // +1 = counterclockwise, -1 = clockwise
static constexpr float LED_RADIUS = 0.9f;  // LED ring radius, in hex radii

// All in 0..255 "wall pixels", y up. A = angle about the cluster centre (256 = full turn, counterclockwise).
static uint8_t GX[GEO_PANELS][GEO_LEDS], GY[GEO_PANELS][GEO_LEDS], GD[GEO_PANELS][GEO_LEDS], GA[GEO_PANELS][GEO_LEDS];
static uint8_t PCX[GEO_PANELS], PCY[GEO_PANELS], PD[GEO_PANELS]; // panel centre x/y, distance from cluster centre
static uint16_t NEIGHBORS[GEO_PANELS];                           // bitmask of adjacent panels

static void initGeometry()
{
    const float H = 0.8660254f;                    // sqrt(3)/2: one row step, in hex radii
    const float S = 255.0f / (6.0f + 2 * LED_RADIUS); // wall pixels per hex radius (6 = 4 columns * 1.5)
    const float cx = 3.0f, cy = 3 * H;             // cluster centre = column 2, row 3
    for (int p = 0; p < GEO_PANELS; p++)
    {
        const float px = 1.5f * PANEL_XY[p][0], py = H * PANEL_XY[p][1];
        PCX[p] = (uint8_t)((px + LED_RADIUS) * S);
        PCY[p] = (uint8_t)((py + LED_RADIUS) * S);
        PD[p] = (uint8_t)(hypotf(px - cx, py - cy) * S);
        for (int i = 0; i < GEO_LEDS; i++)
        {
            const float th = (LED_ZERO_DEG + LED_DIR * i * 360.0f / GEO_LEDS) * (float)M_PI / 180;
            const float x = px + LED_RADIUS * sinf(th), y = py - LED_RADIUS * cosf(th);
            GX[p][i] = (uint8_t)((x + LED_RADIUS) * S);
            GY[p][i] = (uint8_t)((y + LED_RADIUS) * S);
            GD[p][i] = (uint8_t)(hypotf(x - cx, y - cy) * S);
            GA[p][i] = (uint8_t)(int)(atan2f(y - cy, x - cx) * 128 / (float)M_PI);
        }
        NEIGHBORS[p] = 0;
        for (int q = 0; q < GEO_PANELS; q++)
        {
            const int dc = abs(PANEL_XY[p][0] - PANEL_XY[q][0]), dr = abs(PANEL_XY[p][1] - PANEL_XY[q][1]);
            if ((dc == 1 && dr == 1) || (dc == 0 && dr == 2))
                NEIGHBORS[p] |= 1u << q;
        }
    }
}

// Side of a panel for logical LED i: 0 bottom, 1 bottom-right, 2 top-right, 3 top, 4 top-left, 5 bottom-left.
// Bottom has 5 LEDs (33,34,0,1,2), the rest 6.
static inline uint8_t sideOf(uint8_t i)
{
    const uint8_t j = (i + 2) % GEO_LEDS;
    return j < 5 ? 0 : (j - 5) / 6 + 1;
}
