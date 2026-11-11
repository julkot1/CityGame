#include "ui/TileInspectorHUD.h"
#include "ui/UIStyle.h"
#include "world/Chunk.h"
#include <imgui.h>

namespace Magnum {

TileInspectorHUD::TileInspectorHUD() = default;

void TileInspectorHUD::draw(const Vector2i& windowSize, const World& world, const World::RaycastHit& selectedHit) {
    if(!selectedHit.hit) return;

    const Chunk* ch = world.chunk(selectedHit.chunkCoord);
    if(!ch) return;

    const Tile& tile = ch->tile(selectedHit.localTileCoord.x(), selectedHit.localTileCoord.y());
    const Entity* entity = ch->entityAt(selectedHit.localTileCoord.x(), selectedHit.localTileCoord.y());

    constexpr float padding = 16.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImVec2 bottomLeftPos(padding, windowSize.y() - padding);
    ImGui::SetNextWindowPos(bottomLeftPos, ImGuiCond_Always, ImVec2(0.0f, 1.0f)); // Pivot bottom-left
    ImGui::SetNextWindowBgAlpha(0.88f);

    if(ImGui::Begin("##TileInspectorHUD", nullptr, flags)) {
        ImGui::TextColored(UIStyle::Colors::SkyBlue, "SURVEY // TILE DATA");
        ImGui::Separator();

        const char* biomeName = "Unknown";
        ImVec4 biomeColor(1.0f, 1.0f, 1.0f, 1.0f);
        switch(tile.type) {
            case TileType::Grass:
                biomeName = "Grassland Plain";
                biomeColor = ImVec4(0.41f, 0.62f, 0.42f, 1.0f);
                break;
            case TileType::Forest:
                biomeName = "Lush Forest";
                biomeColor = ImVec4(0.26f, 0.48f, 0.35f, 1.0f);
                break;
            case TileType::Rock:
                biomeName = "Mountain Rock";
                biomeColor = ImVec4(0.57f, 0.51f, 0.45f, 1.0f);
                break;
            case TileType::Snow:
                biomeName = "Alpine Snow Peak";
                biomeColor = ImVec4(0.95f, 0.95f, 0.85f, 1.0f);
                break;
            case TileType::Sand:
                biomeName = "Coastal Sand";
                biomeColor = ImVec4(0.84f, 0.77f, 0.63f, 1.0f);
                break;
            case TileType::Water:
                biomeName = "Deep Water";
                biomeColor = ImVec4(0.24f, 0.49f, 0.65f, 1.0f);
                break;
            case TileType::Empty:
            default:
                biomeName = "Empty Void";
                biomeColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                break;
        }

        ImGui::TextColored(biomeColor, "Biome: %s", biomeName);
        ImGui::Text("Elevation: %.2f m", static_cast<double>(tile.height));
        ImGui::Text("World Pos: (%d, %d)", selectedHit.worldTileX, selectedHit.worldTileZ);
        ImGui::TextDisabled("Chunk [%d, %d] Local (%d, %d)",
                            selectedHit.chunkCoord.x(), selectedHit.chunkCoord.y(),
                            selectedHit.localTileCoord.x(), selectedHit.localTileCoord.y());

        ImGui::Separator();

        // Entity / Occupant info
        if(entity) {
            const char* entityName = "Unknown Object";
            ImVec4 entColor = UIStyle::Colors::AmberGold;
            if(entity->type == EntityType::TreeOak) {
                entityName = "Mature Oak Tree";
                entColor = ImVec4(0.45f, 0.85f, 0.35f, 1.0f);
            } else if(entity->type == EntityType::TreePine) {
                entityName = "Alpine Pine Tree";
                entColor = ImVec4(0.30f, 0.70f, 0.45f, 1.0f);
            } else if(entity->type == EntityType::Rock) {
                entityName = "Granite Boulder";
                entColor = ImVec4(0.70f, 0.65f, 0.60f, 1.0f);
            }
            ImGui::TextColored(entColor, "Entity: %s", entityName);
            ImGui::TextDisabled("Scale: %.2fx", static_cast<double>(entity->scale));
        } else {
            ImGui::TextDisabled("Entity: None (Vacant)");
        }

        ImGui::End();
    }
}

}
