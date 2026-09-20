#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <cmath>
#include <string>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <set>

#include "Includes/Icon.h"
#include "Includes/include/Includes.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_android.h"
#include "Includes/import.h"

using namespace Tools;
#include "hooks.h"

static bool g_Initialized = false;


static ImGuiWindow* g_window = nullptr;
static ImVec4 AccentColor = ImVec4(0.0471f, 0.2353f, 0.5216f, 1.0f);
static int selectedTab = 0;

int sliderValue = 1;
int level = 0;
void* instanceBtn = nullptr;
uintptr_t il2cpp = 0, unity = 0, anogs = 0;
Vector3 MyPos;
ImVec2 DrawFrom(0,0), DrawTo(0,0);
const char* currentMap = nullptr;
ImFont* icons_font = nullptr;
bool font_load_failed = false;

std::atomic<bool> espLine{false};
std::atomic<bool> espName{false};
std::atomic<bool> espDistance{false};
std::atomic<bool> espHealth{false};
std::atomic<bool> espBox{false};
std::atomic<bool> espSleepingBody{false};
std::atomic<bool> espDrops{false};
std::atomic<bool> espArmario{false};
std::atomic<bool> espArmarioTime{false};
std::atomic<float> espMaxDistance{9999.0f};
std::atomic<float> espArmarioMaxDistance{250.0f};

// Anti-cheat ESP
std::atomic<bool> espAntiCheat{false};
std::atomic<bool> espTextShadow{false};
std::atomic<float> espTextSize{22.0f};
ImFont* esp_font = nullptr;

std::atomic<bool> armarioBypass{true};

ImVec4 espLineColor = ImVec4(0, 1, 0, 1);
ImVec4 espBoxColor = ImVec4(1, 0, 0, 1);
ImVec4 espTextColor = ImVec4(1, 1, 1, 1);
ImVec4 espHealthColor = ImVec4(0, 1, 0, 1);
ImVec4 espDistanceColor = ImVec4(1, 1, 0, 1);
ImVec4 espNameColor = ImVec4(0, 0.5f, 1, 1);
ImVec4 espArmarioColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
ImVec4 espArmarioTimeColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
std::string armarioTimeFilter;

// Anti-cheat colors
const ImVec4 HACK_LINE_COLOR = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 HACK_FLAG_COLOR = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
const ImVec4 HACK_BOX_COLOR  = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

std::mutex espMutex;
std::vector<ESPPlayerData> espPlayers;
std::atomic<int> espPlayerCount{0};
std::atomic<int> espVisibleCount{0};

std::mutex selectedPlayersMutex;
std::set<void*> selectedPlayers;

// Online players list (todos os jogadores do servidor)
std::mutex onlinePlayersMutex;
std::vector<OnlinePlayerData> onlinePlayers;
std::atomic<int> onlinePlayerCount{0};

static TimePoint lastCleanup = Clock::now();

const float PLAYER_HEIGHT_STANDING = 1.8f;
const float PLAYER_CENTER_TO_FEET_OFFSET = PLAYER_HEIGHT_STANDING / 2.0f;
const ImVec4 GOLD_COLOR = ImVec4(1.0f, 0.843f, 0.0f, 1.0f);

void SetupModernStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.FrameRounding = 6.0f;
    s.WindowRounding = 10.0f;
    s.ChildRounding = 6.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding = 6.0f;
    s.TabRounding = 6.0f;
    s.FramePadding = ImVec2(16, 12);
    s.ItemSpacing = ImVec2(14, 14);
    s.WindowPadding = ImVec2(20, 20);
    s.WindowBorderSize = 2.0f;
    s.FrameBorderSize = 2.0f;
    s.PopupBorderSize = 2.0f;

    ImVec4 bg  = ImVec4(0.05f, 0.05f, 0.08f, 1.0f);
    ImVec4 bg2 = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
    ImVec4 hov = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);
    ImVec4 act = ImVec4(0.22f, 0.22f, 0.30f, 1.0f);
    ImVec4 text = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);

    s.Colors[ImGuiCol_Text] = text;
    s.Colors[ImGuiCol_WindowBg] = bg;
    s.Colors[ImGuiCol_ChildBg] = bg;
    s.Colors[ImGuiCol_PopupBg] = bg2;
    s.Colors[ImGuiCol_FrameBg] = bg2;
    s.Colors[ImGuiCol_FrameBgHovered] = hov;
    s.Colors[ImGuiCol_FrameBgActive] = act;
    s.Colors[ImGuiCol_Button] = bg2;
    s.Colors[ImGuiCol_ButtonHovered] = hov;
    s.Colors[ImGuiCol_ButtonActive] = act;
    s.Colors[ImGuiCol_Header] = bg2;
    s.Colors[ImGuiCol_HeaderHovered] = hov;
    s.Colors[ImGuiCol_HeaderActive] = act;
    s.Colors[ImGuiCol_Separator] = bg2;
    s.Colors[ImGuiCol_SeparatorHovered] = hov;
    s.Colors[ImGuiCol_SeparatorActive] = act;
    s.Colors[ImGuiCol_Tab] = bg2;
    s.Colors[ImGuiCol_TabHovered] = hov;
    s.Colors[ImGuiCol_TabActive] = act;
    s.Colors[ImGuiCol_TabUnfocused] = bg2;
    s.Colors[ImGuiCol_TabUnfocusedActive] = hov;
    s.ScaleAllSizes(2.0f);
}

void ApplyAccentColor() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 hover = ImVec4(std::min(AccentColor.x + 0.1f, 1.0f), std::min(AccentColor.y + 0.1f, 1.0f), std::min(AccentColor.z + 0.1f, 1.0f), 1.0f);
    ImVec4 active = ImVec4(std::max(AccentColor.x - 0.1f, 0.0f), std::max(AccentColor.y - 0.1f, 0.0f), std::max(AccentColor.z - 0.1f, 0.0f), 1.0f);
    style.Colors[ImGuiCol_Button] = AccentColor;
    style.Colors[ImGuiCol_ButtonHovered] = hover;
    style.Colors[ImGuiCol_ButtonActive] = active;
    style.Colors[ImGuiCol_ResizeGrip] = AccentColor;
    style.Colors[ImGuiCol_ResizeGripHovered] = AccentColor;
    style.Colors[ImGuiCol_ResizeGripActive] = AccentColor;
    style.Colors[ImGuiCol_SliderGrab] = AccentColor;
    style.Colors[ImGuiCol_SliderGrabActive] = active;
    style.Colors[ImGuiCol_CheckMark] = AccentColor;
    style.Colors[ImGuiCol_Header] = AccentColor;
    style.Colors[ImGuiCol_HeaderHovered] = AccentColor;
    style.Colors[ImGuiCol_HeaderActive] = active;
    ImVec4 base = ImVec4(AccentColor.x * 0.20f, AccentColor.y * 0.20f, AccentColor.z * 0.20f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = base;
    style.Colors[ImGuiCol_ChildBg] = base;
    style.Colors[ImGuiCol_PopupBg] = base;
    style.Colors[ImGuiCol_TitleBg] = AccentColor;
    style.Colors[ImGuiCol_TitleBgActive] = hover;
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0471f, 0.2353f, 0.5216f, 1.0f);
}

void SetupFontAwesomeIcons() {
    if (icons_font || font_load_failed) return;
    ImGuiIO& io = ImGui::GetIO();
    float fontBaseSize = 2.0f;
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    float iconFontSize = fontBaseSize * 8.0f / 7.0f;
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    icons_config.OversampleH = 2.5f;
    icons_config.OversampleV = 2.5f;
    icons_font = io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, iconFontSize, &icons_config, icons_ranges);
    if (!icons_font) font_load_failed = true;
}

inline bool ESPWorldToScreen(const Vector3& worldPos, ImVec2& screenPos, float* outDepth = nullptr) {
    if (!Camera_main || !WorldToScreenPoint) return false;
    void* camera = Camera_main();
    if (!IsValidPtr(camera)) return false;
    Vector3 screen = WorldToScreenPoint(camera, worldPos);
    if (screen.Z < 0.1f) return false;
    if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) return false;
    screenPos.x = screen.X;
    screenPos.y = static_cast<float>(screenHeight) - screen.Y;
    if (outDepth) *outDepth = screen.Z;
    return true;
}

inline bool IsOnScreen(const ImVec2& pos, float margin = 50.0f) {
    return pos.x >= -margin && pos.x <= static_cast<float>(screenWidth) + margin &&
           pos.y >= -margin && pos.y <= static_cast<float>(screenHeight) + margin;
}

inline ImVec2 ClampToScreen(const ImVec2& pos, float margin = 10.0f) {
    return ImVec2(SafeClamp(pos.x, margin, static_cast<float>(screenWidth) - margin),
                  SafeClamp(pos.y, margin, static_cast<float>(screenHeight) - margin));
}

inline float ESPVector3Distance(const Vector3& a, const Vector3& b) {
    return std::sqrt(DistanceSqr(a, b));
}

inline void DrawRectLines(ImVec2 min, ImVec2 max, ImVec4 color) {
    ESP::DrawLine(ImVec2(min.x, min.y), ImVec2(max.x, min.y), color);
    ESP::DrawLine(ImVec2(min.x, max.y), ImVec2(max.x, max.y), color);
    ESP::DrawLine(ImVec2(min.x, min.y), ImVec2(min.x, max.y), color);
    ESP::DrawLine(ImVec2(max.x, min.y), ImVec2(max.x, max.y), color);
}

inline void DrawThickLine(ImVec2 a, ImVec2 b, ImVec4 color, int thickness = 2) {
    ESP::DrawLine(a, b, color);
    if (thickness >= 2) {
        float dx = b.x - a.x, dy = b.y - a.y;
        float len = std::sqrt(dx*dx + dy*dy);
        if (len > 0.1f) {
            float nx = -dy / len, ny = dx / len;
            for (int i = 1; i < thickness; i++) {
                float off = i * 0.8f;
                ESP::DrawLine(ImVec2(a.x + nx*off, a.y + ny*off), ImVec2(b.x + nx*off, b.y + ny*off), color);
                ESP::DrawLine(ImVec2(a.x - nx*off, a.y - ny*off), ImVec2(b.x - nx*off, b.y - ny*off), color);
            }
        }
    }
}

inline void DrawThickRectLines(ImVec2 min, ImVec2 max, ImVec4 color, int thickness = 2) {
    DrawThickLine(ImVec2(min.x, min.y), ImVec2(max.x, min.y), color, thickness);
    DrawThickLine(ImVec2(min.x, max.y), ImVec2(max.x, max.y), color, thickness);
    DrawThickLine(ImVec2(min.x, min.y), ImVec2(min.x, max.y), color, thickness);
    DrawThickLine(ImVec2(max.x, min.y), ImVec2(max.x, max.y), color, thickness);
}

// Mede o texto com a FONTE DO ESP (nao a fonte gigante do menu)
inline ImVec2 ESPTextSize(const char* text) {
    if (!text) return ImVec2(0, 0);
    if (esp_font) return esp_font->CalcTextSizeA(espTextSize.load(std::memory_order_relaxed), 8192.0f, -1.0f, text);
    return ImGui::CalcTextSize(text);
}

// Texto do ESP com fonte propria e tamanho controlavel (slider no menu)
inline void DrawESPText(const ImVec2& pos, const ImVec4& color, const char* text) {
    if (!text) return;
    float sz = espTextSize.load(std::memory_order_relaxed);
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    if (esp_font) {
        if (espTextShadow.load(std::memory_order_relaxed)) {
            dl->AddText(esp_font, sz, ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 255), text);
        }
        dl->AddText(esp_font, sz, pos, ImGui::ColorConvertFloat4ToU32(color), text);
    } else {
        if (espTextShadow.load(std::memory_order_relaxed)) {
            ESP::DrawText(ImVec2(pos.x + 1.0f, pos.y + 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 1.0f), text);
        }
        ESP::DrawText(pos, color, text);
    }
}

