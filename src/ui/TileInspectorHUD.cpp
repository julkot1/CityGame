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
    const Entity* entity = world.entityAt(selectedHit.worldTileX, selectedHit.worldTileZ);

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
                biomeName = "Lowland Meadow Plain";
                biomeColor = ImVec4(0.48f, 0.60f, 0.38f, 1.0f);
                break;
            case TileType::Heath:
                biomeName = "Heather Moorland";
                biomeColor = ImVec4(0.68f, 0.58f, 0.42f, 1.0f);
                break;
            case TileType::Forest:
                biomeName = "Woodland";
                biomeColor = ImVec4(0.32f, 0.48f, 0.31f, 1.0f);
                break;
            case TileType::Clay:
                biomeName = "Clay Bank";
                biomeColor = ImVec4(0.72f, 0.50f, 0.36f, 1.0f);
                break;
            case TileType::Rock:
                biomeName = "Basalt & Slate Crag";
                biomeColor = ImVec4(0.58f, 0.60f, 0.65f, 1.0f);
                break;
            case TileType::Snow:
                biomeName = "Highland Frost Peak";
                biomeColor = ImVec4(0.92f, 0.89f, 0.82f, 1.0f);
                break;
            case TileType::Sand:
                biomeName = "Coastal Silt & Shingle";
                biomeColor = ImVec4(0.78f, 0.68f, 0.54f, 1.0f);
                break;
            case TileType::Road:
                biomeName = "Road";
                biomeColor = ImVec4(0.65f, 0.63f, 0.60f, 1.0f);
                break;
            case TileType::Water:
                biomeName = "Deep Canal / Waterway";
                biomeColor = ImVec4(0.28f, 0.54f, 0.64f, 1.0f);
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

        if(entity) {
            const char* entityName = "Unknown Object";
            ImVec4 entColor = UIStyle::Colors::AmberGold;
            switch(entity->type) {
                case EntityType::TreeOak:
                    entityName = "Highland Oak Tree";
                    entColor = ImVec4(0.58f, 0.72f, 0.36f, 1.0f);
                    break;
                case EntityType::TreePine:
                    entityName = "Slender Larch / Pine";
                    entColor = ImVec4(0.34f, 0.64f, 0.44f, 1.0f);
                    break;
                case EntityType::Rock:
                    entityName = "Mossy Slate Boulder";
                    entColor = ImVec4(0.66f, 0.68f, 0.72f, 1.0f);
                    break;
                case EntityType::CoalDeposit:
                    entityName = "Anthracite Coal Seam";
                    entColor = ImVec4(0.55f, 0.58f, 0.65f, 1.0f);
                    break;
                case EntityType::IronDeposit:
                    entityName = "Hematite Iron Ore Vein";
                    entColor = ImVec4(0.88f, 0.45f, 0.30f, 1.0f);
                    break;
                case EntityType::CopperDeposit:
                    entityName = "Verdigris Copper Vein";
                    entColor = ImVec4(0.36f, 0.80f, 0.66f, 1.0f);
                    break;
                case EntityType::AncientRuins:
                    entityName = "Old World Stone Ruins";
                    entColor = ImVec4(0.82f, 0.78f, 0.68f, 1.0f);
                    break;
                case EntityType::RoadCobblestone:
                    entityName = "Cobblestone Street";
                    entColor = UIStyle::Colors::AmberGold;
                    break;
                case EntityType::House2x2:
                    entityName = "Townhouse";
                    entColor = ImVec4(0.95f, 0.72f, 0.35f, 1.0f);
                    break;
                case EntityType::HouseStandalone2x2:
                    entityName = "Standalone Cottage";
                    entColor = ImVec4(0.95f, 0.72f, 0.35f, 1.0f);
                    break;
                case EntityType::CityCenter2x3:
                    entityName = "Victorian City Center";
                    entColor = ImVec4(0.95f, 0.85f, 0.45f, 1.0f);
                    break;
                case EntityType::CityPlaza4x2:
                    entityName = "Victorian City Plaza";
                    entColor = ImVec4(0.55f, 0.82f, 0.90f, 1.0f);
                    break;
                case EntityType::Warehouse4x4:
                    entityName = "Industrial Warehouse (4x4)";
                    entColor = ImVec4(0.85f, 0.65f, 0.45f, 1.0f);
                    break;
                default:
                    break;
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
