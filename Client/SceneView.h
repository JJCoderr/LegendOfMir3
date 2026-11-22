#pragma once

#include "ClientState.h"

namespace mir3_client {

class LoginClient;

struct SceneSprite {
    cui_texture texture = 0;
    int x = 0, y = 0, width = 0, height = 0, row = 0;
    std::uint8_t opacity = 255;
    float sourceWidth = 1;
    std::optional<cui_effect_blend> effect;
    cui_color tint{255,255,255,255};
};

struct SceneActorHit {
    cui_rect bounds{};
    std::uint32_t actor = 0;
    unsigned row = 0;
    bool monster = false;
};

struct GroundLabel {
    cui_texture texture;
    int x, y, width;
};

struct SceneCaption {
    cui_texture texture;
    std::array<int,4> rect;
    cui_color color;
};

struct SceneEffectQuad {
    cui_texture texture;
    std::array<cui_vec2,4> corners;
    cui_color tint;
    bool upright;
    cui_effect_blend blend;
};

struct SceneActorAnchor {
    std::uint32_t actor;
    cui_rect bounds;
    int x, y, order;
};

struct SceneMapArt {
    const std::vector<SceneSprite>* sprites = nullptr;
    const std::vector<SceneActorHit>* npcs = nullptr;
    const std::vector<SceneActorHit>* targets = nullptr;
    const std::vector<SceneActorHit>* corpses = nullptr;
    const std::vector<GroundLabel>* groundLabels = nullptr;
    const std::vector<SceneCaption>* names = nullptr;
    const std::vector<SceneCaption>* speech = nullptr;
    int cameraX = 0, cameraY = 0;
    std::size_t overlayStart = 0;
    const mir3::SceneLightGrid* light = nullptr;
    const std::vector<SceneEffectQuad>* weather = nullptr;
    const std::vector<SceneEffectQuad>* levelEffects = nullptr;
    const std::vector<SceneEffectQuad>* spellEffects = nullptr;
};

struct EquipmentPortrait {
    mir3_cui::CharacterAppearance appearance;
    bool helmet = false;
};

void PaintEquipmentPortrait(cui_ctx* ui, cui_rect rect, void* user);
void DrawLocalScene(cui_ctx* ui, LoginClient& client, bool interactive, bool nativeInventory,
    bool legacyHud = false, SceneMapArt art = {});

} // namespace mir3_client