// Nome com fundo escuro = maxima legibilidade
inline void DrawESPTextBg(const ImVec2& pos, const ImVec4& color, const char* text) {
    if (!text) return;
    ImVec2 size = ESPTextSize(text);
    ImGui::GetForegroundDrawList()->AddRectFilled(
        ImVec2(pos.x - 3.0f, pos.y - 2.0f),
        ImVec2(pos.x + size.x + 3.0f, pos.y + size.y + 2.0f),
        IM_COL32(0, 0, 0, 150));
    DrawESPText(pos, color, text);
}

void DrawArmariosNormalESP() {
    bool drawArmario = espArmario.load(std::memory_order_relaxed);
    if (!drawArmario || !Camera_main || !WorldToScreenPoint) return;
    if (screenWidth <= 0 || screenHeight <= 0) return;

    CleanupArmarios();

    Vector3 localPos = MyPos;
    bool hasLocalPos = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f);
    float maxDist = espArmarioMaxDistance.load(std::memory_order_relaxed);
    float maxDistSqr = maxDist * maxDist;
    const ImVec4 armarioBoxColor = espArmarioColor;
    const ImVec4 armarioTextColor = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);
    char distBuffer[32];
    int drawnCount = 0;

    std::vector<ArmarioData> localCopy;
    {
        std::lock_guard<std::mutex> lock(armariosMutex);
        localCopy.reserve(armariosList.size());
        for (auto& a : armariosList) {
            if (!IsUnityObjectAlive(a.instance)) continue;
            if (!a.hasValidPosition) {
                void* transform = GetTransformFromComponent(a.instance);
                if (IsValidPtr(transform) && get_position) {
                    Vector3 rawPos = get_position(transform);
                    if (std::isfinite(rawPos.X) && std::isfinite(rawPos.Y) && std::isfinite(rawPos.Z)) {
                        a.pos = rawPos;
                        a.hasValidPosition = true;
                    }
                }
            }
            if (!a.hasValidPosition || (a.pos.X == 0.0f && a.pos.Y == 0.0f && a.pos.Z == 0.0f)) continue;
            if (hasLocalPos) {
                float distSqr = DistanceSqr(localPos, a.pos);
                if (distSqr > maxDistSqr) continue;
                a.distance = std::sqrt(distSqr);
            }
            localCopy.push_back(a);
        }
    }

    if (hasLocalPos && localCopy.size() > 15) {
        std::partial_sort(localCopy.begin(), localCopy.begin() + 15, localCopy.end(),
            [](const ArmarioData& a, const ArmarioData& b) { return a.distance < b.distance; });
        localCopy.resize(15);
    }

    for (auto& armario : localCopy) {
        ImVec2 screenPos;
        void* camera = Camera_main();
        if (!IsValidPtr(camera)) continue;
        Vector3 screen = WorldToScreenPoint(camera, armario.pos);
        if (screen.Z < 0.01f) continue;
        if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) continue;
        screenPos.x = screen.X;
        screenPos.y = static_cast<float>(screenHeight) - screen.Y;
        if (!IsOnScreen(screenPos)) continue;
        drawnCount++;

        {
            std::lock_guard<std::mutex> lock(armariosMutex);
            auto it = std::find_if(armariosList.begin(), armariosList.end(),
                [&armario](const ArmarioData& a) { return a.instance == armario.instance; });
            if (it != armariosList.end()) {
                it->lastUpdate = Now();
            }
        }

        float boxSize = 28.0f;
        ImVec2 boxMin(screenPos.x - boxSize / 2.0f, screenPos.y - boxSize / 2.0f);
        ImVec2 boxMax(screenPos.x + boxSize / 2.0f, screenPos.y + boxSize / 2.0f);
        DrawThickRectLines(boxMin, boxMax, armarioBoxColor, 3);

        int authCount = armario.authorizedCount;
        if (authCount < 1) authCount = 1;
        char nameWithAuth[128];
        snprintf(nameWithAuth, sizeof(nameWithAuth), "%s [%d auth]", armario.donoNome.c_str(), authCount);
        const char* displayName = nameWithAuth;
        ImVec2 textSize = ESPTextSize(displayName);
        float nameY = boxMin.y - textSize.y - 8.0f - 20.0f;
        ImVec2 textPos(screenPos.x - textSize.x / 2.0f, nameY);
        textPos = ClampToScreen(textPos, 5.0f);
        ImVec4 authColor = (authCount >= 3) ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        DrawESPTextBg(textPos, authColor, displayName);

        if (armario.distance > 0.0f) {
            snprintf(distBuffer, sizeof(distBuffer), "[%.0fm]", armario.distance);
            ImVec2 distSize = ESPTextSize(distBuffer);
            ImVec2 distPos(screenPos.x - distSize.x / 2.0f, boxMax.y + 6.0f + 20.0f);
            distPos = ClampToScreen(distPos, 5.0f);
            DrawESPText(distPos, armarioTextColor, distBuffer);
        }
    }

    if (drawnCount > 0) {
        char debugBuf[64];
        snprintf(debugBuf, sizeof(debugBuf), "Armarios: %d/15 (max %.0fm)", drawnCount, maxDist);
        ImVec2 dbgSize = ImGui::CalcTextSize(debugBuf);
        DrawESPText(ImVec2(20.0f, static_cast<float>(screenHeight) - 60.0f), ImVec4(1.0f, 0.5f, 0.0f, 1.0f), debugBuf);
    }
}

void DrawFovCircle() {
    bool asa = aimbotSempreAcerta.load(std::memory_order_relaxed);
    bool mb = magicBullet.load(std::memory_order_relaxed);
    if (!asa && !mb) return;
    float fovRadius = aimbotFov.load(std::memory_order_relaxed);
    if (fovRadius < 10.0f) fovRadius = 10.0f;
    float cx = static_cast<float>(screenWidth) * 0.5f;
    float cy = static_cast<float>(screenHeight) * 0.5f;
    ImVec4 circleColor = ImVec4(1.0f, 0.0f, 1.0f, 0.6f);
    int segments = 64;
    float step = 6.28318530718f / segments;
    for (int i = 0; i < segments; i++) {
        float angle1 = i * step;
        float angle2 = (i + 1) * step;
        ImVec2 p1(cx + std::cos(angle1) * fovRadius, cy + std::sin(angle1) * fovRadius);
        ImVec2 p2(cx + std::cos(angle2) * fovRadius, cy + std::sin(angle2) * fovRadius);
        ESP::DrawLine(p1, p2, circleColor);
    }
    float crossLen = 8.0f;
    ImVec4 crossColor = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
    ESP::DrawLine(ImVec2(cx - crossLen, cy), ImVec2(cx + crossLen, cy), crossColor);
    ESP::DrawLine(ImVec2(cx, cy - crossLen), ImVec2(cx, cy + crossLen), crossColor);
}

void DrawMagicBulletLine() {
    bool asa = aimbotSempreAcerta.load(std::memory_order_relaxed);
    bool mb = magicBullet.load(std::memory_order_relaxed);
    if (!asa && !mb) return;
    if (!Camera_main || !WorldToScreenPoint) return;
    void* camera = Camera_main();
    if (!IsValidPtr(camera)) return;
    float cx = static_cast<float>(screenWidth) * 0.5f;
    float cy = static_cast<float>(screenHeight) * 0.5f;

    if (asa && g_aimbotTarget && IsIl2CppObjectValid(g_aimbotTarget)) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Now() - g_aimbotTargetTime).count();
        if (elapsed < 1000) {
            Vector3 w2sAim = WorldToScreenPoint(camera, g_aimbotTargetPos);
            if (w2sAim.Z >= 0.1f && std::isfinite(w2sAim.X) && std::isfinite(w2sAim.Y)) {
                ImVec2 aimScreen(w2sAim.X, static_cast<float>(screenHeight) - w2sAim.Y);
                ImVec4 aimLineColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
                DrawThickLine(ImVec2(cx, cy), aimScreen, aimLineColor, 3);
                ESP::DrawLine(ImVec2(aimScreen.x - 6, aimScreen.y - 6), ImVec2(aimScreen.x + 6, aimScreen.y + 6), aimLineColor);
                ESP::DrawLine(ImVec2(aimScreen.x + 6, aimScreen.y - 6), ImVec2(aimScreen.x - 6, aimScreen.y + 6), aimLineColor);
            }
        }
    }

    if (mb && g_magicBulletTarget && IsIl2CppObjectValid(g_magicBulletTarget)) {
        Vector3 w2sTarget = WorldToScreenPoint(camera, g_magicBulletTargetPos);
        if (w2sTarget.Z >= 0.1f && std::isfinite(w2sTarget.X) && std::isfinite(w2sTarget.Y)) {
            ImVec2 targetScreen(w2sTarget.X, static_cast<float>(screenHeight) - w2sTarget.Y);
            Vector3 originPos = g_magicBulletOrigin;
            if (originPos.X == 0.0f && originPos.Y == 0.0f && originPos.Z == 0.0f) {
                originPos = MyPos;
                originPos.Y += 1.5f;
            }
            Vector3 w2sOrigin = WorldToScreenPoint(camera, originPos);
            if (w2sOrigin.Z >= 0.1f && std::isfinite(w2sOrigin.X) && std::isfinite(w2sOrigin.Y)) {
                ImVec2 originScreen(w2sOrigin.X, static_cast<float>(screenHeight) - w2sOrigin.Y);
                ImVec4 bulletLineColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
                ESP::DrawLine(ImVec2(targetScreen.x - 5, targetScreen.y - 5), ImVec2(targetScreen.x + 5, targetScreen.y - 5), bulletLineColor);
                ESP::DrawLine(ImVec2(targetScreen.x + 5, targetScreen.y - 5), ImVec2(targetScreen.x + 5, targetScreen.y + 5), bulletLineColor);
                ESP::DrawLine(ImVec2(targetScreen.x + 5, targetScreen.y + 5), ImVec2(targetScreen.x - 5, targetScreen.y + 5), bulletLineColor);
                ESP::DrawLine(ImVec2(targetScreen.x - 5, targetScreen.y + 5), ImVec2(targetScreen.x - 5, targetScreen.y - 5), bulletLineColor);
            }
        }
    }
}

