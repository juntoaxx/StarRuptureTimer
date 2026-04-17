#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// Windows.h defines DrawText as DrawTextW in Unicode builds — collides with AHUD::DrawText.
#ifdef DrawText
#undef DrawText
#endif

#include "plugin.h"
#include "plugin_helpers.h"

#if defined(MODLOADER_CLIENT_BUILD)
#include "Basic.hpp"
#include "Engine_classes.hpp"
#include "Chimera_classes.hpp"
#endif

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <mutex>

// ---------------------------------------------------------------------------
//  Plugin identity
// ---------------------------------------------------------------------------

static IPluginSelf* g_self = nullptr;
IPluginSelf* GetSelf() { return g_self; }

static PluginInfo s_info = {
    "StarRuptureTimer",
    "1.0.0",
    "Juntoaxx",
    "HUD overlay showing the current Arcadia rupture phase and countdown.",
    PLUGIN_INTERFACE_VERSION
};

// ---------------------------------------------------------------------------
//  32-colour preset table
//  Set Preset=N in any [Colors.*] section to pick a named colour.
//  Preset=0 (default) uses the custom R/G/B values instead.
// ---------------------------------------------------------------------------

struct ColorPreset { const char* name; int r, g, b; };

static const ColorPreset k_presets[32] = {
    /*  1 */ { "White",      255, 255, 255 },
    /*  2 */ { "LightGrey",  200, 200, 200 },
    /*  3 */ { "Grey",       128, 128, 128 },
    /*  4 */ { "DarkGrey",    64,  64,  64 },
    /*  5 */ { "Black",       20,  20,  20 },
    /*  6 */ { "Red",        220,  50,  50 },
    /*  7 */ { "DarkRed",    160,  20,  20 },
    /*  8 */ { "Orange",     255, 140,   0 },
    /*  9 */ { "Amber",      255, 185,   0 },
    /* 10 */ { "Yellow",     240, 220,   0 },
    /* 11 */ { "Lime",        80, 220,  50 },
    /* 12 */ { "Green",        0, 190,  80 },
    /* 13 */ { "DarkGreen",    0, 130,  50 },
    /* 14 */ { "Teal",         0, 175, 160 },
    /* 15 */ { "Cyan",         0, 210, 210 },
    /* 16 */ { "SkyBlue",    100, 175, 225 },
    /* 17 */ { "Blue",        50, 100, 220 },
    /* 18 */ { "DarkBlue",    20,  40, 160 },
    /* 19 */ { "Navy",        10,  20, 120 },
    /* 20 */ { "Indigo",      80,   0, 160 },
    /* 21 */ { "Purple",     150,   0, 200 },
    /* 22 */ { "Violet",     180,  80, 230 },
    /* 23 */ { "Magenta",    220,   0, 180 },
    /* 24 */ { "HotPink",    255,  80, 160 },
    /* 25 */ { "Pink",       255, 155, 195 },
    /* 26 */ { "Coral",      255, 100,  75 },
    /* 27 */ { "Salmon",     240, 120, 100 },
    /* 28 */ { "Gold",       255, 200,  40 },
    /* 29 */ { "Bronze",     180, 120,  50 },
    /* 30 */ { "Turquoise",   60, 215, 185 },
    /* 31 */ { "Mint",       140, 235, 185 },
    /* 32 */ { "Lavender",   180, 150, 240 },
};

// Compact colour list written into the INI as documentation.
#define PRESET_LIST \
    "0=Custom  1=White  2=LightGrey  3=Grey  4=DarkGrey  5=Black  " \
    "6=Red  7=DarkRed  8=Orange  9=Amber  10=Yellow  11=Lime  12=Green  " \
    "13=DarkGreen  14=Teal  15=Cyan  16=SkyBlue  17=Blue  18=DarkBlue  " \
    "19=Navy  20=Indigo  21=Purple  22=Violet  23=Magenta  24=HotPink  " \
    "25=Pink  26=Coral  27=Salmon  28=Gold  29=Bronze  30=Turquoise  " \
    "31=Mint  32=Lavender"

// ---------------------------------------------------------------------------
//  Config schema
// ---------------------------------------------------------------------------

