#include "HexlightCluster.h"
#include "geometry.h"

#define FRAME_MS 20 // 50 fps; show() for 490 LEDs takes ~15ms

static Hexlight HEX_LIGHTS[HEXLIGHT_CLUSTER_SIZE] = {
    {0, 0}, {35, 1}, {70, 1}, {105, 0}, {140, 4}, {175, 4}, {210, 4},
    {245, 5}, {280, 1}, {315, 1}, {350, 1}, {385, 5}, {420, 4}, {455, 4},
};

#define FOR_LEDS                                       \
    for (uint8_t p = 0; p < HEXLIGHT_CLUSTER_SIZE; p++) \
        for (uint8_t i = 0; i < LIGHT_LED_COUNT; i++)

static inline CRGB &px(uint8_t p, uint8_t i) { return HEX_LIGHTS[p].hexLights[i]; }
static void fillAll(CRGB c) { for (auto &h : HEX_LIGHTS) h.setAll(c); }
static void fadeAll(uint8_t by) { for (auto &h : HEX_LIGHTS) fadeToBlackBy(h.hexLights, LIGHT_LED_COUNT, by); }
static inline CRGB scaled(CRGB c, int v) { return c.nscale8(constrain(v, 0, 255)); }

// Fires every `period` ms of effect time; also fires on the first call after a mode restart.
struct Ticker
{
    uint32_t last = 0;
    bool operator()(uint32_t t, uint32_t period)
    {
        if (t < last || t - last >= period) { last = t; return true; }
        return false;
    }
};

struct Frame
{
    uint32_t t; // effect time, ms
    CRGB color; // user colour
    bool first; // first frame since the mode was set: reset any static state
};

// ---- effects. Each draws the canvas for time f.t; returning true ends the effect (back to previous mode).

static bool fxNone(const Frame &) { return false; } // freeze the last frame

static bool fxSolid(const Frame &f)
{
    fillAll(f.color);
    return false;
}

// Horizontal line bouncing bottom to top, new rainbow colour each bounce.
static bool fxHorizontal(const Frame &f)
{
    static CRGB c;
    static uint32_t bounce;
    const uint32_t step = f.t / 25, b = step / 60;
    const uint32_t idx = (b & 1) ? 60 - step % 60 : step % 60;
    if (f.first || b != bounce) { bounce = b; c = RainbowColors_p[random8(16)]; }
    fadeAll(24);
    for (uint8_t p = 0; p < HEXLIGHT_CLUSTER_SIZE; p++)
        HEX_LIGHTS[p].waveLightStep(idx - PANEL_XY[p][1] * 7, c);
    return false;
}

// Every 10s, slowly fade between a "tree" (orange top, green sides, violet bottom, each panel randomly
// rotated) and a random solid colour per panel.
static bool fxTree(const Frame &f)
{
    static const CRGB PRETTY[6] = {CRGB::BlueViolet, CRGB::Green, CRGB::Black, CRGB::DarkOrange, CRGB::Black, CRGB::Green};
    static CRGB target[HEXLIGHT_CLUSTER_SIZE][LIGHT_LED_COUNT];
    static uint32_t phase;
    const uint32_t ph = f.t / 10000;
    if (f.first || ph != phase)
    {
        phase = ph;
        for (uint8_t p = 0; p < HEXLIGHT_CLUSTER_SIZE; p++)
        {
            const uint8_t rot = random8(6);
            const CRGB solid = RainbowColors_p[random8(16)];
            for (uint8_t i = 0; i < LIGHT_LED_COUNT; i++)
                target[p][i] = (ph & 1) ? solid : PRETTY[(sideOf(i) + rot) % 6];
        }
    }
    FOR_LEDS stepShift(px(p, i), target[p][i]);
    return false;
}

// Per-panel spinner: a comet with a tail runs round each panel, alternate columns turning opposite ways.
static bool fxSweep(const Frame &f)
{
    const uint8_t head = f.t / 8; // ~2s per turn
    FOR_LEDS
    {
        const uint8_t a = i * 256 / LIGHT_LED_COUNT;
        const uint8_t d = (PANEL_XY[p][0] & 1) ? uint8_t(head - a) : uint8_t(a + head);
        px(p, i) = scaled(f.color, 255 - d * 3);
    }
    return false;
}

// Panels randomly flood with a rainbow colour, then sparkle for a few beats.
static bool fxTwinkle(const Frame &f)
{
    static uint8_t left[HEXLIGHT_CLUSTER_SIZE];
    static Ticker tick;
    if (f.first) memset(left, 0, sizeof(left));
    fadeAll(24);
    if (tick(f.t, 300))
        for (uint8_t p = 0; p < HEXLIGHT_CLUSTER_SIZE; p++)
        {
            if (left[p]) { px(p, random8(LIGHT_LED_COUNT)) = RainbowColors_p[random8(16)]; left[p]--; }
            else if (random8(100) > 90) { HEX_LIGHTS[p].setAll(RainbowColors_p[random8(16)]); left[p] = random8(10); }
        }
    return false;
}