void DrawRecursosESP() {
    bool anyEnabled = espMadeira.load(std::memory_order_relaxed) ||
                      espPedra.load(std::memory_order_relaxed) ||
                      espMetal.load(std::memory_order_relaxed) ||
                      espEnxofre.load(std::memory_order_relaxed) ||
                      espRecursosGeral.load(std::memory_order_relaxed);
    if (!anyEnabled || !Camera_main || !WorldToScreenPoint) return;
    if (screenWidth <= 0 || screenHeight <= 0) return;

    CleanupRecursos();

    Vector3 localPos = MyPos;
    bool hasLocalPos = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f);
    float maxDist = espRecursosMaxDist.load(std::memory_order_relaxed);
    float maxDistSqr = maxDist * maxDist;
    char distBuffer[32];
    int drawnCount = 0;

    std::vector<RecursoData> localCopy;
    {
        std::lock_guard<std::mutex> lock(recursosMutex);
        localCopy.reserve(recursosList.size());
        for (auto& r : recursosList) {
            if (!IsUnityObjectAlive(r.instance)) continue;
            if (r.jaQuebrou || !r.ativo) continue;
            if (!ShouldShowRecurso(r.nome)) continue;
            if (hasLocalPos) {
                float distSqr = DistanceSqr(localPos, r.pos);
                if (distSqr > maxDistSqr) continue;
            }
            localCopy.push_back(r);
        }
    }

    if (hasLocalPos && localCopy.size() > 30) {
        std::partial_sort(localCopy.begin(), localCopy.begin() + 30, localCopy.end(),
            [&localPos](const RecursoData& a, const RecursoData& b) {
                return DistanceSqr(localPos, a.pos) < DistanceSqr(localPos, b.pos);
            });
        localCopy.resize(30);
    }

    for (auto& recurso : localCopy) {
        ImVec2 screenPos;
        void* camera = Camera_main();
        if (!IsValidPtr(camera)) continue;
        Vector3 screen = WorldToScreenPoint(camera, recurso.pos);
        if (screen.Z < 0.01f) continue;
        if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) continue;
        screenPos.x = screen.X;
        screenPos.y = static_cast<float>(screenHeight) - screen.Y;
        if (!IsOnScreen(screenPos)) continue;
        drawnCount++;

        ImVec4 color = GetRecursoColor(recurso.nome);
        float boxSize = 24.0f;
        ImVec2 boxMin(screenPos.x - boxSize / 2.0f, screenPos.y - boxSize / 2.0f);
        ImVec2 boxMax(screenPos.x + boxSize / 2.0f, screenPos.y + boxSize / 2.0f);
        DrawThickRectLines(boxMin, boxMax, color, 2);

        const char* displayName = recurso.nome.c_str();
        char nameWithQty[128];
        if (recurso.quantidade > 0.0f) {
            snprintf(nameWithQty, sizeof(nameWithQty), "%s [%.0f%%]", recurso.nome.c_str(), recurso.quantidade);
            displayName = nameWithQty;
        }
        ImVec2 textSize = ESPTextSize(displayName);
        float nameY = boxMin.y - textSize.y - 6.0f;
        ImVec2 textPos(screenPos.x - textSize.x / 2.0f, nameY);
        textPos = ClampToScreen(textPos, 5.0f);
        DrawESPTextBg(textPos, color, displayName);

        if (hasLocalPos) {
            float dist = std::sqrt(DistanceSqr(localPos, recurso.pos));
            snprintf(distBuffer, sizeof(distBuffer), "[%.0fm]", dist);
            ImVec2 distSize = ESPTextSize(distBuffer);
            ImVec2 distPos(screenPos.x - distSize.x / 2.0f, boxMax.y + 4.0f);
            distPos = ClampToScreen(distPos, 5.0f);
            DrawESPText(distPos, color, distBuffer);
        }
    }

    if (drawnCount > 0) {
        char debugBuf[64];
        snprintf(debugBuf, sizeof(debugBuf), "Recursos: %d/30 (max %.0fm)", drawnCount, maxDist);
        ImVec2 dbgSize = ImGui::CalcTextSize(debugBuf);
        DrawESPText(ImVec2(20.0f, static_cast<float>(screenHeight) - 80.0f), ImVec4(0.0f, 1.0f, 0.5f, 1.0f), debugBuf);
    }
}

void DrawTorretasESP() {
    bool enabled = espTorretas.load(std::memory_order_relaxed);
    if (!enabled || !Camera_main || !WorldToScreenPoint) return;
    if (screenWidth <= 0 || screenHeight <= 0) return;

    CleanupTorretas();

    Vector3 localPos = MyPos;
    bool hasLocalPos = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f);
    float maxDist = espTorretasMaxDist.load(std::memory_order_relaxed);
    float maxDistSqr = maxDist * maxDist;
    int drawnCount = 0;

    std::vector<TorretaData> localCopy;
    {
        std::lock_guard<std::mutex> lock(torretasMutex);
        localCopy.reserve(torretasList.size());
        for (auto& t : torretasList) {
            if (!IsUnityObjectAlive(t.instance)) continue;
            if (hasLocalPos) {
                float distSqr = DistanceSqr(localPos, t.pos);
                if (distSqr > maxDistSqr) continue;
            }
            localCopy.push_back(t);
        }
    }

    if (hasLocalPos && localCopy.size() > 20) {
        std::partial_sort(localCopy.begin(), localCopy.begin() + 20, localCopy.end(),
            [&localPos](const TorretaData& a, const TorretaData& b) {
                return DistanceSqr(localPos, a.pos) < DistanceSqr(localPos, b.pos);
            });
        localCopy.resize(20);
    }

    for (auto& torreta : localCopy) {
        ImVec2 screenPos;
        void* camera = Camera_main();
        if (!IsValidPtr(camera)) continue;
        Vector3 screen = WorldToScreenPoint(camera, torreta.pos);
        if (screen.Z < 0.01f) continue;
        if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) continue;
        screenPos.x = screen.X;
        screenPos.y = static_cast<float>(screenHeight) - screen.Y;
        if (!IsOnScreen(screenPos)) continue;
        drawnCount++;

        ImVec4 color = torreta.ligada ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        float boxSize = 28.0f;
        ImVec2 boxMin(screenPos.x - boxSize / 2.0f, screenPos.y - boxSize / 2.0f);
        ImVec2 boxMax(screenPos.x + boxSize / 2.0f, screenPos.y + boxSize / 2.0f);
        DrawThickRectLines(boxMin, boxMax, color, 3);

        char nameBuffer[128];
        const char* displayName = torreta.donoNome.c_str();
        if (torreta.donoNome.length() > 24) {
            snprintf(nameBuffer, sizeof(nameBuffer), "%.21s...", displayName);
            displayName = nameBuffer;
        }

        char statusText[64];
        if (torreta.semMunicao) {
            snprintf(statusText, sizeof(statusText), "%s [SEM MUNICAO]", displayName);
        } else {
            snprintf(statusText, sizeof(statusText), "%s [%s]", displayName, torreta.ligada ? "LIGADA" : "DESLIGADA");
        }

        ImVec2 textSize = ESPTextSize(statusText);
        float nameY = boxMin.y - textSize.y - 6.0f;
        ImVec2 textPos(screenPos.x - textSize.x / 2.0f, nameY);
        textPos = ClampToScreen(textPos, 5.0f);
        DrawESPTextBg(textPos, color, statusText);

        if (hasLocalPos) {
            float dist = std::sqrt(DistanceSqr(localPos, torreta.pos));
            char distBuffer[32];
            snprintf(distBuffer, sizeof(distBuffer), "[%.0fm]", dist);
            ImVec2 distSize = ESPTextSize(distBuffer);
            ImVec2 distPos(screenPos.x - distSize.x / 2.0f, boxMax.y + 4.0f);
            distPos = ClampToScreen(distPos, 5.0f);
            DrawESPText(distPos, color, distBuffer);
        }
    }

    if (drawnCount > 0) {
        char debugBuf[64];
        snprintf(debugBuf, sizeof(debugBuf), "Torretas: %d/20 (max %.0fm)", drawnCount, maxDist);
        ImVec2 dbgSize = ImGui::CalcTextSize(debugBuf);
        DrawESPText(ImVec2(20.0f, static_cast<float>(screenHeight) - 100.0f), ImVec4(1.0f, 0.0f, 1.0f, 1.0f), debugBuf);
    }
}

void DrawLootESP() {
    bool enabled = espDrops.load(std::memory_order_relaxed);
    if (!enabled || !Camera_main || !WorldToScreenPoint) return;
    if (screenWidth <= 0 || screenHeight <= 0) return;

    CleanupLoot();

    Vector3 localPos = MyPos;
    bool hasLocalPos = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f);
    float maxDist = espMaxDistance.load(std::memory_order_relaxed);
    float maxDistSqr = maxDist * maxDist;
    char distBuffer[32];
    int drawnCount = 0;

    std::vector<LootData> localCopy;
    {
        std::lock_guard<std::mutex> lock(lootMutex);
        localCopy.reserve(lootList.size());
        for (auto& l : lootList) {
            if (!IsUnityObjectAlive(l.instance)) continue;
            if (hasLocalPos) {
                float distSqr = DistanceSqr(localPos, l.pos);
                if (distSqr > maxDistSqr) continue;
            }
            localCopy.push_back(l);
        }
    }

    if (hasLocalPos && localCopy.size() > 30) {
        std::partial_sort(localCopy.begin(), localCopy.begin() + 30, localCopy.end(),
            [&localPos](const LootData& a, const LootData& b) {
                return DistanceSqr(localPos, a.pos) < DistanceSqr(localPos, b.pos);
            });
        localCopy.resize(30);
    }

    ImVec4 lootColor = ImVec4(0.8f, 0.4f, 1.0f, 1.0f);

    for (auto& loot : localCopy) {
        ImVec2 screenPos;
        void* camera = Camera_main();
        if (!IsValidPtr(camera)) continue;
        Vector3 screen = WorldToScreenPoint(camera, loot.pos);
        if (screen.Z < 0.01f) continue;
        if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) continue;
        screenPos.x = screen.X;
        screenPos.y = static_cast<float>(screenHeight) - screen.Y;
        if (!IsOnScreen(screenPos)) continue;
        drawnCount++;

        float boxSize = 20.0f;
        ImVec2 boxMin(screenPos.x - boxSize / 2.0f, screenPos.y - boxSize / 2.0f);
        ImVec2 boxMax(screenPos.x + boxSize / 2.0f, screenPos.y + boxSize / 2.0f);
        DrawThickRectLines(boxMin, boxMax, lootColor, 2);

        char qtyName[192];
        if (loot.quantidade > 1)
            snprintf(qtyName, sizeof(qtyName), "%s x%d", loot.nome.c_str(), loot.quantidade);
        else
            snprintf(qtyName, sizeof(qtyName), "%s", loot.nome.c_str());
        const char* displayName = qtyName;
        ImVec2 textSize = ESPTextSize(displayName);
        float nameY = boxMin.y - textSize.y - 6.0f;
        ImVec2 textPos(screenPos.x - textSize.x / 2.0f, nameY);
        textPos = ClampToScreen(textPos, 5.0f);
        DrawESPTextBg(textPos, lootColor, displayName);

        if (hasLocalPos) {
            float dist = std::sqrt(DistanceSqr(localPos, loot.pos));
            snprintf(distBuffer, sizeof(distBuffer), "[%.0fm]", dist);
            ImVec2 distSize = ESPTextSize(distBuffer);
            ImVec2 distPos(screenPos.x - distSize.x / 2.0f, boxMax.y + 4.0f);
            distPos = ClampToScreen(distPos, 5.0f);
            DrawESPText(distPos, lootColor, distBuffer);
        }
    }

    if (drawnCount > 0) {
        char debugBuf[64];
        snprintf(debugBuf, sizeof(debugBuf), "Loot: %d/30", drawnCount);
        ImVec2 dbgSize = ImGui::CalcTextSize(debugBuf);
        DrawESPText(ImVec2(20.0f, static_cast<float>(screenHeight) - 120.0f), lootColor, debugBuf);
    }
}

void DrawSleepingBodiesESP() {
    bool enabled = espSleepingBody.load(std::memory_order_relaxed);
    if (!enabled || !Camera_main || !WorldToScreenPoint) return;
    if (screenWidth <= 0 || screenHeight <= 0) return;

    CleanupSleepingBodies();

    // Scan de corpos ja existentes (classe no namespace Decay.Persistence)
    static TimePoint lastSBSceneScan;
    if (ShouldUpdate(lastSBSceneScan, 3000)) {
        RefreshSleepingBodiesFromScene();
    }

    Vector3 localPos = MyPos;
    bool hasLocalPos = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f);
    float maxDist = espMaxDistance.load(std::memory_order_relaxed);
    float maxDistSqr = maxDist * maxDist;

    std::vector<SleepingBodyData> localCopy;
    {
        std::lock_guard<std::mutex> lock(sleepingBodiesMutex);
        localCopy.reserve(sleepingBodiesList.size());
        for (auto& s : sleepingBodiesList) {
            if (!IsUnityObjectAlive(s.instance)) continue;
            if (hasLocalPos) {
                float distSqr = DistanceSqr(localPos, s.pos);
                if (distSqr > maxDistSqr) continue;
            }
            localCopy.push_back(s);
        }
    }

    const ImVec4 bodyColor = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);
    for (auto& sb : localCopy) {
        ImVec2 screenPos;
        void* camera = Camera_main();
        if (!IsValidPtr(camera)) continue;
        Vector3 screen = WorldToScreenPoint(camera, sb.pos);
        if (screen.Z < 0.01f) continue;
        if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) continue;
        screenPos.x = screen.X;
        screenPos.y = static_cast<float>(screenHeight) - screen.Y;
        if (!IsOnScreen(screenPos)) continue;

        float boxSize = 26.0f;
        ImVec2 boxMin(screenPos.x - boxSize / 2.0f, screenPos.y - boxSize / 2.0f);
        ImVec2 boxMax(screenPos.x + boxSize / 2.0f, screenPos.y + boxSize / 2.0f);
        DrawThickRectLines(boxMin, boxMax, bodyColor, 2);

        char label[160];
        int hp = static_cast<int>(sb.health);
        if (hp < 0) hp = 0;
        if (hp > 100) hp = 100;
        const char* statusTag = sb.morto ? "[MORTO-LOOT]" : "[DORMINDO]";
        if (hasLocalPos) {
            float dist = std::sqrt(DistanceSqr(localPos, sb.pos));
            snprintf(label, sizeof(label), "%s %s HP:%d%% [%.0fm]", sb.ownerName.c_str(), statusTag, hp, dist);
        } else {
            snprintf(label, sizeof(label), "%s %s HP:%d%%", sb.ownerName.c_str(), statusTag, hp);
        }
        ImVec2 textSize = ESPTextSize(label);
        ImVec2 textPos(screenPos.x - textSize.x / 2.0f, boxMin.y - textSize.y - 6.0f);
        textPos = ClampToScreen(textPos, 5.0f);
        DrawESPTextBg(textPos, bodyColor, label);
    }
}

