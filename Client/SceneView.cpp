#include "SceneView.h"
#include "LoginClient.h"

namespace mir3_client {

static void PaintCharacterFigure(cui_ctx* ui, cui_rect rect, const EquipmentPortrait& portrait, float scale = 1.0f)
{
    const auto& appearance = portrait.appearance;
    if (!appearance.available) {
        cui_draw_icon(ui, CUI_ICON_USER, rect, 20, {100, 116, 139, 255});
        return;
    }

    static const cui_color coats[] = {{100, 116, 139, 255}, {38, 104, 144, 255}, {43, 115, 94, 255},
        {126, 71, 108, 255}, {158, 89, 55, 255}, {65, 92, 153, 255}, {133, 92, 55, 255},
        {46, 113, 134, 255}, {140, 64, 71, 255}};
    static const cui_color hairColors[] = {{64, 47, 38, 255}, {47, 44, 59, 255}, {113, 67, 49, 255},
        {127, 91, 57, 255}, {92, 96, 115, 255}};
    const auto point = [&](float x, float y) { return cui_vec2{rect.x + x * scale, rect.y + y * scale}; };
    const float shoulder = appearance.body == 0 ? 10.0f : 8.0f;
    const cui_vec2 coat[] = {point(24 - shoulder, 26), point(24 + shoulder, 26), point(38, 37),
        point(34, 41), point(14, 41), point(10, 37)};
    cui_draw_polygon(ui, coat, 6, coats[appearance.clothing]);
    cui_draw_circle(ui, point(24, 17), 8.0f * scale, {229, 180, 143, 255});

    if (portrait.helmet) {
        const cui_vec2 cap[] = {point(15, 16), point(18, 9), point(30, 9), point(33, 16), point(29, 19), point(19, 19)};
        cui_draw_polygon(ui, cap, 6, {70, 86, 105, 255});
    } else {
        const float top = 8.0f + static_cast<float>(appearance.hair % 3);
        const cui_vec2 fringe[] = {point(16, 16), point(19, top), point(29, top), point(32, 16), point(25, 18)};
        cui_draw_polygon(ui, fringe, 5, hairColors[appearance.hair % 5]);
    }
    if (appearance.weapon) {
        cui_draw_line(ui, point(38, 37), point(48, 14), 3.0f * scale, {75, 99, 122, 255});
        cui_draw_line(ui, point(36, 29), point(44, 33), 2.0f * scale, {54, 71, 91, 255});
    }
}

void PaintEquipmentPortrait(cui_ctx* ui, cui_rect rect, void* user)
{
    cui_draw_rect(ui, rect, {10, 10, 10, 10}, {231, 237, 243, 255});
    PaintCharacterFigure(ui, rect, *static_cast<const EquipmentPortrait*>(user));
}

struct LocalSceneDrawing {
    EquipmentPortrait portrait;
    unsigned x, y, facing;
    LocalScene::Movement pending;
    bool terrainReady;
    bool legacyHud;
    SceneMapArt art;
    mir3_cui::MapTerrain::View ground;
    const std::map<std::uint32_t, LocalScene::Player>* players;
    char location[832];
    char name[128];
};

static void PaintLocalScene(cui_ctx* ui, cui_rect rect, void* user)
{
    const auto& model = *static_cast<const LocalSceneDrawing*>(user);
    if (model.art.sprites) {
        cui_draw_rect(ui,rect,{}, {0,0,0,255});
        cui_push_clip(ui,rect);
        const float scale=rect.w/800;
        const auto drawSprite=[&](const SceneSprite& sprite) {
            if (sprite.effect) {
                const float x=rect.x+sprite.x*scale,y=rect.y+sprite.y*scale;
                const float r=x+sprite.width*scale,b=y+sprite.height*scale;
                const cui_vec2 quad[]{{x,y},{r,y},{r,b},{x,b}};
                constexpr cui_vec2 uv[]{{0,0},{1,0},{1,1},{0,1}};
                cui_draw_effect_quad(ui,sprite.texture,quad,uv,sprite.tint,*sprite.effect);
                return;
            }
            cui_draw_image(ui,sprite.texture,{rect.x+sprite.x*scale,rect.y+sprite.y*scale,
                sprite.width*scale,sprite.height*scale},{0,0,sprite.sourceWidth,1},{},{255,255,255,sprite.opacity});
        };
        const auto& sprites=*model.art.sprites;
        for (std::size_t i=0;i<model.art.overlayStart;++i) drawSprite(sprites[i]);
        const auto effects=[&](const std::vector<SceneEffectQuad>* quads) {
            if (!quads) return;
            constexpr cui_vec2 uprightUV[]{{0,0},{1,0},{1,1},{0,1}};
            constexpr cui_vec2 particleUV[]{{1,1},{1,0},{0,0},{0,1}};
            for (const auto& sprite:*quads) {
                cui_vec2 positions[4];
                for (int k=0;k<4;++k)
                    positions[k]={rect.x+sprite.corners[k].x*scale,rect.y+sprite.corners[k].y*scale};
                cui_draw_effect_quad(ui,sprite.texture,positions,sprite.upright ? uprightUV : particleUV,sprite.tint,sprite.blend);
            }
        };
        effects(model.art.weather);
        if (const auto* grid=model.art.light) {
            const auto triangle=[&](const std::array<int,3>& vertices) {
                cui_vec2 points[3];
                cui_color colors[3];
                for (int k=0;k<3;++k) {
                    const int index=vertices[k];
                    points[k]={rect.x+(index%18)*48*scale,rect.y+(index/18)*32*scale};
                    const auto& rgb=grid->samples[index];
                    colors[k]={std::uint8_t(rgb[0]),std::uint8_t(rgb[1]),std::uint8_t(rgb[2]),255};
                }
                cui_draw_multiply_triangle(ui,points,colors);
            };
            for (int y=0;y<18;++y) for (int x=0;x<17;++x) {
                const int a=y*18+x, b=a+1, d=a+18, c=d+1;
                if (grid->diagonals[y*17+x]) {
                    triangle({a,b,c});
                    triangle({a,c,d});
                } else {
                    triangle({a,b,d});
                    triangle({b,c,d});
                }
            }
        }
        effects(model.art.levelEffects);
        effects(model.art.spellEffects);
        const auto captions=[&](const std::vector<SceneCaption>* lines) {
            if (!lines) return;
            for (const auto& line:*lines) {
                const auto [x,y,w,h]=line.rect;
                const cui_rect at{rect.x+x*scale,rect.y+y*scale,w*scale,h*scale};
                if (line.texture) cui_draw_image(ui,line.texture,at,{0,0,1,1},{},line.color);
                else cui_draw_rect_premultiplied(ui,at,line.color);
            }
        };
        captions(model.art.names);
        if (model.art.groundLabels) for (const auto& label:*model.art.groundLabels) {
            const auto at=[&](int dx,int dy) {
                return cui_rect{rect.x+(label.x+dx)*scale,rect.y+(label.y+dy)*scale,label.width*scale,15*scale};
            };
            cui_draw_rect_premultiplied(ui,at(0,0),{80,60,40,150});
            cui_draw_image(ui,label.texture,at(-1,2),{0,0,1,1},{},{0,0,0,255});
            cui_draw_image(ui,label.texture,at(0,1),{0,0,1,1},{},{255,255,125,255});
        }
        captions(model.art.speech);
        for (std::size_t i=model.art.overlayStart;i<sprites.size();++i) drawSprite(sprites[i]);
        cui_pop_clip(ui);
        return;
    }
    const auto corners=model.legacyHud ? cui_corners{} : cui_corners{10,10,10,10};
    cui_draw_rect(ui, rect, corners, {22, 33, 47, 255});
    cui_push_clip(ui, rect);
    const float cx = rect.x + rect.w / 2, cy = rect.y + rect.h / 2;
    const float pitch = SceneCellPitch*(model.legacyHud ? rect.w/800 : 1);
    const float padding = 6;
    const float markerY = cy + 10;
    if (model.terrainReady) {
        constexpr int columns = mir3_cui::MapTerrain::ViewColumns;
        constexpr int rows = mir3_cui::MapTerrain::ViewRows;
        static const cui_color cellColors[] = {{22, 33, 47, 255}, {40, 68, 66, 255}, {69, 63, 72, 255},
            {45, 104, 92, 255}, {134, 88, 50, 255}};
        for (int row = 0; row < rows; ++row) {
            const float y = markerY + pitch * (row - rows / 2) - pitch / 2;
            if (y + pitch < rect.y || y >= rect.y + rect.h) continue;
            for (int column = 0; column < columns; ++column) {
                const float x = cx + pitch * (column - columns / 2) - pitch / 2;
                if (x + pitch < rect.x || x >= rect.x + rect.w) continue;
                const auto cell = static_cast<unsigned>(model.ground[row * columns + column]);
                if (cell) cui_draw_rect(ui, {x + 1, y + 1, pitch - 2, pitch - 2}, {}, cellColors[cell]);
            }
        }
    } else {
        const cui_color grid{38, 53, 70, 255};
        for (float x = cx - pitch * static_cast<int>((rect.w / 2 - padding) / pitch); x < rect.x + rect.w - padding; x += pitch)
            cui_draw_line(ui, {x, rect.y + padding}, {x, rect.y + rect.h - padding}, 1, grid);
        for (float y = cy - pitch * static_cast<int>((rect.h / 2 - padding) / pitch); y < rect.y + rect.h - padding; y += pitch)
            cui_draw_line(ui, {rect.x + padding, y}, {rect.x + rect.w - padding, y}, 1, grid);
    }

    static const cui_vec2 directions[] = {{0,-1},{0.7f,-0.7f},{1,0},{0.7f,0.7f},
        {0,1},{-0.7f,0.7f},{-1,0},{-0.7f,-0.7f}};
    const auto direction = directions[model.facing];
    for (const auto& entry : *model.players) {
        const auto& player = entry.second;
        const float px = cx + pitch * (static_cast<int>(player.x) - static_cast<int>(model.x));
        const float py = markerY + pitch * (static_cast<int>(player.y) - static_cast<int>(model.y));
        if (px < rect.x - pitch || px > rect.x + rect.w + pitch || py < rect.y - pitch || py > rect.y + rect.h + pitch)
            continue;
        if (player.dead) {
            cui_draw_line(ui, {px - 8, py}, {px + 8, py}, 3, {187, 147, 149, 255});
        } else if (player.npcType) {
            cui_draw_circle(ui, {px, py}, 11, {115, 83, 57, 255});
            cui_draw_icon(ui, CUI_ICON_MESSAGE_SQUARE, {px - 10, py - 11, 20, 20}, 15, {250, 218, 165, 255});
        } else {
            constexpr float scale = 0.7f;
            const auto facing = directions[player.facing];
            cui_draw_circle(ui, {px, py}, 9, {67, 66, 106, 255});
            cui_draw_circle(ui, {px + facing.x * 13, py + facing.y * 12}, 2, {195, 182, 238, 255});
            PaintCharacterFigure(ui, {px - 24 * scale, py - 44 * scale, 56 * scale, 44 * scale},
                {player.appearance, false}, scale);
        }
        if (!player.name.empty())
            cui_draw_text(ui, CUI_FONT_BODY, 11, {px + 13, py - 7}, {224, 215, 247, 255}, player.name.c_str(), -1);
    }
    if (model.pending.deadline) {
        const float targetX = cx + (static_cast<int>(model.pending.x) - static_cast<int>(model.x)) * pitch;
        const float targetY = markerY + (static_cast<int>(model.pending.y) - static_cast<int>(model.y)) * pitch;
        cui_draw_circle(ui, {targetX, targetY}, 7, {229, 177, 82, 255});
    }
    cui_draw_circle(ui, {cx, markerY}, 18, {28, 72, 83, 255});
    cui_draw_circle(ui, {cx + direction.x * 25, markerY + direction.y * 21}, 3, {110, 237, 201, 255});
    PaintCharacterFigure(ui, {cx - 24, markerY - 43, 56, 44}, model.portrait);
    if (!model.legacyHud)
        cui_draw_text(ui, CUI_FONT_MEDIUM, 13, {rect.x + 10, rect.y + 8}, {233, 241, 249, 255}, model.location, -1);
    cui_draw_text(ui, CUI_FONT_BODY, 12, {cx + 32, markerY - 6}, {187, 223, 220, 255}, model.name, -1);
    cui_pop_clip(ui);
}

void DrawLocalScene(cui_ctx* ui, LoginClient& client, bool interactive, bool nativeInventory, bool legacyHud,
                   SceneMapArt art)
{
    auto& scene = client.scene;
    LocalSceneDrawing view{};
    view.portrait = {client.characterAppearance, !client.equipment.slots[4].key.name.empty()};
    view.x = scene.x;
    view.y = scene.y;
    view.facing = scene.facing;
    view.pending = scene.pending;
    view.terrainReady = scene.terrain.Ready();
    view.legacyHud = legacyHud;
    view.art = art;
    view.ground = scene.ground;
    view.players = &scene.players;
    const auto& map = scene.mapDescription.empty() ? scene.mapId : scene.mapDescription;
    std::snprintf(view.location, sizeof view.location, "%s  (%u, %u)", map.empty() ? "Scene" : map.c_str(), view.x, view.y);
    std::snprintf(view.name, sizeof view.name, "%s", client.gameCharacter.c_str());
    const auto now=GetTickCount64();
    const bool delayed=scene.InputDelayed(now);
    cui_push_disabled(ui,delayed);
    const auto canvas = cui_canvas(ui, "local-scene", CUI_FILL,
        legacyHud ? CUI_FILL : CUI_PX(client.inventory.shortcutsVisible ? 70 : 96),
        PaintLocalScene, &view, sizeof view);
    cui_pop_disabled(ui);
    const bool cutRelease=scene.crouchPointerCaptured && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool swingRelease=scene.meleePointerCaptured && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool walkRelease=scene.walkPointerAt && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    if (!cui_mouse_down(ui,CUI_MOUSE_LEFT)) {
        scene.crouchHoldAt=0;
        scene.crouchPointerCaptured=false;
        scene.meleePointerCaptured=false;
        scene.walkPointerAt=0;
    }
    if (!cui_mouse_down(ui,CUI_MOUSE_RIGHT)) scene.runPointerCaptured=false;
    if (!interactive || delayed || !view.terrainReady || !canvas.hovered || cutRelease || swingRelease || walkRelease) return;
    if (nativeInventory && (client.inventory.heldGold || client.HeldItem())) {
        if (cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) client.BeginHeldDrop();
        return;
    }
    if (client.trade.returnSlot>=0) return;
    if (!legacyHud) cui_tooltip(ui, "Left-click to walk. Right-click to run or turn.");
    const bool rightPress=cui_mouse_clicked(ui,CUI_MOUSE_RIGHT);
    if (rightPress && art.sprites) scene.runPointerCaptured=true;
    const bool run=rightPress || scene.runPointerCaptured;
    const auto mods=cui_key_mods(ui);
    const bool altCut=art.sprites && (mods&(CUI_MOD_ALT|CUI_MOD_SHIFT))==CUI_MOD_ALT;
    const bool cutPress=!run && altCut && cui_mouse_clicked(ui,CUI_MOUSE_LEFT);
    const bool cutRepeat=altCut && scene.crouchHoldAt && now-scene.crouchHoldAt>=300;
    const bool swingHeld=art.sprites && !run && !altCut && canvas.pressed &&
        (scene.meleePointerCaptured || cui_mouse_clicked(ui,CUI_MOUSE_LEFT));
    const bool walkHeld=canvas.pressed && scene.walkPointerAt && now-scene.walkPointerAt>=300;
    if (cutPress) scene.crouchPointerCaptured=true;
    if (!canvas.clicked && !run && !cutPress && !cutRepeat && !swingHeld && !walkHeld) return;
    if (art.sprites) {
        const float scale=canvas.rect.w/800;
        const float viewX=canvas.mouse.x/scale, viewY=canvas.mouse.y/scale;
        if (!run && (canvas.clicked || cutPress || swingHeld)) {
            const auto& npcs=*art.npcs;
            const auto hit=std::find_if(npcs.rbegin(),npcs.rend(),[&](const SceneActorHit& npc) {
                const auto& r=npc.bounds;
                return viewX>=r.x && viewX<r.x+r.w && viewY>=r.y && viewY<r.y+r.h;
            });
            if (hit!=npcs.rend() && ((canvas.clicked || cutPress) ? client.ClickNpcActor(hit->actor) : swingHeld)) return;
        }
        int x=static_cast<int>(viewX+art.cameraX)/48;
        int y=static_cast<int>(viewY+art.cameraY)/32;
        if (!run && (canvas.clicked || cutPress || swingHeld) && client.ClickNpc(x,y)) return;
        if (!run && (cutPress || cutRepeat)) {
            if (cutPress) scene.crouchHoldAt=now;
            const auto hit=[&](const SceneActorHit& actor) {
                const auto& r=actor.bounds;
                return viewX>=r.x && viewX<r.x+r.w && viewY>=r.y && viewY<r.y+r.h;
            };
            std::uint32_t monsterCorpse=0, humanCorpse=0;
            if (art.corpses && art.targets && std::none_of(art.targets->begin(),art.targets->end(),hit)) {
                for (const auto& candidate:*art.corpses) {
                    const auto& actor=scene.players.at(candidate.actor);
                    if (!hit(candidate) || !scene.CanButcher(actor)) continue;
                    auto& selected=candidate.monster ? monsterCorpse : humanCorpse;
                    selected=(std::max)(selected,candidate.actor);
                }
            }
            client.CrouchInScene(x,y,humanCorpse ? humanCorpse : monsterCorpse);
            return;
        }
        bool approachActor=false;
        if (!run && art.targets) {
            const auto& targets=*art.targets;
            const auto hit=std::find_if(targets.rbegin(),targets.rend(),[&](const SceneActorHit& target) {
                const auto& r=target.bounds;
                return viewX>=r.x && viewX<r.x+r.w && viewY>=r.y && viewY<r.y+r.h;
            });
            if (client.MeleeInScene(x,y,hit==targets.rend() ? 0 : hit->actor,(mods&CUI_MOD_SHIFT)!=0)) {
                if (swingHeld) {
                    scene.meleePointerCaptured=true;
                    cui_request_wakeup(ui,0.016f);
                }
                return;
            }
            const auto weapon=client.characterAppearance.weapon;
            if (hit!=targets.rend() && (weapon<34 || weapon>37)) {
                const auto& target=scene.players.at(hit->actor);
                x=int(target.x);
                y=int(target.y);
                approachActor=true;
            }
        }
        if (swingHeld) scene.walkPointerAt=now;
        if (scene.walkPointerAt || scene.runPointerCaptured) cui_request_wakeup(ui,0.05f);
        client.MoveInScene(x,y,run,approachActor,rightPress);
        return;
    }
    const float pitch=SceneCellPitch*(legacyHud ? canvas.rect.w/800 : 1);
    const int x = static_cast<int>(view.x) +
        static_cast<int>(std::floor((canvas.mouse.x - canvas.rect.w / 2) / pitch + 0.5f));
    const int y = static_cast<int>(view.y) +
        static_cast<int>(std::floor((canvas.mouse.y - canvas.rect.h / 2 - 10) / pitch + 0.5f));
    if (!run && client.ClickNpc(x, y)) return;
    client.MoveInScene(x,y,run,false,rightPress);
}

} // namespace mir3_client