static const ConfigEntry k_configEntries[] = {
    // ── General ──────────────────────────────────────────────────────────────
    // Set Enabled=false to completely hide the overlay without unloading the plugin.
    { "General", "Enabled", ConfigValueType::Boolean, "true",
      "true = overlay visible | false = overlay hidden" },

    // ── Display ──────────────────────────────────────────────────────────────
    // Alpha controls how opaque the overlay is.  0 = invisible, 100 = fully solid.
    { "Display", "Alpha", ConfigValueType::Integer, "80",
      "Overlay opacity: 0 (invisible) to 100 (fully solid). Default: 80" },

    // Scale controls the width of the panel in pixels.
    // 0 = 200 px wide (narrowest), 100 = 300 px wide (widest).
    { "Display", "Scale", ConfigValueType::Integer, "50",
      "Panel width: 0 (200px/narrowest) to 100 (300px/widest). Default: 50" },

    // FontSize scales the text from normal (1) up to double size (10).
    // Increase this if the text is hard to read at your resolution.
    { "Display", "FontSize", ConfigValueType::Integer, "1",
      "Text size: 1 (normal) to 10 (double size). Default: 1" },

    // PosX / PosY set the top-left corner of the panel in screen pixels.
    // The origin (0, 0) is the top-left corner of the screen.
    // Set both to -1 to use automatic positioning (lower-left corner).
    // Examples for 1920x1080:
    //   Lower-left  -> PosX=20   PosY=1004
    //   Lower-right -> PosX=1650 PosY=1004
    //   Top-left    -> PosX=20   PosY=20
    { "Display", "PosX", ConfigValueType::Integer, "-1",
      "Panel X position (pixels from left edge). -1 = auto lower-left. Default: -1" },
    { "Display", "PosY", ConfigValueType::Integer, "-1",
      "Panel Y position (pixels from top edge).  -1 = auto lower-left. Default: -1" },

    // Rotation tilts the entire panel around its centre point.
    // Positive values lean clockwise, negative values lean counter-clockwise.
    // Text labels stay screen-aligned but move with the panel at all angles.
    { "Display", "Rotation", ConfigValueType::Integer, "0",
      "Panel tilt in degrees: -90 (counter-clockwise) to 90 (clockwise). 0 = flat. Default: 0" },

    // ── Phase colours ─────────────────────────────────────────────────────────
    // Each phase has its own colour used for the phase name, timer text,
    // accent bar, and progress bar fill.
    //
    // TWO ways to set a colour:
    //   Option A — Preset: set Preset=1-32 to pick a named colour from the list.
    //              The R/G/B values below are ignored when Preset is 1-32.
    //   Option B — Custom: set Preset=0, then edit R, G, B (each 0-255).
    //
    // Colour presets:
    //   " PRESET_LIST "

    // Stable phase — shown while Arcadia is calm.  Default: teal (14).
    { "Colors.Stable", "Preset", ConfigValueType::Integer, "0",  PRESET_LIST },
    { "Colors.Stable", "R",      ConfigValueType::Integer, "30",  "Custom red   0-255 (ignored when Preset is 1-32)" },
    { "Colors.Stable", "G",      ConfigValueType::Integer, "195", "Custom green 0-255 (ignored when Preset is 1-32)" },
    { "Colors.Stable", "B",      ConfigValueType::Integer, "210", "Custom blue  0-255 (ignored when Preset is 1-32)" },

    // Incoming phase — rupture is approaching.  Default: amber (9).
    { "Colors.Incoming", "Preset", ConfigValueType::Integer, "0",  PRESET_LIST },
    { "Colors.Incoming", "R",      ConfigValueType::Integer, "220", "Custom red   0-255 (ignored when Preset is 1-32)" },
    { "Colors.Incoming", "G",      ConfigValueType::Integer, "170", "Custom green 0-255 (ignored when Preset is 1-32)" },
    { "Colors.Incoming", "B",      ConfigValueType::Integer, "40",  "Custom blue  0-255 (ignored when Preset is 1-32)" },

    // Burning phase — active rupture wave.  Default: orange-red (8).
    { "Colors.Burning", "Preset", ConfigValueType::Integer, "0",  PRESET_LIST },
    { "Colors.Burning", "R",      ConfigValueType::Integer, "220", "Custom red   0-255 (ignored when Preset is 1-32)" },
    { "Colors.Burning", "G",      ConfigValueType::Integer, "70",  "Custom green 0-255 (ignored when Preset is 1-32)" },
    { "Colors.Burning", "B",      ConfigValueType::Integer, "30",  "Custom blue  0-255 (ignored when Preset is 1-32)" },

    // Cooling phase — wave receding.  Default: cool blue (17).
    { "Colors.Cooling", "Preset", ConfigValueType::Integer, "0",  PRESET_LIST },
    { "Colors.Cooling", "R",      ConfigValueType::Integer, "40",  "Custom red   0-255 (ignored when Preset is 1-32)" },
    { "Colors.Cooling", "G",      ConfigValueType::Integer, "140", "Custom green 0-255 (ignored when Preset is 1-32)" },
    { "Colors.Cooling", "B",      ConfigValueType::Integer, "210", "Custom blue  0-255 (ignored when Preset is 1-32)" },

    // Stabilizing phase — regrowth after the wave.  Default: muted teal-grey (30).
    { "Colors.Stabilizing", "Preset", ConfigValueType::Integer, "0",  PRESET_LIST },
    { "Colors.Stabilizing", "R",      ConfigValueType::Integer, "100", "Custom red   0-255 (ignored when Preset is 1-32)" },
    { "Colors.Stabilizing", "G",      ConfigValueType::Integer, "155", "Custom green 0-255 (ignored when Preset is 1-32)" },
    { "Colors.Stabilizing", "B",      ConfigValueType::Integer, "165", "Custom blue  0-255 (ignored when Preset is 1-32)" },
};
static const ConfigSchema k_schema = { k_configEntries, sizeof(k_configEntries) / sizeof(k_configEntries[0]) };