void DrawCamasESP() {
    bool enabled = espCamas.load(std::memory_order_relaxed);
    if (!enabled) return;
    // Scan independente: nao depende do ESP de jogador nem do bloco de cleanup
    static TimePoint lastCamaScan;
    if (ShouldUpdate(lastCamaScan, 2000)) {
        RefreshSacosFromScene();   // camas como objetos na cena
        RefreshSacosFromPlayers(); // camas via sacosPlayerJson (SyncVar)
    }
    if (!Camera_main || !WorldToScreenPoint) return;
    if (screenWidth <= 0 || screenHeight <= 0) return;

    Vector3 localPos = MyPos;
    bool hasLocalPos = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f);
    float maxDist = espCamasMaxDist.load(std::memory_order_relaxed);
    float maxDistSqr = maxDist * maxDist;
    const ImVec4 camaColor = ImVec4(1.0f, 0.4f, 0.75f, 1.0f); // rosa
    char distBuffer[32];
    int drawnCount = 0;

    std::vector<SacodeDormirData> localCopy;
    {
        std::lock_guard<std::mutex> lock(sacosMutex);
        localCopy.reserve(sacosList.size());
        for (auto& c : sacosList) {
            if (!IsUnityObjectAlive(c.instance)) continue;
            if (!c.hasPos) {
                void* transform = GetTransformFromComponent(c.instance);
                if (IsValidPtr(transform) && get_position) {
                    Vector3 rawPos = get_position(transform);
                    if (std::isfinite(rawPos.X) && std::isfinite(rawPos.Y) && std::isfinite(rawPos.Z)) {
                        c.pos = rawPos;
                        c.hasPos = true;
                    }
                }
            }
            if (!c.hasPos) continue;
            if (hasLocalPos) {
                float distSqr = DistanceSqr(localPos, c.pos);
                if (distSqr > maxDistSqr) continue;
            }
            localCopy.push_back(c);
        }
    }

    if (hasLocalPos && localCopy.size() > 20) {
        std::partial_sort(localCopy.begin(), localCopy.begin() + 20, localCopy.end(),
            [&localPos](const SacodeDormirData& a, const SacodeDormirData& b) {
                return DistanceSqr(localPos, a.pos) < DistanceSqr(localPos, b.pos);
            });
        localCopy.resize(20);
    }

    for (auto& cama : localCopy) {
        ImVec2 screenPos;
        void* camera = Camera_main();
        if (!IsValidPtr(camera)) continue;
        Vector3 screen = WorldToScreenPoint(camera, cama.pos);
        if (screen.Z < 0.01f) continue;
        if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) continue;
        screenPos.x = screen.X;
        screenPos.y = static_cast<float>(screenHeight) - screen.Y;
        if (!IsOnScreen(screenPos)) continue;
        drawnCount++;

        float boxSize = 26.0f;
        ImVec2 boxMin(screenPos.x - boxSize / 2.0f, screenPos.y - boxSize / 2.0f);
        ImVec2 boxMax(screenPos.x + boxSize / 2.0f, screenPos.y + boxSize / 2.0f);
        DrawThickRectLines(boxMin, boxMax, camaColor, 2);

        // Resolve o nick NA HORA DO DESENHO: playerDono -> ID online -> json (worldId) -> ID -> "?"
        std::string donoStr = cama.donoNome;
        if (donoStr.empty()) donoStr = ResolveDonoNomeById(cama.donoPlayerID);
        if (donoStr.empty() && !cama.worldId.empty()) {
            std::lock_guard<std::mutex> jLock(sacosJsonMutex);
            for (auto& sj : sacosJsonList) {
                if (sj.worldId == cama.worldId && !sj.donoNome.empty()) {
                    donoStr = sj.donoNome;
                    break;
                }
            }
        }
        // Match por posicao: a entrada json e o saco na cena sao o mesmo
        // objeto fisico (mesma posicao). Funciona mesmo sem worldId.
        if (donoStr.empty() && cama.hasPos) {
            std::lock_guard<std::mutex> jLock(sacosJsonMutex);
            for (auto& sj : sacosJsonList) {
                if (sj.donoNome.empty()) continue;
                if (DistanceSqr(sj.pos, cama.pos) < 4.0f) { // ~2m de tolerancia
                    donoStr = sj.donoNome;
                    break;
                }
            }
        }
        if (donoStr.empty())
            donoStr = cama.donoPlayerID.empty() ? "?" : cama.donoPlayerID;
        const char* dono = donoStr.c_str();

        char nameBuffer[160];
        const char* donoDisplay = dono;
        char donoClipped[64];
        if (strlen(dono) > 20) {
            snprintf(donoClipped, sizeof(donoClipped), "%.17s...", dono);
            donoDisplay = donoClipped;
        }
        snprintf(nameBuffer, sizeof(nameBuffer), "%s | dono: %s", cama.nome.c_str(), donoDisplay);

        ImVec2 textSize = ESPTextSize(nameBuffer);
        float nameY = boxMin.y - textSize.y - 6.0f;
        ImVec2 textPos(screenPos.x - textSize.x / 2.0f, nameY);
        textPos = ClampToScreen(textPos, 5.0f);
        DrawESPTextBg(textPos, camaColor, nameBuffer);

        if (hasLocalPos) {
            float dist = std::sqrt(DistanceSqr(localPos, cama.pos));
            snprintf(distBuffer, sizeof(distBuffer), "[%.0fm]", dist);
            ImVec2 distSize = ESPTextSize(distBuffer);
            ImVec2 distPos(screenPos.x - distSize.x / 2.0f, boxMax.y + 4.0f);
            distPos = ClampToScreen(distPos, 5.0f);
            DrawESPText(distPos, camaColor, distBuffer);
        }
    }

    // Camas obtidas via sacosPlayerJson (SyncVar dos PlayerStatus) - sem objeto na cena
    {
        std::vector<SacodeDormirJsonData> jsonCopy;
        {
            std::lock_guard<std::mutex> lock(sacosJsonMutex);
            jsonCopy.reserve(sacosJsonList.size());
            for (auto& s : sacosJsonList) {
                if (hasLocalPos && DistanceSqr(localPos, s.pos) > maxDistSqr) continue;
                jsonCopy.push_back(s);
            }
        }
        if (hasLocalPos && jsonCopy.size() > 15) {
            std::partial_sort(jsonCopy.begin(), jsonCopy.begin() + 15, jsonCopy.end(),
                [&localPos](const SacodeDormirJsonData& a, const SacodeDormirJsonData& b) {
                    return DistanceSqr(localPos, a.pos) < DistanceSqr(localPos, b.pos);
                });
            jsonCopy.resize(15);
        }
        for (auto& cama : jsonCopy) {
            ImVec2 screenPos;
            void* camera = Camera_main();
            if (!IsValidPtr(camera)) continue;
            Vector3 screen = WorldToScreenPoint(camera, cama.pos);
            if (screen.Z < 0.01f) continue;
            if (!std::isfinite(screen.X) || !std::isfinite(screen.Y) || !std::isfinite(screen.Z)) continue;
            screenPos.x = screen.X;
            screenPos.y = static_cast<float>(screenHeight) - screen.Y;
            if (!IsOnScreen(screenPos)) continue;
            drawnCount++;

            float boxSize = 26.0f;
            ImVec2 boxMin(screenPos.x - boxSize / 2.0f, screenPos.y - boxSize / 2.0f);
            ImVec2 boxMax(screenPos.x + boxSize / 2.0f, screenPos.y + boxSize / 2.0f);
            DrawThickRectLines(boxMin, boxMax, camaColor, 2);

            char nameBuffer[192];
            const char* dono = cama.donoNome.empty() ? "?" : cama.donoNome.c_str();
            snprintf(nameBuffer, sizeof(nameBuffer), "%s | dono: %.20s", cama.nome.c_str(), dono);
            ImVec2 textSize = ESPTextSize(nameBuffer);
            ImVec2 textPos(screenPos.x - textSize.x / 2.0f, boxMin.y - textSize.y - 6.0f);
            textPos = ClampToScreen(textPos, 5.0f);
            DrawESPTextBg(textPos, camaColor, nameBuffer);

            if (hasLocalPos) {
                float dist = std::sqrt(DistanceSqr(localPos, cama.pos));
                snprintf(distBuffer, sizeof(distBuffer), "[%.0fm]", dist);
                ImVec2 distSize = ESPTextSize(distBuffer);
                ImVec2 distPos(screenPos.x - distSize.x / 2.0f, boxMax.y + 4.0f);
                distPos = ClampToScreen(distPos, 5.0f);
                DrawESPText(distPos, camaColor, distBuffer);
            }
        }
    }

    if (drawnCount > 0) {
        char debugBuf[64];
        snprintf(debugBuf, sizeof(debugBuf), "Camas: %d (max %.0fm)", drawnCount, maxDist);
        DrawESPText(ImVec2(20.0f, static_cast<float>(screenHeight) - 140.0f), camaColor, debugBuf);
    }
}

