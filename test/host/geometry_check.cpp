// Host-side sanity check for src/geometry.h.
// Run: c++ -std=c++17 -Isrc test/host/geometry_check.cpp -o /tmp/geo && /tmp/geo
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "geometry.h"

int main()
{
    initGeometry();
    // poo_offset used to be row * 7; rows must still match it.
    const uint8_t poo_offset[14] = {0, 7, 14, 28, 21, 14, 7, 14, 21, 28, 35, 42, 35, 28};
    for (int p = 0; p < 14; p++) assert(PANEL_XY[p][1] * 7 == poo_offset[p]);
    // Neighbour table from the layout map.
    assert(NEIGHBORS[5] == ((1u << 0) | (1u << 1) | (1u << 4) | (1u << 9) | (1u << 8) | (1u << 6)));
    assert(NEIGHBORS[11] == ((1u << 9) | (1u << 10) | (1u << 12)));
    for (int p = 0; p < 14; p++) for (int q = 0; q < 14; q++)
        assert(((NEIGHBORS[p] >> q) & 1) == ((NEIGHBORS[q] >> p) & 1));
    // LED 0 is bottom centre, LED ~9 is right of centre, LED ~17 is top.
    assert(GY[0][0] < GY[0][17] && GX[0][9] > GX[0][26]);
    // Panel 0 is at the bottom, 11 at the top, 7/13 at the left, 2/3 at the right.
    assert(PCY[0] < PCY[5] && PCY[5] < PCY[9] && PCY[9] < PCY[11]);
    assert(PCX[7] < PCX[6] && PCX[6] < PCX[0] && PCX[0] < PCX[1] && PCX[1] < PCX[2]);
    // Sides: 5 + 6*5 = 35.
    int count[6] = {};
    for (int i = 0; i < 35; i++) count[sideOf(i)]++;
    assert(count[0] == 5 && sideOf(0) == 0 && sideOf(34) == 0 && sideOf(3) == 1 && sideOf(17) == 3 && sideOf(32) == 5);
    for (int c = 1; c < 6; c++) assert(count[c] == 6);
    // Nothing clipped: max of each axis near (not over) 255.
    int mx = 0, my = 0, md = 0;
    for (int p = 0; p < 14; p++) for (int i = 0; i < 35; i++)
    { mx = GX[p][i] > mx ? GX[p][i] : mx; my = GY[p][i] > my ? GY[p][i] : my; md = GD[p][i] > md ? GD[p][i] : md; }
    assert(mx > 240 && my > 200 && md < 200);
    printf("geometry ok (max x %d, y %d, d %d; inner PD %d %d)\n", mx, my, md, PD[5], PD[4]);
}