// User colour breathing, phase offset by distance so it ripples outward from the centre.
static bool fxBreathe(const Frame &f)
{
    for (uint8_t p = 0; p < HEXLIGHT_CLUSTER_SIZE; p++)
        HEX_LIGHTS[p].setAll(scaled(f.color, 40 + scale8(sin8((f.t >> 4) - PD[p]), 215)));
    return false;
}

// Rainbow gradient across the whole wall; its direction slowly rotates.
static bool fxRainbow(const Frame &f)
{
    const uint8_t ang = f.t / 160;
    const int cx = int(cos8(ang)) - 128, sy = int(sin8(ang)) - 128;
    const uint8_t base = f.t / 16;
    FOR_LEDS px(p, i) = CHSV(base + ((GX[p][i] * cx + GY[p][i] * sy) >> 7), 255, 255);
    return false;
}

// Slowly moving 2D noise through a palette.
static bool noiseFx(const Frame &f, const TProgmemRGBPalette16 &pal)
{
    const uint16_t z = f.t / 8;
    FOR_LEDS px(p, i) = ColorFromPalette(pal, inoise8(GX[p][i] * 3, GY[p][i] * 3, z));
    return false;
}

// A beam rotating around the cluster centre, sweeping across every panel.
static bool fxRadar(const Frame &f)
{
    const uint8_t beam = f.t / 12; // ~3s per turn
    FOR_LEDS px(p, i) = scaled(f.color, 255 - uint8_t(beam - GA[p][i]) * 3);
    return false;
}

// Rings expanding from random panels, crossing panel edges smoothly.
static bool fxRipple(const Frame &f)
{
    struct Drop { uint8_t x, y, hue; uint32_t born; bool live; };
    static Drop drops[3];
    static uint8_t next;
    static Ticker tick;
    if (f.first) for (auto &d : drops) d.live = false;
    if (tick(f.t, 900))
    {
        const uint8_t p = random8(HEXLIGHT_CLUSTER_SIZE);
        drops[next++ % 3] = {PCX[p], PCY[p], random8(), f.t, true};
    }
    fadeAll(40);
    for (auto &d : drops)
    {
        if (!d.live) continue;
        const int r = (f.t - d.born) / 6; // wall pixels
        if (r > 300) { d.live = false; continue; }
        const uint8_t fade = 255 - r * 255 / 300;
        FOR_LEDS
        {
            const int dx = GX[p][i] - d.x, dy = GY[p][i] - d.y;
            const int diff = abs(int(sqrt16((dx * dx + dy * dy) >> 2)) * 2 - r);
            if (diff < 16) px(p, i) |= CRGB(CHSV(d.hue, 220, scale8(255 - diff * 16, fade)));
        }
    }
    return false;
}

// Fire2012 on an 8-column x 16-row heat grid; sparks at the bottom, heat rises.
static bool fxFire(const Frame &f)
{
    static uint8_t heat[8][16];
    static Ticker tick;
    if (f.first) memset(heat, 0, sizeof(heat));
    if (tick(f.t, 30))
        for (auto &col : heat)
        {
            for (auto &h : col) h = qsub8(h, random8(0, 55 * 10 / 16 + 2));
            for (int y = 15; y >= 2; y--) col[y] = (col[y - 1] + col[y - 2] * 2) / 3;
            if (random8() < 120) { uint8_t y = random8(3); col[y] = qadd8(col[y], random8(160, 255)); }
        }
    FOR_LEDS px(p, i) = ColorFromPalette(HeatColors_p, scale8(heat[GX[p][i] / 32][GY[p][i] / 15], 240));
    return false;
}

// Sparkles in hues around the user colour.
static bool fxConfetti(const Frame &f)
{
    const CHSV base = rgb2hsv_approximate(f.color);
    fadeAll(10);
    for (int k = 0; k < 3; k++)
        px(random8(HEXLIGHT_CLUSTER_SIZE), random8(LIGHT_LED_COUNT)) |= CRGB(CHSV(base.h + random8(64) - 32, base.s, 255));
    return false;
}

// Panels fill outward from the centre, then drain back (~6s cycle).
static bool fxBloom(const Frame &f)
{
    const int level = scale8(triwave8(f.t / 24), 200);
    for (uint8_t p = 0; p < HEXLIGHT_CLUSTER_SIZE; p++)
        HEX_LIGHTS[p].setAll(scaled(f.color, (level - PD[p]) * 4));
    return false;
}

// Three red pulses, then back to whatever was running.
static bool fxAlert(const Frame &f)
{
    if (f.t >= 1500) return true;
    fillAll(scaled(CRGB::Red, quadwave8(f.t * 256 / 500)));
    return false;
}