void DrawESP() {
    bool hasESP = espLine.load(std::memory_order_relaxed) ||
                  espName.load(std::memory_order_relaxed) ||
                  espDistance.load(std::memory_order_relaxed) ||
                  espHealth.load(std::memory_order_relaxed) ||
                  espBox.load(std::memory_order_relaxed);
    if (!Camera_main || !WorldToScreenPoint) return;

    auto now = Clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastCleanup).count() >= 2) {
        CleanupInvalidPlayers();
        CleanupOnlinePlayers();
        CleanupSleepingBodies();
        RefreshOnlinePlayersFromFPSControllers(); // Scan global de todos os FPSController na cena
        RefreshOnlinePlayersFromGOM();        // Scan global de todos os PlayerStatus na cena
        RefreshOnlinePlayersFromTeamMarkers(); // Scan via PlayerMapController (jogadores do time, qualquer distancia)
        RefreshSacosFromScene();               // Scan de sacos de dormir na cena
        GetPersistenceService();               // Aquece o cache do PlayerPersistenceService (cliente)
        lastCleanup = now;
    }

    int visiblePlayers = 0;
    const ImVec4 countColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    if (hasESP) {
    Vector3 localPos = MyPos;
    bool hasLocalPos = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f);
    float maxDist = espMaxDistance.load(std::memory_order_relaxed);
    float maxDistSqr = maxDist * maxDist;

    const ImVec4 defaultBoxColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    const ImVec4 defaultNameColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f);
    const ImVec4 defaultDistColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    const ImVec4 defaultLineColor = ImVec4(0.0f, 1.0f, 0.0f, 0.8f);
    const ImVec4 healthColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    const ImVec4 barBg = ImVec4(0.1f, 0.1f, 0.1f, 0.8f);
    const ImVec2 screenTop(static_cast<float>(screenWidth) / 2.0f, 0.0f);

    const bool drawLine = espLine.load(std::memory_order_relaxed);
    const bool drawBox = espBox.load(std::memory_order_relaxed);
    const bool drawName = espName.load(std::memory_order_relaxed);
    const bool drawDist = espDistance.load(std::memory_order_relaxed);
    const bool drawHealth = espHealth.load(std::memory_order_relaxed);
    const bool antiCheatOn = espAntiCheat.load(std::memory_order_relaxed);
    char distBuffer[32];
    char nameBuffer[128];

    std::vector<ESPPlayerData> localCopy;
    std::set<void*> localSelected;
    {
        std::lock_guard<std::mutex> lock(espMutex);
        std::lock_guard<std::mutex> selLock(selectedPlayersMutex);
        localCopy.reserve(espPlayers.size());
        localSelected = selectedPlayers;

        for (auto& p : espPlayers) {
            if (p.hasValidPosition && !p.isDead && !p.isLocal && IsIl2CppObjectValid(p.instance)) {
                bool isSelected = localSelected.count(p.instance) > 0;
                float finalDistance = p.distance;

                if (hasLocalPos) {
                    float distSqr = DistanceSqr(localPos, p.pos);
                    if (!isSelected && distSqr > maxDistSqr) continue;
                    finalDistance = std::sqrt(distSqr);
                } else if (!isSelected && (p.distance > maxDist || p.distance <= 0.0f)) {
                    continue;
                }
                localCopy.emplace_back(ESPPlayerData{});
                auto& dest = localCopy.back();
                dest.instance = p.instance;
                dest.pos = p.pos;
                dest.health = p.health;
                dest.isDead = p.isDead;
                dest.isLocal = p.isLocal;
                dest.distance = finalDistance;
                dest.name = p.name;
                dest.hasValidPosition = p.hasValidPosition;
                dest.hackFlags = p.hackFlags;
                dest.lastYaw = p.lastYaw;
                dest.lastPitch = p.lastPitch;
                dest.lastAimCheck = p.lastAimCheck;
                dest.lastSvUltimoTiro = p.lastSvUltimoTiro;
                dest.lastSvUltimoTiroTime = p.lastSvUltimoTiroTime;
            }
        }
    }

    for (const auto& player : localCopy) {
        bool isSelected = localSelected.count(player.instance) > 0;
        bool isHacker = antiCheatOn && player.hackFlags.Any();

        ImVec4 boxColor   = isSelected ? GOLD_COLOR : (isHacker ? HACK_BOX_COLOR   : defaultBoxColor);
        ImVec4 nameColor  = isSelected ? GOLD_COLOR : (isHacker ? HACK_FLAG_COLOR  : defaultNameColor);
        ImVec4 distColor  = isSelected ? GOLD_COLOR : (isHacker ? HACK_FLAG_COLOR  : defaultDistColor);
        ImVec4 lineColor  = isSelected ? GOLD_COLOR : (isHacker ? HACK_LINE_COLOR  : defaultLineColor);

        Vector3 headPos = player.pos;
        headPos.Y += PLAYER_HEIGHT_STANDING;
        Vector3 midPos = player.pos;
        midPos.Y += PLAYER_HEIGHT_STANDING * 0.5f;

        ImVec2 headScreen, feetScreen, midScreen;
        float headDepth = 0.0f, feetDepth = 0.0f, midDepth = 0.0f;
        bool hasHead = ESPWorldToScreen(headPos, headScreen, &headDepth);
        bool hasFeet = ESPWorldToScreen(player.pos, feetScreen, &feetDepth);
        bool hasMid = ESPWorldToScreen(midPos, midScreen, &midDepth);

        if (!hasHead && !hasFeet && !hasMid) continue;
        visiblePlayers++;

        if (!hasHead && hasFeet) {
            float estimatedScreenHeight = PLAYER_HEIGHT_STANDING * (static_cast<float>(screenHeight) / feetDepth) * 0.5f;
            headScreen = ImVec2(feetScreen.x, feetScreen.y - estimatedScreenHeight);
            hasHead = true;
        } else if (hasHead && !hasFeet) {
            float estimatedScreenHeight = PLAYER_HEIGHT_STANDING * (static_cast<float>(screenHeight) / headDepth) * 0.5f;
            feetScreen = ImVec2(headScreen.x, headScreen.y + estimatedScreenHeight);
            hasFeet = true;
        } else if (!hasHead && !hasFeet && hasMid) {
            float estimatedHalfHeight = PLAYER_HEIGHT_STANDING * 0.5f * (static_cast<float>(screenHeight) / midDepth) * 0.5f;
            headScreen = ImVec2(midScreen.x, midScreen.y - estimatedHalfHeight);
            feetScreen = ImVec2(midScreen.x, midScreen.y + estimatedHalfHeight);
            hasHead = true; hasFeet = true;
        }

        float boxHeight = std::abs(headScreen.y - feetScreen.y);
        bool isDistant = boxHeight < 5.0f;
        if (isDistant) boxHeight = 5.0f;
        float boxWidth = boxHeight * 0.40f;
        if (boxWidth < 3.0f) boxWidth = 3.0f;
        ImVec2 boxMin(headScreen.x - boxWidth / 2.0f, headScreen.y);
        ImVec2 boxMax(headScreen.x + boxWidth / 2.0f, feetScreen.y);

        if (drawLine && hasHead) {
            float distToCenter = std::abs(headScreen.x - screenTop.x);
            float fadeAlpha = 0.3f + (0.7f * (1.0f - std::min(distToCenter / (static_cast<float>(screenWidth) / 2.0f), 1.0f)));
            ImVec4 fadeLine = ImVec4(lineColor.x, lineColor.y, lineColor.z, lineColor.w * fadeAlpha);
            DrawThickLine(screenTop, ImVec2(headScreen.x, headScreen.y), fadeLine, 3);
            if (!isDistant) {
                DrawThickLine(ImVec2(headScreen.x - 4, headScreen.y), ImVec2(headScreen.x + 4, headScreen.y), boxColor, 2);
            }
        }

        if (drawBox) {
            if (isDistant) {
                DrawThickLine(ImVec2(headScreen.x - 3, headScreen.y - 3), ImVec2(headScreen.x + 3, headScreen.y + 3), boxColor, 2);
                DrawThickLine(ImVec2(headScreen.x + 3, headScreen.y - 3), ImVec2(headScreen.x - 3, headScreen.y + 3), boxColor, 2);
            } else {
                float cornerSize = boxWidth * 0.2f;
                DrawThickLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMax.x, boxMin.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMax.x, boxMax.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x, boxMax.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x, boxMax.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x + cornerSize, boxMin.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x, boxMin.y + cornerSize), boxColor, 2);
                DrawThickLine(ImVec2(boxMax.x - cornerSize, boxMin.y), ImVec2(boxMax.x, boxMin.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x, boxMin.y + cornerSize), boxColor, 2);
                DrawThickLine(ImVec2(boxMin.x, boxMax.y - cornerSize), ImVec2(boxMin.x, boxMax.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x + cornerSize, boxMax.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMax.x - cornerSize, boxMax.y), ImVec2(boxMax.x, boxMax.y), boxColor, 2);
                DrawThickLine(ImVec2(boxMax.x, boxMax.y - cornerSize), ImVec2(boxMax.x, boxMax.y), boxColor, 2);
            }
        }

        if (drawHealth && player.health > 0 && !isDistant) {
            // Health pode vir normalizado 0..1 (0.85 = 85%) ou 0..100
            float hpVal = player.health;
            if (hpVal > 0.0f && hpVal <= 1.5f) hpVal *= 100.0f;
            hpVal = SafeClamp(hpVal, 0.0f, 100.0f);
            float healthPercent = hpVal / 100.0f;
            float barWidth = 5.0f;
            float barHeight = boxHeight * 0.8f;
            float barX = boxMin.x - barWidth - 4.0f;
            float barY = boxMin.y + (boxHeight - barHeight) / 2.0f;
            DrawThickLine(ImVec2(barX, barY), ImVec2(barX + barWidth, barY), barBg, 2);
            DrawThickLine(ImVec2(barX, barY + barHeight), ImVec2(barX + barWidth, barY + barHeight), barBg, 2);
            DrawThickLine(ImVec2(barX, barY), ImVec2(barX, barY + barHeight), barBg, 2);
            DrawThickLine(ImVec2(barX + barWidth, barY), ImVec2(barX + barWidth, barY + barHeight), barBg, 2);
            float fillHeight = barHeight * healthPercent;
            float fillY = barY + barHeight - fillHeight;
            if (fillHeight > 0) {
                DrawThickLine(ImVec2(barX + 1, fillY), ImVec2(barX + barWidth - 1, fillY), healthColor, 2);
                DrawThickLine(ImVec2(barX + 1, barY + barHeight - 1), ImVec2(barX + barWidth - 1, barY + barHeight - 1), healthColor, 2);
                DrawThickLine(ImVec2(barX + 1, fillY), ImVec2(barX + 1, barY + barHeight - 1), healthColor, 2);
                DrawThickLine(ImVec2(barX + barWidth - 1, fillY), ImVec2(barX + barWidth - 1, barY + barHeight - 1), healthColor, 2);
            }
        }

        if (antiCheatOn && isHacker) {
            std::string flagsStr = player.hackFlags.BuildString();
            if (!flagsStr.empty()) {
                ImVec2 flagSize = ESPTextSize(flagsStr.c_str());
                ImVec2 flagPos(headScreen.x - flagSize.x / 2.0f,
                               isDistant ? headScreen.y - flagSize.y - 4.0f : boxMin.y - flagSize.y - 8.0f);
                DrawESPText(flagPos, HACK_FLAG_COLOR, flagsStr.c_str());
            }
        }

        if (drawName && !player.name.empty() && hasHead) {
            const char* displayName = player.name.c_str();
            if (player.name.length() > 32) {
                snprintf(nameBuffer, sizeof(nameBuffer), "%.29s...", displayName);
                displayName = nameBuffer;
            }
            ImVec2 textSize = ESPTextSize(displayName);
            ImVec2 textPos(headScreen.x - textSize.x / 2.0f, isDistant ? headScreen.y - textSize.y - 4.0f : boxMin.y - textSize.y - 8.0f);
            DrawESPTextBg(textPos, nameColor, displayName);
        }

        if (drawDist) {
            int distInt = static_cast<int>(player.distance);
            if (distInt < 1) distInt = 1;
            snprintf(distBuffer, sizeof(distBuffer), "%dm", distInt);
            ImVec2 textSize = ESPTextSize(distBuffer);
            ImVec2 textPos(headScreen.x - textSize.x / 2.0f, isDistant ? headScreen.y + 6.0f : boxMax.y + 4.0f);
            DrawESPText(textPos, distColor, distBuffer);
        }
    }
    } // fim do bloco ESP de jogadores (hasESP) - sub-ESPs rodam independentes

    DrawLootESP();
    DrawSleepingBodiesESP();
    DrawArmariosNormalESP();
    DrawFovCircle();
    DrawMagicBulletLine();
    DrawRecursosESP();
    DrawTorretasESP();
    DrawCamasESP();

    if (visiblePlayers > 0) {
        char countBuffer[32];
        snprintf(countBuffer, sizeof(countBuffer), "%d Players", visiblePlayers);
        ImVec2 textSize = ESPTextSize(countBuffer);
        ImVec2 textPos(static_cast<float>(screenWidth) / 2.0f - textSize.x / 2.0f, 20.0f);
        DrawESPText(textPos, countColor, countBuffer);
    }

    espVisibleCount.store(visiblePlayers, std::memory_order_relaxed);
}