// ---------------------------------------------------------------------------
//  Phase definitions
// ---------------------------------------------------------------------------

struct Phase {
    const char* name;
    float       r, g, b;
    float       defaultSecs;
};

static Phase k_unknown     = { "Rupture Timer",    0.2f, 0.8f, 0.8f,    0.f };
static Phase k_stable      = { "Arcadia Stable",   0.12f, 0.77f, 0.82f, 3120.f };
static Phase k_incoming    = { "Rupture Incoming", 0.86f, 0.67f, 0.16f,   30.f };
static Phase k_burning     = { "Arcadia Burning",  0.86f, 0.27f, 0.12f,   30.f };
static Phase k_cooling     = { "Arcadia Cooling",  0.16f, 0.55f, 0.82f,   60.f };
static Phase k_stabilizing = { "Stabilizing",      0.39f, 0.61f, 0.65f,  600.f };

// ---------------------------------------------------------------------------
//  Display config
// ---------------------------------------------------------------------------

struct DisplayConfig {
    float alpha     = 0.8f;
    int   scale     = 50;
    float fontScale = 1.f;
    int   posX      = -1;
    int   posY      = -1;
    int   rotation  = 0;
};
static DisplayConfig g_display;
static std::mutex    g_displayMutex;

static float Chan(int v) { return std::max(0.f, std::min(1.f, v / 255.f)); }

static void LoadDisplayConfig()
{
    auto* s = GetSelf();
    if (!s) return;

    DisplayConfig d;
    d.alpha     = std::max(0.f, std::min(1.f, s->config->ReadInt(s, "Display", "Alpha", 80) / 100.f));
    d.scale     = std::max(0,   std::min(100, s->config->ReadInt(s, "Display", "Scale", 50)));
    int fontSize = std::max(1, std::min(10, s->config->ReadInt(s, "Display", "FontSize", 1)));
    d.fontScale = 1.0f + (fontSize - 1) * (1.0f / 9.0f);  // 1->1.0, 10->2.0
    d.posX      = s->config->ReadInt(s, "Display", "PosX", -1);
    d.posY      = s->config->ReadInt(s, "Display", "PosY", -1);
    d.rotation  = std::max(-90, std::min(90, s->config->ReadInt(s, "Display", "Rotation", 0)));

    auto readPhase = [&](Phase& ph, const char* sec, int dr, int dg, int db) {
        int preset = s->config->ReadInt(s, sec, "Preset", 0);
        if (preset >= 1 && preset <= 32) {
            const ColorPreset& p = k_presets[preset - 1];
            ph.r = Chan(p.r);
            ph.g = Chan(p.g);
            ph.b = Chan(p.b);
        } else {
            ph.r = Chan(s->config->ReadInt(s, sec, "R", dr));
            ph.g = Chan(s->config->ReadInt(s, sec, "G", dg));
            ph.b = Chan(s->config->ReadInt(s, sec, "B", db));
        }
    };
    readPhase(k_stable,      "Colors.Stable",        30, 195, 210);
    readPhase(k_incoming,    "Colors.Incoming",      220, 170,  40);
    readPhase(k_burning,     "Colors.Burning",       220,  70,  30);
    readPhase(k_cooling,     "Colors.Cooling",        40, 140, 210);
    readPhase(k_stabilizing, "Colors.Stabilizing",   100, 155, 165);

    std::lock_guard<std::mutex> lk(g_displayMutex);
    g_display = d;
}