// Dark -> deep red -> orange -> warm white over 10 minutes at speed 128 (20 min at 64), rising from the
// bottom; then holds.
DEFINE_GRADIENT_PALETTE(sunrise_gp){0, 0, 0, 0, 90, 120, 8, 0, 170, 255, 70, 0, 255, 255, 170, 80};
static bool fxSunrise(const Frame &f)
{
    static const CRGBPalette16 pal = sunrise_gp;
    const int prog = min<uint32_t>(255, f.t / (600000 / 255));
    FOR_LEDS px(p, i) = ColorFromPalette(pal, constrain(prog * 5 / 4 + (128 - GY[p][i]) / 4 - 32, 0, 240));
    return false;
}

// Hex life (B2/S34) on the 14 panels; newborns inherit a neighbour's hue with drift. Reseeds when it dies,
// stalls, or after 40 generations.
static bool fxLife(const Frame &f)
{
    static uint16_t alive;
    static uint8_t hue[HEXLIGHT_CLUSTER_SIZE], still, gens;
    static Ticker tick;
    const bool reseed = f.first || alive == 0 || still >= 3 || gens >= 40;
    if (reseed && (f.first || tick(f.t, 1200)))
    {
        alive = random16() & 0x3FFF;
        for (auto &h : hue) h = random8();
        still = gens = 0;
    }
    else if (!reseed && tick(f.t, 1200))
    {
        uint16_t nxt = 0;
        for (uint8_t p = 0; p < HEXLIGHT_CLUSTER_SIZE; p++)
        {
            const uint16_t nb = alive & NEIGHBORS[p];
            const int n = __builtin_popcount(nb);
            if ((alive >> p) & 1) { if (n == 3 || n == 4) nxt |= 1u << p; }
            else if (n == 2)
            {
                nxt |= 1u << p;
                hue[p] = hue[__builtin_ctz(nb)] + random8(24);
            }
        }
        still = nxt == alive ? still + 1 : 0;
        alive = nxt;
        gens++;
    }
    FOR_LEDS nblend(px(p, i), ((alive >> p) & 1) ? CRGB(CHSV(hue[p], 255, 255)) : CRGB::Black, 12);
    return false;
}

struct Effect
{
    const char *name;
    bool (*fn)(const Frame &);
};
static const Effect EFFECTS[] = {
    {"none", fxNone},
    {"solid", fxSolid},
    {"horizontal", fxHorizontal},
    {"tree", fxTree},
    {"sweep", fxSweep},
    {"twinkle", fxTwinkle},
    {"breathe", fxBreathe},
    {"rainbow", fxRainbow},
    {"plasma", [](const Frame &f) { return noiseFx(f, PartyColors_p); }},
    {"lava", [](const Frame &f) { return noiseFx(f, LavaColors_p); }},
    {"ocean", [](const Frame &f) { return noiseFx(f, OceanColors_p); }},
    {"forest", [](const Frame &f) { return noiseFx(f, ForestColors_p); }},
    {"radar", fxRadar},
    {"ripple", fxRipple},
    {"fire", fxFire},
    {"confetti", fxConfetti},
    {"bloom", fxBloom},
    {"alert", fxAlert},
    {"sunrise", fxSunrise},
    {"life", fxLife},
};
static constexpr uint8_t EFFECT_COUNT = sizeof(EFFECTS) / sizeof(EFFECTS[0]);

void HexlightCluster::begin() { initGeometry(); }

uint8_t HexlightCluster::effectCount() { return EFFECT_COUNT; }

const char *HexlightCluster::effectName(uint8_t i) { return i < EFFECT_COUNT ? EFFECTS[i].name : nullptr; }

bool HexlightCluster::setMode(const char *name)
{
    for (uint8_t m = 0; name && m < EFFECT_COUNT; m++)
        if (strcmp(name, EFFECTS[m].name) == 0) { setMode(m); return true; }
    return false;
}

void HexlightCluster::setMode(uint8_t m)
{
    if (EFFECTS[m].fn == fxAlert && mode != m) prevMode = mode;
    mode = m;
    vt = acc = 0;
    first = true;
    if (EFFECTS[m].fn != fxNone) fillAll(CRGB::Black);
}

bool HexlightCluster::loop()
{
    const uint32_t now = millis();
    if (now - lastFrame < FRAME_MS) return false;
    const uint32_t dt = min<uint32_t>(now - lastFrame, 100); // don't jump after a stall
    lastFrame = now;

    const bool done = EFFECTS[mode].fn({vt, color, first});
    first = false;
    acc += dt * speed;
    vt += acc >> 7;
    acc &= 127;

    for (auto &h : HEX_LIGHTS) h.syncAll();
    FastLED.show();
    if (done) setMode(prevMode);
    return done;
}