void BeginDraw() {
    ApplyAccentColor();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("ESP Mod Menu - Decay Survival", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
        g_window = ImGui::GetCurrentWindow();
    }

    auto now = Clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastCleanup).count() >= 10) {
        CleanupArmarios();
        CleanupRecursos();
        CleanupSleepingBodies();
        CleanupNearbyLoot();
        CleanupColetorItens();
        CleanupLoot();
        lastCleanup = now;
    }

    ImGui::BeginChild("Sidebar", ImVec2(180, 0), true);
    const char* tabs[] = {"ESP", "Combate", "Anti-Cheat", "Coletar", "Coletor", "Settings"};
    for (int i = 0; i < IM_ARRAYSIZE(tabs); i++) {
        if (ImGui::Selectable(tabs[i], selectedTab == i, 0, ImVec2(150, 45)))
            selectedTab = i;
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("MainContent", ImVec2(0, 0), true);

    switch (selectedTab) {
    case 0: {
        ImGui::Text("ESP Controls");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Box: Vermelha");
        ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f), "Nome: Azul");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Distancia: Amarela");
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Linha e Vida: Verde");
        ImGui::TextColored(espArmarioColor, "Armario: Laranja (verde=1-2, vermelho=3+ auth)");
        ImGui::TextColored(GOLD_COLOR, "Selecionado: Dourado (ignora distancia)");
        ImGui::Separator();

        bool line = espLine.load(std::memory_order_relaxed);
        bool box = espBox.load(std::memory_order_relaxed);
        bool name = espName.load(std::memory_order_relaxed);
        bool dist = espDistance.load(std::memory_order_relaxed);
        bool health = espHealth.load(std::memory_order_relaxed);
        float maxDist = espMaxDistance.load(std::memory_order_relaxed);

        if (ImGui::Checkbox("ESP Line (Top to Enemy)", &line))
            espLine.store(line, std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Box", &box))
            espBox.store(box, std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Name", &name))
            espName.store(name, std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Distance", &dist))
            espDistance.store(dist, std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Health", &health))
            espHealth.store(health, std::memory_order_relaxed);



        bool slBody = espSleepingBody.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Corpos Dormindo (SleepingBody)", &slBody))
            espSleepingBody.store(slBody, std::memory_order_relaxed);
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Azul claro = jogador offline/dormindo no chao");

        bool txtShadow = espTextShadow.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Sombra no texto ESP (opcional)", &txtShadow))
            espTextShadow.store(txtShadow, std::memory_order_relaxed);

        float txtSize = espTextSize.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Tamanho do Texto ESP", &txtSize, 14.0f, 48.0f, "%.0f px")) {
            espTextSize.store(txtSize, std::memory_order_relaxed);
        }

        bool armario = espArmario.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Armario (Dono)", &armario)) {
            espArmario.store(armario, std::memory_order_relaxed);
        }

        bool torreta = espTorretas.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Torretas", &torreta))
            espTorretas.store(torreta, std::memory_order_relaxed);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Verde = Ligada | Vermelho = Desligada");

        bool camas = espCamas.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Camas / Sacos de Dormir", &camas))
            espCamas.store(camas, std::memory_order_relaxed);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.75f, 1.0f), "Rosa = Cama (nome da cama + dono)");
        bool drops = espDrops.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("ESP Loot / Drops", &drops))
            espDrops.store(drops, std::memory_order_relaxed);
        ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "Roxo = Itens dropados no chao");


        float torMaxDist = espTorretasMaxDist.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Distancia Max Torretas", &torMaxDist, 50.0f, 2000.0f, "%.0f m")) {
            espTorretasMaxDist.store(torMaxDist, std::memory_order_relaxed);
        }
        int torCount = torretasCount.load(std::memory_order_relaxed);
        ImGui::Text("Torretas detectadas: %d", torCount);

        float camaMaxDist = espCamasMaxDist.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Distancia Max Camas", &camaMaxDist, 50.0f, 2000.0f, "%.0f m")) {
            espCamasMaxDist.store(camaMaxDist, std::memory_order_relaxed);
        }
        int camaCount = sacosCount.load(std::memory_order_relaxed);
        ImGui::Text("Camas detectadas: %d", camaCount);
        int sacStatus = sacScanStatus.load(std::memory_order_relaxed);
        if (sacStatus == 2)
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Scan camas: OK (tipo encontrado)");
        else if (sacStatus == 1)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Scan camas: TIPO NAO ENCONTRADO (Type.GetType falhou)");
        else if (sacStatus == 3)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Scan camas: FindObjectsOfType/Type_GetType NAO resolvido");
        else if (sacStatus == 4)
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Scan camas: array invalido do FindObjectsOfType");
        else if (sacStatus == 5)
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Scan camas: tipo OK mas 0 objetos na cena");
        else if (sacStatus == 6)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Scan camas: API IL2CPP nao resolvida (dlopen falhou)");
        else if (sacStatus == 7)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Scan camas: classe de cama NAO encontrada no jogo");
        else
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Scan camas: aguardando...");
        if (sacStatus == 2 && !g_camaClassName.empty())
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Classe de cama: %s", g_camaClassName.c_str());
        ImGui::Text("Camas via sacosPlayerJson: %d", sacosJsonCount.load(std::memory_order_relaxed));

        ImGui::Separator();
        float armMaxDist = espArmarioMaxDistance.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Distancia Max Armario", &armMaxDist, 50.0f, 250.0f, "%.0f m")) {
            espArmarioMaxDistance.store(armMaxDist, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Max 15 armarios proximos (evita crash)");
        int armCount = armariosCount.load(std::memory_order_relaxed);
        ImGui::Text("Armarios detectados: %d", armCount);

        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "HACKER: Vermelho (veja aba Anti-Cheat)");
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "BYPASS ARMARIO");
        bool bypass = armarioBypass.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Acesso Total a Qualquer Armario", &bypass))
            armarioBypass.store(bypass, std::memory_order_relaxed);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Ativado = Voce eh dono de todos os armarios");

        ImGui::Separator();
        if (ImGui::SliderFloat("Max Distance", &maxDist, ESP_MAX_DISTANCE_MIN, ESP_MAX_DISTANCE_MAX, "%.0f m")) {
            espMaxDistance.store(maxDist, std::memory_order_relaxed);
        }
        break;
    }

    case 1: {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.5f, 1.0f), "COMBATE - Magic Bullet & Aimbot");
        ImGui::Separator();

        bool mb = magicBullet.load(std::memory_order_relaxed);
        bool asa = aimbotSempreAcerta.load(std::memory_order_relaxed);
        bool head = aimAtHead.load(std::memory_order_relaxed);
        float smooth = aimbotSmooth.load(std::memory_order_relaxed);
        float mbMaxDist = magicBulletMaxDist.load(std::memory_order_relaxed);
        float fov = aimbotFov.load(std::memory_order_relaxed);

        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Bala Magica: Mira automatica no inimigo dentro do FOV");
        if (ImGui::Checkbox("Bala Magica", &mb))
            magicBullet.store(mb, std::memory_order_relaxed);

        if (mb) {
            if (ImGui::SliderFloat("Distancia Bala Magica", &mbMaxDist, 50.0f, 9999.0f, "%.0f m")) {
                magicBulletMaxDist.store(mbMaxDist, std::memory_order_relaxed);
            }
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Alcance maximo da bala magica");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Aimbot Sempre Acerta: Aumenta hitbox e mira automaticamente");
        if (ImGui::Checkbox("Aimbot Sempre Acerta", &asa))
            aimbotSempreAcerta.store(asa, std::memory_order_relaxed);

        ImGui::Separator();
        bool noRecoil = semRecoil.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Sem Recoil (arma nao sobe)", &noRecoil))
            semRecoil.store(noRecoil, std::memory_order_relaxed);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Zera o recuo da arma a cada tiro");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "FOV Circle: Circulo no centro da tela. Aimbot so trava se o inimigo estiver DENTRO.");
        if (ImGui::SliderFloat("FOV Circle (pixels)", &fov, 20.0f, 400.0f, "%.0f px")) {
            aimbotFov.store(fov, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "20 = Muito pequeno | 400 = Quase tela inteira");

        ImGui::Separator();
        ImGui::Text("Configuracoes");
        if (ImGui::Checkbox("Mirar na Cabeca", &head))
            aimAtHead.store(head, std::memory_order_relaxed);
        if (!head) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Peito)");
        }
        if (ImGui::SliderFloat("Suavizacao", &smooth, 0.01f, 1.0f, "%.2f")) {
            aimbotSmooth.store(smooth, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "0.01 = Instantaneo | 1.0 = Muito Lento");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "DEBUG");
        if (g_CTC && IsIl2CppObjectValid(g_CTC)) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "CTC Local: OK");
            // CTC_baseYaw = 0x2C8 | CTC_basePitch = 0x2CC (ATUALIZADO)
            float curYaw   = ReadFloatSafe(g_CTC, 0x2C8);
            float curPitch = ReadFloatSafe(g_CTC, 0x2CC);
            ImGui::Text("Yaw: %.1f | Pitch: %.1f", curYaw, curPitch);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CTC Local: NAO DETECTADO");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Entre no jogo e mova a camera");
        }
        if (g_aimbotTarget && IsIl2CppObjectValid(g_aimbotTarget)) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Now() - g_aimbotTargetTime).count();
            if (elapsed < 1000) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "[AIMBOT] Alvo travado!");
            }
        }
        if (mb && g_magicBulletTarget && IsIl2CppObjectValid(g_magicBulletTarget)) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "[BALA MAGICA] Alvo travado - Linha magenta ativa!");
        }
        break;
    }

    case 2: {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ANTI-CHEAT - Deteccao de Hackers");
        ImGui::Separator();

        bool antiCheat = espAntiCheat.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Ativar Deteccao de Hacks no ESP", &antiCheat))
            espAntiCheat.store(antiCheat, std::memory_order_relaxed);

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Jogadores com hack aparecem em VERMELHO no ESP");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "As flags [AIMBOT] [TP] [NORECOIL] etc aparecem ao lado do nome");
        ImGui::Separator();

        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Legenda de Hacks Detectados:");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[GOD]    = GodMode ativo");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[FLY]    = Voando (Fly Hack)");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[GHOST]  = Modo Fantasma");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[SPEED]  = Speed Hack (>15m/s)");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[JUMP]   = Super Pulo (>3m)");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[RAPID]  = Tiro Rapido");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[AIMBOT] = Mira Automatica");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[MAGIC]  = Bala Magica");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[WALL]   = Wallhack (Xray)");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[TP]     = Teleporte");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[NORECOIL] = Atira sem recuo (mira nao sobe)");
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[MAGIC]/[AIMBOT] em kill = CONFIRMADO por analise de mira");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Hack confirmado fica marcado pra SEMPRE (ate trocar de server)");
        ImGui::Separator();

        // ============================================================
        // LISTA DE TODOS OS JOGADORES ONLINE NO SERVIDOR
        // ============================================================
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "JOGADORES ONLINE NO SERVIDOR");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Lista completa de todos os jogadores detectados na rede");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Apenas jogadores sincronizados pelo servidor aparecem aqui)");
        {
            std::lock_guard<std::mutex> lock(onlinePlayersMutex);
            int totalOnline = (int)onlinePlayers.size();

            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Total de jogadores online: %d", totalOnline);

            if (onlinePlayers.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Nenhum jogador online detectado.");
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Entre em um servidor com jogadores.");
            } else {
                // Leitura em tempo real dos dados de cada jogador
                struct LivePlayerInfo {
                    void* instance = nullptr;
                    std::string name;
                    float health = 0.0f;
                    bool isDead = false;
                    bool isValid = false;
                    float distance = 0.0f;
                    bool hasPos = false;
                };
                std::vector<LivePlayerInfo> liveList;
                liveList.reserve(onlinePlayers.size());

                for (auto& p : onlinePlayers) {
                    if (!p.instance || !IsUnityObjectAlive(p.instance)) continue;

                    LivePlayerInfo info;
                    info.instance = p.instance;
                    info.isValid = true;

                    // PlayerStatus_PlayerName = 0x80 (ATUALIZADO)
                    void* nomeStr = ReadPtrSafe(p.instance, 0x80);
                    std::string nome = ReadIl2CppString(nomeStr);
                    info.name = nome.empty() ? "Jogador" : nome;

                    // Leitura em tempo real do HP (offset auto-detectado)
                    info.health = GetPlayerHealth(p.instance);

                    // PlayerStatus_isDead = 0x2ED (ATUALIZADO)
                    info.isDead = ReadBoolSafe(p.instance, 0x2ED);

                    // Leitura em tempo real da posição
                    void* transform = GetTransformFromComponent(p.instance);
                    if (IsValidPtr(transform) && get_position) {
                        Vector3 rawPos = get_position(transform);
                        if (std::isfinite(rawPos.X) && std::isfinite(rawPos.Y) && std::isfinite(rawPos.Z)) {
                            Vector3 feetPos = { rawPos.X, rawPos.Y - PLAYER_CENTER_TO_FEET_OFFSET, rawPos.Z };
                            info.hasPos = true;
                            if (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f) {
                                info.distance = std::sqrt(DistanceSqr(feetPos, MyPos));
                            }
                        }
                    }

                    liveList.push_back(std::move(info));
                }

                std::sort(liveList.begin(), liveList.end(), [](const LivePlayerInfo& a, const LivePlayerInfo& b) {
                    if (a.hasPos && b.hasPos) return a.distance < b.distance;
                    if (a.hasPos) return true;
                    if (b.hasPos) return false;
                    return a.name < b.name;
                });

                ImGui::BeginChild("AllPlayersList", ImVec2(0, 250), true);
                for (auto& lp : liveList) {
                    // Hack confirmado por kill analysis: vermelho + flags do cheat
                    uint32_t confBits = GetConfirmedHackFlags(lp.name);
                    HackFlags confHf; confHf.bits = confBits;
                    std::string confStr = confHf.BuildString();

                    ImVec4 rowColor;
                    if (!confStr.empty()) rowColor = ImVec4(1.0f, 0.25f, 0.25f, 1.0f);
                    else if (lp.isDead)   rowColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                    else                  rowColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, rowColor);

                    const char* status = lp.isDead ? "[MORTO]" : "[VIVO]";
                    int hp = static_cast<int>(lp.health);
                    if (hp < 0) hp = 0;
                    if (hp > 100) hp = 100;

                    char buf[384];
                    if (!confStr.empty()) {
                        if (lp.hasPos)
                            snprintf(buf, sizeof(buf), "%.0fm | %s %s | HP: %d%% %s", lp.distance, lp.name.c_str(), confStr.c_str(), hp, status);
                        else
                            snprintf(buf, sizeof(buf), "?m | %s %s | HP: %d%% %s", lp.name.c_str(), confStr.c_str(), hp, status);
                    } else if (lp.hasPos) {
                        snprintf(buf, sizeof(buf), "%.0fm | %s | HP: %d%% %s", lp.distance, lp.name.c_str(), hp, status);
                    } else {
                        snprintf(buf, sizeof(buf), "?m | %s | HP: %d%% %s", lp.name.c_str(), hp, status);
                    }

                    ImGui::Text("%s", buf);
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();
            }
        }
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Jogadores Detectados com Hack:");
        {
            std::lock_guard<std::mutex> lock(espMutex);
            int hackerCount = 0;
            std::vector<ESPPlayerData*> hackers;
            hackers.reserve(espPlayers.size());
            for (auto& p : espPlayers) {
                if (p.hackFlags.Any() && !p.isDead && IsIl2CppObjectValid(p.instance)) {
                    hackers.push_back(&p);
                    hackerCount++;
                }
            }

            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Total de Hackers detectados: %d", hackerCount);
            ImGui::Separator();

            if (hackers.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Nenhum hacker detectado no momento.");
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Ative o ESP e o Anti-Cheat para detectar.");
            } else {
                std::sort(hackers.begin(), hackers.end(), [](ESPPlayerData* a, ESPPlayerData* b) {
                    return a->distance < b->distance;
                });

                ImGui::BeginChild("HackerList", ImVec2(0, 350), true);
                for (auto* hp : hackers) {
                    ImVec4 rowColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, rowColor);

                    std::string display = hp->name.empty() ? "Jogador" : hp->name;
                    std::string flags = hp->hackFlags.BuildString();
                    int flagCount = hp->hackFlags.Count();

                    char buf[256];
                    if (hp->distance > 0.0f)
                        snprintf(buf, sizeof(buf), "%.0fm | %s | %s (%d hacks)", hp->distance, display.c_str(), flags.c_str(), flagCount);
                    else
                        snprintf(buf, sizeof(buf), "?m | %s | %s (%d hacks)", display.c_str(), flags.c_str(), flagCount);

                    ImGui::Text("%s", buf);
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();
            }
        }
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.0f, 1.0f), "FEED DE KILLS - ANALISE DA MIRA DO KILLER");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "mira < 0.1 = matou SEM olhar [MAGIC] | mira > 0.995 = mira perfeita [AIMBOT]");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Hackers confirmados por kill: %d", CountConfirmedHackers());
        {
            std::lock_guard<std::mutex> lock(killEventsMutex);
            if (killEvents.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Nenhum kill analisado ainda (aguardando IniciarMorte).");
            } else {
                ImGui::BeginChild("KillFeedList", ImVec2(0, 220), true);
                for (auto it = killEvents.rbegin(); it != killEvents.rend(); ++it) {
                    long long age = (long long)std::chrono::duration_cast<std::chrono::seconds>(Now() - it->time).count();
                    HackFlags hf; hf.bits = it->flags;
                    std::string fl = hf.BuildString();
                    char buf[384];
                    if (!fl.empty())
                        snprintf(buf, sizeof(buf), "[%llds] %s %s matou %s (%.0fm | mira %.2f)", age, it->killer.c_str(), fl.c_str(), it->victim.c_str(), it->distance, it->aimDot);
                    else
                        snprintf(buf, sizeof(buf), "[%llds] %s matou %s (%.0fm | mira %.2f)", age, it->killer.c_str(), it->victim.c_str(), it->distance, it->aimDot);
                    if (fl.empty())
                        ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "%s", buf);
                    else
                        ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "%s", buf);
                }
                ImGui::EndChild();
            }
        }
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "ANTI-ESP - SPOOF DE POSICAO (esconder sua posicao)");
        bool spoof = spoofPosicao.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("ATIVAR SPOOF DE POSICAO (liga/desliga na hora)", &spoof)) {
            spoofPosicao.store(spoof, std::memory_order_relaxed);
        }
        const char* spoofModes[] = {"So altura (Y falsa) - NAO quebra o mapa",
                                    "Deslocamento X/Z - some do mapa dos inimigos",
                                    "Fora do mapa (99999) - QUEBRA a renderizacao"};
        int modo = spoofModo.load(std::memory_order_relaxed);
        if (ImGui::Combo("Modo do spoof", &modo, spoofModes, IM_ARRAYSIZE(spoofModes))) {
            spoofModo.store(modo, std::memory_order_relaxed);
        }
        if (modo == 0) {
            float yoff = spoofYOffset.load(std::memory_order_relaxed);
            if (ImGui::SliderFloat("Altitude falsa (Y)", &yoff, -500.0f, 2000.0f, "%.0f m")) {
                spoofYOffset.store(yoff, std::memory_order_relaxed);
            }
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "X/Z continuam reais: mapa e renderizacao normais");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ESP 3D dos inimigos mostra distancia/altura erradas");
        } else if (modo == 1) {
            float xoff = spoofXOffset.load(std::memory_order_relaxed);
            if (ImGui::SliderFloat("Deslocamento X", &xoff, -2000.0f, 2000.0f, "%.0f m")) {
                spoofXOffset.store(xoff, std::memory_order_relaxed);
            }
            float zoff = spoofZOffset.load(std::memory_order_relaxed);
            if (ImGui::SliderFloat("Deslocamento Z", &zoff, -2000.0f, 2000.0f, "%.0f m")) {
                spoofZOffset.store(zoff, std::memory_order_relaxed);
            }
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Inimigos te veem ~%.0fm deslocado no mapa", std::sqrt(xoff*xoff + zoff*zoff));
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Se o mundo sumir, diminua o deslocamento (tente 100-300m)");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "MODO ANTIGO: o mapa/itens somem (voce ja viu)");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "So use se nao precisar ver o mundo ao redor");
        }
        if (spoof) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "SPOOF ATIVO (modo %d): leituras externas veem posicao falsa", modo);
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "AVISO: baus/fornalhas/torretas podem nao detectar voce.");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "SPOOF DESATIVADO: posicao real visivel para leituras externas");
        }
        int spoofStatus = spoofHookStatus.load(std::memory_order_relaxed);
        if (spoofStatus == 2)
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Hook spoof: OK - seu ESP fica 100% enquanto voce esta oculto");
        else if (spoofStatus == 1)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hook spoof: FALHOU - spoof indisponivel neste dispositivo");
        else
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Hook spoof: aguardando instalacao...");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Seu proprio ESP/aimbot/coletor continuam 100% (leitura interna isolada)");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "FERRAMENTAS CLIENTE (funcionam em qualquer servidor)");
        {
            int ppsStatus = ppsScanStatus.load(std::memory_order_relaxed);
            if (ppsStatus == 2)
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "PlayerPersistenceService: OK");
            else if (ppsStatus == 1)
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "PlayerPersistenceService: NAO ENCONTRADO");
            else
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "PlayerPersistenceService: aguardando scan...");
        }
        static int offlineId = 0;
        static char offlineInfo[256] = "Informe o ID do jogador e consulte a posicao salva.";
        static char offlineRaw[128] = "";
        static char offlineAlt[128] = "";
        ImGui::InputInt("Player ID (offline)", &offlineId);
        if (ImGui::Button("Ver Posicao OFFLINE (ultima salva)")) {
            Vector3 p{};
            offlineRaw[0] = 0;
            offlineAlt[0] = 0;
            if (ppsScanStatus.load(std::memory_order_relaxed) != 2) {
                GetPersistenceService(); // forca re-scan
            }
            if (ppsScanStatus.load(std::memory_order_relaxed) != 2) {
                snprintf(offlineInfo, sizeof(offlineInfo), "PlayerPersistenceService NAO encontrado na cena.");
            } else if (Client_GetOfflinePositionDbg(offlineId, p, offlineRaw, sizeof(offlineRaw))) {
                float dist = (MyPos.X != 0.0f || MyPos.Y != 0.0f || MyPos.Z != 0.0f)
                    ? std::sqrt(DistanceSqr(p, MyPos)) : 0.0f;
                snprintf(offlineInfo, sizeof(offlineInfo), "ID %d @ offset4: %.1f, %.1f, %.1f (%.0fm de voce)",
                    offlineId, p.X, p.Y, p.Z, dist);
                // leitura alternativa em offset 8 (diagnostico de layout do Nullable)
                alignas(16) char buf2[32] = {0};
                uintptr_t addr2 = il2cpp + Offsets::PPS_GetTransformDoCache;
                if (IsValidAddress(addr2)) {
                    auto fn2 = reinterpret_cast<void(*)(void*, void*, int)>(addr2);
                    fn2(buf2, g_persistenceService, offlineId);
                    float* f8 = reinterpret_cast<float*>(buf2 + 8);
                    snprintf(offlineAlt, sizeof(offlineAlt), "offset8: %.1f, %.1f, %.1f", f8[0], f8[1], f8[2]);
                }
            } else {
                snprintf(offlineInfo, sizeof(offlineInfo), "ID %d: sem posicao salva (hasValue=0) ou ID invalido", offlineId);
            }
        }
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", offlineInfo);
        if (offlineRaw[0]) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", offlineRaw);
            if (offlineAlt[0]) ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", offlineAlt);
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Me diga qual linha bate com a posicao real (offset4 ou offset8)");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Jogadores online - clique para usar o ID:");
        {
            std::lock_guard<std::mutex> lock(onlinePlayersMutex);
            ImGui::BeginChild("OnlineIdList", ImVec2(0, 160), true);
            bool anyOnline = false;
            for (auto& p : onlinePlayers) {
                if (!IsUnityObjectAlive(p.instance)) continue;
                anyOnline = true;
                int pid = ReadIntSafe(p.instance, Offsets::PlayerStatus_PlayerID);
                ImGui::Text("%s = ID %d", p.name.empty() ? "Jogador" : p.name.c_str(), pid);
                ImGui::SameLine();
                char btnId[64];
                snprintf(btnId, sizeof(btnId), "Usar##%p", p.instance);
                if (ImGui::SmallButton(btnId)) offlineId = pid;
            }
            if (!anyOnline)
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Nenhum jogador online detectado.");
            ImGui::EndChild();
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "LOOT DOS CORPOS (leitura cliente)");
        {
            std::lock_guard<std::mutex> lock(sleepingBodiesMutex);
            ImGui::BeginChild("CorposLootList", ImVec2(0, 220), true);
            bool anyCorpo = false;
            for (auto& sb : sleepingBodiesList) {
                if (!IsUnityObjectAlive(sb.instance)) continue;
                anyCorpo = true;
                int hp = static_cast<int>(sb.health);
                if (hp < 0) hp = 0;
                if (hp > 100) hp = 100;
                ImGui::TextColored(sb.morto ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.2f, 0.8f, 1.0f, 1.0f),
                    "%s%s | %d slots | HP:%d%%", sb.ownerName.c_str(),
                    sb.morto ? " [MORTO]" : " [DORMINDO]", sb.slotCount, hp);
                if (!sb.inventarioJson.empty()) {
                    std::string clipped = sb.inventarioJson.size() > 220
                        ? sb.inventarioJson.substr(0, 220) + "..." : sb.inventarioJson;
                    ImGui::TextWrapped("  %s", clipped.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  (inventario vazio/nao lido)");
                }
            }
            if (!anyCorpo)
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Nenhum corpo detectado ainda.");
            ImGui::EndChild();
        }
        break;
    }
    case 3: {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "COLETAR - ESP de Recursos & Loot");
        ImGui::Separator();

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "NEARBY LOOT SYSTEM");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Interage com itens atraves de paredes!");
        ImGui::Separator();

        bool lWall = lootWallhack.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Loot Wallhack (Ver Atraves de Paredes)", &lWall)) {
            lootWallhack.store(lWall, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Zera layerObstaculos = detecta itens em qualquer lugar");

        bool lAuto = lootAutoCollect.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Coleta Automatica (NearbyLoot)", &lAuto)) {
            lootAutoCollect.store(lAuto, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Coleta o item selecionado automaticamente a cada Tick");

        float lRadius = lootRadius.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Raio de Deteccao Loot", &lRadius, 30.0f, 500.0f, "%.0f m")) {
            lootRadius.store(lRadius, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Padrao: 30m | Recomendado: 200m+");

        int nlCount = nearbyLootCount.load(std::memory_order_relaxed);
        ImGui::Text("NearbyLoot instancias: %d", nlCount);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "ESP DE RECURSOS (Minerios)");
        ImGui::Separator();

        bool madeira = espMadeira.load(std::memory_order_relaxed);
        bool pedra = espPedra.load(std::memory_order_relaxed);
        bool metal = espMetal.load(std::memory_order_relaxed);
        bool enxofre = espEnxofre.load(std::memory_order_relaxed);
        bool geral = espRecursosGeral.load(std::memory_order_relaxed);
        float recMaxDist = espRecursosMaxDist.load(std::memory_order_relaxed);

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Ative o tipo de recurso que deseja ver no ESP");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "'Geral' mostra TODOS os recursos detectados");
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Text, espMadeiraColor);
        if (ImGui::Checkbox("ESP Madeira / Arvore", &madeira))
            espMadeira.store(madeira, std::memory_order_relaxed);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, espPedraColor);
        if (ImGui::Checkbox("ESP Pedra", &pedra))
            espPedra.store(pedra, std::memory_order_relaxed);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, espMetalColor);
        if (ImGui::Checkbox("ESP Minerio de Metal", &metal))
            espMetal.store(metal, std::memory_order_relaxed);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, espEnxofreColor);
        if (ImGui::Checkbox("ESP Minerio de Enxofre", &enxofre))
            espEnxofre.store(enxofre, std::memory_order_relaxed);
        ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, espRecursosGeralColor);
        if (ImGui::Checkbox("ESP Geral (Todos os Recursos)", &geral))
            espRecursosGeral.store(geral, std::memory_order_relaxed);
        ImGui::PopStyleColor();

        ImGui::Separator();
        if (ImGui::SliderFloat("Distancia Max Recursos", &recMaxDist, 50.0f, 9999.0f, "%.0f m")) {
            espRecursosMaxDist.store(recMaxDist, std::memory_order_relaxed);
        }

        int recCount = recursosCount.load(std::memory_order_relaxed);
        ImGui::Text("Recursos detectados: %d", recCount);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "FERRAMENTA LONGE - Machado / Coleta Remota");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Bata em arvores e recursos de longe!");
        ImGui::Separator();

        bool ferrLonge = ferramentaAlcanceLonge.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Ativar Alcance Longo (Machado)", &ferrLonge)) {
            ferramentaAlcanceLonge.store(ferrLonge, std::memory_order_relaxed);
        }

        float ferrAlcance = ferramentaAlcanceMax.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Alcance Maximo (m)", &ferrAlcance, 5.0f, 200.0f, "%.0f m")) {
            ferramentaAlcanceMax.store(ferrAlcance, std::memory_order_relaxed);
        }

        float ferrRaio = ferramentaRaioGolpe.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Raio do Golpe (area)", &ferrRaio, 1.0f, 50.0f, "%.0f m")) {
            ferramentaRaioGolpe.store(ferrRaio, std::memory_order_relaxed);
        }

        float ferrFallback = ferramentaAlcanceFallback.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Alcance Fallback (m)", &ferrFallback, 5.0f, 200.0f, "%.0f m")) {
            ferramentaAlcanceFallback.store(ferrFallback, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Padrao: 100m alcance | 15m raio");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.8f, 1.0f), "INSTA-BREAK & FERRAMENTA INFINITA");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Quebra recursos com 1 golpe + ferramenta nao quebra");
        ImGui::Separator();

        bool insta = instaBreak.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Insta-Break (1 Golpe Quebra)", &insta)) {
            instaBreak.store(insta, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Madeira, Pedra, Metal e Enxofre com 1 de vida");

        bool infinita = ferramentaInfinita.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Ferramenta Infinita (Sem Desgaste)", &infinita)) {
            ferramentaInfinita.store(infinita, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Machado/Picareta nunca quebra");

        float mult = dropMultiplier.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Multiplicador de Drop", &mult, 1.0f, 100.0f, "%.0fx")) {
            dropMultiplier.store(mult, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Multiplica a quantidade dos drops no chao");

        bool recInf = recursoInfinito.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Recurso Infinito (minerio nunca acaba)", &recInf)) {
            recursoInfinito.store(recInf, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Madeira/Pedra/Metal/Enxofre com 999999 de vida");

        int porBatida = recursoPorBatida.load(std::memory_order_relaxed);
        if (ImGui::SliderInt("Quantidade por Batida", &porBatida, 1, 500, "%d")) {
            recursoPorBatida.store(porBatida, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Quanto cada golpe coleta do recurso");
        break;
    }

    case 4: {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "COLETOR ITENS - Coleta Automatica & Interacao");
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Sistema direto no ColetorItens do jogador");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Mais confiavel que NearbyLootSystem!");
        ImGui::Separator();

        bool cAuto = coletorAutoCollect.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Coleta Automatica (ColetorItens)", &cAuto)) {
            coletorAutoCollect.store(cAuto, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Coleta TUDO automaticamente sem apertar botao!");

        float cRadius = coletorAutoRadius.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Raio Coleta Automatica", &cRadius, 5.0f, 200.0f, "%.0f m")) {
            coletorAutoRadius.store(cRadius, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Padrao: 50m | Aumente pra coletar de longe");

        ImGui::Separator();

        bool cWall = coletorWallhack.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Wallhack Deteccao (Atravessa Paredes)", &cWall)) {
            coletorWallhack.store(cWall, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Zera layersObstaculo = detecta atraves de paredes");

        float cRayDist = coletorRaycastDist.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Distancia Raycast", &cRayDist, 10.0f, 500.0f, "%.0f m")) {
            coletorRaycastDist.store(cRayDist, std::memory_order_relaxed);
        }

        float cSphere = coletorSphereRadius.load(std::memory_order_relaxed);
        if (ImGui::SliderFloat("Raio SphereCast", &cSphere, 1.0f, 50.0f, "%.0f m")) {
            coletorSphereRadius.store(cSphere, std::memory_order_relaxed);
        }

        ImGui::Separator();

        bool cInteract = coletorAutoInteract.load(std::memory_order_relaxed);
        if (ImGui::Checkbox("Interagir Automaticamente", &cInteract)) {
            coletorAutoInteract.store(cInteract, std::memory_order_relaxed);
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Abre baus e fornalhas automaticamente ao mirar");

        ImGui::Separator();
        int cCount = coletorCount.load(std::memory_order_relaxed);
        ImGui::Text("ColetorItens detectados: %d", cCount);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Offset coletaAutomatica: 0x11C");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Offset raioColetaAuto: 0x120");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Offset layersObstaculo: 0x78");
        break;
    }

    case 5: {
        ImGui::Text("Settings");
        ImGui::Separator();
        ImGui::ColorEdit3("Accent Color", reinterpret_cast<float*>(&AccentColor));
        break;
    }
    }

    ImGui::EndChild();
    ImGui::End();
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_init(JNIEnv* env, jclass cls);
    JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_resize(JNIEnv* env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_step(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_imgui_Shutdown(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_MotionEventClick(JNIEnv* env, jobject obj, jboolean down, jfloat PosX, jfloat PosY);
    JNIEXPORT jstring JNICALL Java_com_mycompany_application_GLES3JNIView_getWindowRect(JNIEnv *env, jobject thiz);
    JNIEXPORT void JNICALL Java_com_mycompany_application_GLES3JNIView_real(JNIEnv* env, jobject obj, jint width, jint height);
};

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_init(JNIEnv* env, jclass cls) {
    if (g_Initialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    SetupModernStyle();
    io.Fonts->AddFontDefault();
    {
        // Fonte exclusiva do ESP: tamanho fixo, ignora o FontGlobalScale do menu
        ImFontConfig esp_cfg;
        esp_cfg.SizePixels = 26.0f;
        esp_font = io.Fonts->AddFontDefault(&esp_cfg);
    }
    io.FontGlobalScale = 2.5f;
    SetupFontAwesomeIcons();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 6.0f;
    s.FrameRounding = 6.0f;
    s.GrabRounding = 6.0f;
    g_Initialized = true;
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_resize(JNIEnv* env, jobject obj, jint width, jint height) {
    screenWidth = static_cast<int>(width);
    screenHeight = static_cast<int>(height);
    glViewport(0, 0, width, height);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_step(JNIEnv* env, jobject obj) {
    ImGuiIO& io = ImGui::GetIO();
    static bool show_MainMenu_window = true;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(screenWidth, screenHeight);
    ImGui::NewFrame();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.75f, 1.00f, 1.00f);
    style.GrabMinSize = 15.0f;
    style.GrabRounding = 8.0f;
    DrawESP();
    if (show_MainMenu_window) BeginDraw();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void StartBackend(JNIEnv* env) {
    #if defined(__aarch64__)
        A64HookFunction((void*)env->functions->RegisterNatives, (void*)hook_RegisterNatives, (void**)&old_RegisterNatives);
    #else
        MSHookFunction((void*)env->functions->RegisterNatives, (void*)hook_RegisterNatives, (void**)&old_RegisterNatives);
    #endif
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_imgui_Shutdown(JNIEnv* env, jobject obj) {
    if (!g_Initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    g_Initialized = false;
}

JNIEXPORT void JNICALL
Java_com_mycompany_application_GLES3JNIView_MotionEventClick(JNIEnv* env, jobject obj, jboolean down, jfloat PosX, jfloat PosY) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[0] = down;
    io.MousePos = ImVec2(PosX, PosY);
}

JNIEXPORT jstring JNICALL
Java_com_mycompany_application_GLES3JNIView_getWindowRect(JNIEnv *env, jobject thiz) {
    char result[256] = "0|0|0|0";
    if (g_window) {
        snprintf(result, sizeof(result), "%d|%d|%d|%d",
            static_cast<int>(g_window->Pos.x),
            static_cast<int>(g_window->Pos.y),
            static_cast<int>(g_window->Size.x),
            static_cast<int>(g_window->Size.y));
    }
    return env->NewStringUTF(result);
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *globalEnv;
    vm->GetEnv((void**)&globalEnv, JNI_VERSION_1_6);
    publicVM = vm;
    publicEnv = globalEnv;
    pthread_t ptid;
    pthread_create(&ptid, nullptr, hack_thread, nullptr);
    StartBackend(globalEnv);
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *vm, void *reserved) {}