static void OnConfigChanged(const char*, const char*, const char*) { LoadDisplayConfig(); }

// ---------------------------------------------------------------------------
//  Timer state
// ---------------------------------------------------------------------------

struct TimerState {
    const Phase* phase     = &k_unknown;
    float        remaining = 0.f;
    float        announced = 0.f;
    bool         hasData   = false;
};
static TimerState g_state;
static std::mutex g_timerMutex;

// ---------------------------------------------------------------------------
//  Game world polling  (client builds only)
// ---------------------------------------------------------------------------

#if defined(MODLOADER_CLIENT_BUILD)

static SDK::UWorld* g_world = nullptr;

static constexpr float COOLING_DURATION     = 60.f;
static constexpr float STABILIZING_DURATION = 600.f;
static float s_phase3StartRemaining = -1.f;
static int   s_prevNextPhase        = -1;

static void PollTimerState()
{
    SDK::UWorld* world = g_world ? g_world : SDK::UWorld::GetWorld();
    if (!world) return;

    auto* gameState = static_cast<SDK::ACrGameStateBase*>(world->GameState);
    if (!gameState || !gameState->WaveTimerActor) return;

    SDK::ACrWaveTimerActor* timer = gameState->WaveTimerActor;
    double serverTime             = gameState->GetServerWorldTimeSeconds();
    float  unclamped              = timer->NextTime - static_cast<float>(serverTime);
    float  nextTimeRemaining      = std::max(0.f, unclamped);
    int    nextPhase              = (int)timer->NextPhase;

    if (nextPhase != s_prevNextPhase) {
        if (nextPhase == 3) s_phase3StartRemaining = nextTimeRemaining;
        s_prevNextPhase = nextPhase;
    }

    const Phase* phase     = nullptr;
    float        remaining = 0.f;
    float        announced = 0.f;

    switch (nextPhase) {
        case 0:
            phase = &k_stable; remaining = nextTimeRemaining; announced = nextTimeRemaining;
            break;
        case 1:
            phase = &k_incoming; remaining = nextTimeRemaining; announced = nextTimeRemaining;
            break;
        case 2:
            phase = &k_burning; remaining = nextTimeRemaining;
            announced = (s_phase3StartRemaining > 0.f) ? s_phase3StartRemaining : k_burning.defaultSecs;
            break;
        case 3: {
            float boundary = (s_phase3StartRemaining > 0.f)
                ? (s_phase3StartRemaining - COOLING_DURATION)
                : STABILIZING_DURATION;
            if (nextTimeRemaining > boundary) {
                phase = &k_cooling; remaining = nextTimeRemaining - boundary; announced = COOLING_DURATION;
            } else {
                phase = &k_stabilizing; remaining = nextTimeRemaining;
                announced = boundary > 0.f ? boundary : STABILIZING_DURATION;
            }
            break;
        }
        default:
            phase = &k_unknown;
            break;
    }

    if (phase) {
        std::lock_guard<std::mutex> lk(g_timerMutex);
        g_state.phase     = phase;
        g_state.remaining = remaining;
        g_state.announced = (announced > 0.f) ? announced : phase->defaultSecs;
        g_state.hasData   = true;
    }
}

static float s_pollAccum = 0.f;

static void OnTick(float delta)
{
    {
        std::lock_guard<std::mutex> lk(g_timerMutex);
        if (g_state.remaining > 0.f)
            g_state.remaining = std::max(0.f, g_state.remaining - delta);
    }
    s_pollAccum += delta;
    if (s_pollAccum >= 0.5f) {
        s_pollAccum = 0.f;
        PollTimerState();
    }
}

static void OnAnyWorldBeginPlay(SDK::UWorld* world, const char* worldName)
{
    if (strstr(worldName, "ChimeraMain")) {
        g_world                = world;
        s_prevNextPhase        = -1;
        s_phase3StartRemaining = -1.f;
        LOG_INFO("RuptureTimer: game world active — %s", worldName);
    } else {
        g_world = nullptr;
    }
}

// ---------------------------------------------------------------------------
//  F8 test keybind — cycles phases for visual testing
// ---------------------------------------------------------------------------

