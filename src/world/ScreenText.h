#include <glm/glm.hpp>
#include <imgui.h>

struct Text2D
{
    const char *label = R"(Text)";
    ImVec2 position{0.0f, 0.0f};
    ImU32 color = IM_COL32(255, 255, 255, 200);
};

struct TextFade2D
{
    float fadeDelay = 2.0f;
    float fadingProgress = 0.0f;
    float maxDuration = 5.0f;

    const char *label = R"(Text)";
    ImVec2 position{0.0f, 0.0f};
    ImU32 color = IM_COL32(255, 255, 255, 200);
};

struct TextBillboard
{
    float fadeDistance = 2.0f;
    float minFadeDistance = 500.0f;
    float maxFadeDistance = 1000.0f;
    float baseAlpha = 200.0f;

    const char *label = R"(Text)";
    const glm::vec4 *dynamicWorldPos = nullptr;
    glm::vec3 worldPos = glm::vec3(0.0f);
    glm::vec3 rgb = glm::vec3(255, 255, 255);
    ImU32 color = IM_COL32(255, 255, 255, 250); // FIXME REMOVE THIS
};