static void OnTestKey(EModKey, EModKeyEvent)
{
    static const Phase* s_cycle[] = {
        &k_stable, &k_incoming, &k_burning, &k_cooling, &k_stabilizing
    };
    static int s_idx = 0;
    const Phase* p = s_cycle[s_idx++ % 5];
    std::lock_guard<std::mutex> lk(g_timerMutex);
    g_state.phase     = p;
    g_state.remaining = p->defaultSecs;
    g_state.announced = p->defaultSecs;
    g_state.hasData   = true;
    LOG_INFO("RuptureTimer: [TEST] %s", p->name);
}

// ---------------------------------------------------------------------------
//  HUD PostRender — draws the overlay panel with background + border
// ---------------------------------------------------------------------------

// Converts an ASCII string to a wchar_t buffer for SDK::FString (stack-local, non-allocating).
// dest must be at least (strlen(src)+1) wchar_t wide.
static void AsciiToWide(const char* src, wchar_t* dest, int maxLen)
{
    int i = 0;
    while (src[i] && i < maxLen - 1) { dest[i] = (wchar_t)(unsigned char)src[i]; ++i; }
    dest[i] = L'\0';
}

static inline SDK::FLinearColor LC(float r, float g, float b, float a)
{
    return { r, g, b, a };
}


static void OnPostRender(void* hudPtr)
{
    auto* hud = static_cast<SDK::AHUD*>(hudPtr);
    if (!hud || !hud->Canvas) return;

    TimerState    ts;
    DisplayConfig dc;
    { std::lock_guard<std::mutex> lk(g_timerMutex);   ts = g_state;   }
    { std::lock_guard<std::mutex> lk(g_displayMutex); dc = g_display; }

    float a       = dc.alpha;
    float screenH = hud->Canvas->ClipY;

    // Panel dimensions
    float panelW = 200.f + (dc.scale * 100.f / 100.f);  // 200..300 px
    float panelH = 56.f;
    float pad    = 10.f;
    float barH   = 6.f;

    // Top-left anchor position
    float px = (dc.posX >= 0) ? (float)dc.posX : 20.f;
    float py = (dc.posY >= 0) ? (float)dc.posY : (screenH - panelH - 20.f);

    // Rotation — everything rotates around the panel center
    float angleRad = dc.rotation * (3.14159265f / 180.f);
    float cosA     = cosf(angleRad);
    float sinA     = sinf(angleRad);

    float cx = px + panelW * 0.5f;
    float cy = py + panelH * 0.5f;
    float hw = panelW * 0.5f;
    float hh = panelH * 0.5f;

    // Rotate a panel-local offset (dx,dy) into screen space around the panel center
    auto R = [&](float dx, float dy) -> SDK::FVector2D {
        return { (double)(cx + dx * cosA - dy * sinA),
                 (double)(cy + dx * sinA + dy * cosA) };
    };

    // Thick K2_DrawLine: fills the rectangle swept along the line at the given thickness
    auto Line = [&](SDK::FVector2D a2, SDK::FVector2D b2, float thick, SDK::FLinearColor col) {
        hud->Canvas->K2_DrawLine(a2, b2, thick, col);
    };

    // ── White outer border (4 edges) ─────────────────────────────────────────
    auto borderCol = LC(1.f, 1.f, 1.f, a * 0.55f);
    Line(R(-hw, -hh), R( hw, -hh), 2.f, borderCol);  // top
    Line(R(-hw,  hh), R( hw,  hh), 2.f, borderCol);  // bottom
    Line(R(-hw, -hh), R(-hw,  hh), 2.f, borderCol);  // left
    Line(R( hw, -hh), R( hw,  hh), 2.f, borderCol);  // right

    // ── Phase color accent bar (3px, just inside top border) ─────────────────
    Line(R(-hw + 2.f, -hh + 3.f), R(hw - 2.f, -hh + 3.f), 3.f,
         LC(ts.phase->r, ts.phase->g, ts.phase->b, a * 0.9f));

    // ── Phase name text ───────────────────────────────────────────────────────
    {
        wchar_t wbuf[64];
        AsciiToWide(ts.phase->name, wbuf, 64);
        SDK::FString fs(wbuf);
        auto pos = R(-hw + pad, -hh + 10.f);
        hud->DrawText(fs, LC(ts.phase->r, ts.phase->g, ts.phase->b, a),
                      (float)pos.X, (float)pos.Y, nullptr, dc.fontScale, false);
    }

    if (!ts.hasData) return;

    // ── Countdown text ────────────────────────────────────────────────────────
    {
        char    cbuf[8];
        int     mins = (int)ts.remaining / 60;
        int     secs = (int)ts.remaining % 60;
        snprintf(cbuf, sizeof(cbuf), "%02d:%02d", mins, secs);
        wchar_t wbuf[16];
        AsciiToWide(cbuf, wbuf, 16);
        SDK::FString fs(wbuf);
        auto pos = R(hw - 52.f, -hh + 10.f);
        hud->DrawText(fs, LC(ts.phase->r, ts.phase->g, ts.phase->b, a * 0.85f),
                      (float)pos.X, (float)pos.Y, nullptr, dc.fontScale, false);
    }

    // ── Progress bar ──────────────────────────────────────────────────────────
    // Bar center is 6px below the panel center (py+34 vs panel center at py+28)
    float barDy  = 6.f;
    float barHW  = hw - pad;  // bar half-width
    float barTW  = barHW * 2.f;
    float frac   = (ts.announced > 0.f) ? std::min(1.f, ts.remaining / ts.announced) : 1.f;

    // Empty track
    Line(R(-barHW, barDy), R(barHW, barDy), barH, LC(0.08f, 0.10f, 0.12f, a * 0.8f));

    // Filled portion (left-aligned: from -barHW to -barHW + barTW*frac)
    if (frac > 0.f) {
        float fillEnd = -barHW + barTW * frac;
        Line(R(-barHW, barDy), R(fillEnd, barDy), barH,
             LC(ts.phase->r, ts.phase->g, ts.phase->b, a * 0.85f));
    }

    // Bar border
    float bh = barH * 0.5f;
    auto barBorder = LC(1.f, 1.f, 1.f, a * 0.40f);
    Line(R(-barHW, barDy - bh), R( barHW, barDy - bh), 1.f, barBorder);  // top
    Line(R(-barHW, barDy + bh), R( barHW, barDy + bh), 1.f, barBorder);  // bottom
    Line(R(-barHW, barDy - bh), R(-barHW, barDy + bh), 1.f, barBorder);  // left
    Line(R( barHW, barDy - bh), R( barHW, barDy + bh), 1.f, barBorder);  // right
}

#endif // MODLOADER_CLIENT_BUILD

// ---------------------------------------------------------------------------
//  Plugin entry points
// ---------------------------------------------------------------------------

extern "C" {

__declspec(dllexport) PluginInfo* GetPluginInfo() { return &s_info; }

__declspec(dllexport) bool PluginInit(IPluginSelf* self)
{
    g_self = self;
    LOG_INFO("RuptureTimer: init v%s", self->version);

    self->config->InitializeFromSchema(self, &k_schema);

    if (!self->config->ReadBool(self, "General", "Enabled", true)) {
        LOG_INFO("RuptureTimer: disabled by config");
        return true;
    }

    LoadDisplayConfig();

#if defined(MODLOADER_CLIENT_BUILD)
    self->hooks->Engine->RegisterOnTick(&OnTick);
    self->hooks->World->RegisterOnAnyWorldBeginPlay(&OnAnyWorldBeginPlay);

    if (self->hooks->HUD)
        self->hooks->HUD->RegisterOnPostRender(&OnPostRender);

    if (self->hooks->UI)
        self->hooks->UI->RegisterOnConfigChanged(&OnConfigChanged);

    if (self->hooks->Input)
        self->hooks->Input->RegisterKeybind(EModKey::F8, EModKeyEvent::Pressed, &OnTestKey);

    LOG_INFO("RuptureTimer: ready (F8 = cycle test phases)");
#endif

    return true;
}

__declspec(dllexport) void PluginShutdown()
{
    LOG_INFO("RuptureTimer: shutdown");
    if (!g_self) return;

#if defined(MODLOADER_CLIENT_BUILD)
    g_self->hooks->Engine->UnregisterOnTick(&OnTick);
    g_self->hooks->World->UnregisterOnAnyWorldBeginPlay(&OnAnyWorldBeginPlay);

    if (g_self->hooks->HUD)
        g_self->hooks->HUD->UnregisterOnPostRender(&OnPostRender);

    if (g_self->hooks->Input)
        g_self->hooks->Input->UnregisterKeybind(EModKey::F8, EModKeyEvent::Pressed, &OnTestKey);

    if (g_self->hooks->UI)
        g_self->hooks->UI->UnregisterOnConfigChanged(&OnConfigChanged);

    g_world = nullptr;
#endif

    g_self = nullptr;
}

} // extern "C"
