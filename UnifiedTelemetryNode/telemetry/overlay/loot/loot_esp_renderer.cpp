#include "../core/overlay_menu.hpp"
#include "../entities/entity_aliases.hpp"
#include "loot_cluster_renderer.hpp"
#include "loot_debug_renderer.hpp"
#include "loot_source_merge.hpp"
#include "../core/overlay_texture_cache.hpp"
#include "../vehicle/vehicle_resolver.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/Common/Data.h"
#include <protec/skCrypt.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <vector>

namespace {

bool ShouldDrawVehicle(const OverlayMenu& menu, const std::string& name) {
    const VehicleResolver::VehicleInfo info = VehicleResolver::Resolve(name);
    switch (info.CategoryId) {
    case VehicleResolver::Category::Uaz: return menu.loot_vehicle_uaz;
    case VehicleResolver::Category::Dacia: return menu.loot_vehicle_dacia;
    case VehicleResolver::Category::Buggy: return menu.loot_vehicle_buggy;
    case VehicleResolver::Category::Bike: return menu.loot_vehicle_bike;
    case VehicleResolver::Category::Boat: return menu.loot_vehicle_boat;
    case VehicleResolver::Category::Brdm: return menu.loot_vehicle_brdm;
    case VehicleResolver::Category::Scooter: return menu.loot_vehicle_scooter;
    case VehicleResolver::Category::Snow: return menu.loot_vehicle_snow;
    case VehicleResolver::Category::Tuk: return menu.loot_vehicle_tuk;
    case VehicleResolver::Category::Bus: return menu.loot_vehicle_bus;
    case VehicleResolver::Category::Truck: return menu.loot_vehicle_truck;
    case VehicleResolver::Category::Train: return menu.loot_vehicle_train;
    case VehicleResolver::Category::Mirado: return menu.loot_vehicle_mirado;
    case VehicleResolver::Category::Pickup: return menu.loot_vehicle_pickup;
    case VehicleResolver::Category::Rony: return menu.loot_vehicle_rony;
    case VehicleResolver::Category::Blanc: return menu.loot_vehicle_blanc;
    case VehicleResolver::Category::Air: return menu.loot_vehicle_air;
    default: return true;
    }
}

Vector3 PredictProjectilePoint(const ItemData& item, float t) {
    constexpr float kGravityCm = 980.0f;
    return item.Position + (item.Velocity * t) + Vector3{ 0.0f, 0.0f, -0.5f * kGravityCm * t * t };
}

FVector ToFVector(const Vector3& value) {
    return FVector(value.x, value.y, value.z);
}

Vector3 ToVector3(const FVector& value) {
    return Vector3(value.X, value.Y, value.Z);
}

bool RaycastSceneHit(const Vector3& origin, const Vector3& target, Vector3& hitPoint) {
    const Vector3 segment = target - origin;
    const float maxDistance = segment.Length();
    if (maxDistance <= 1.0f) return false;

    const Vector3 dir = segment * (1.0f / maxDistance);
    constexpr float kEpsilon = 0.0001f;
    float closest = FLT_MAX;
    bool hit = false;

    std::shared_lock<std::shared_mutex> lock(GameData.Actors.ClonedMapMutex, std::try_to_lock);
    if (!lock.owns_lock()) return false;

    for (const TriangleMeshData& mesh : GameData.Actors.ClonedMapMeshes) {
        if (mesh.Vertices.empty() || mesh.Indices.size() < 3) continue;

        for (size_t i = 0; i + 2 < mesh.Indices.size(); i += 3) {
            const uint32_t i0 = mesh.Indices[i];
            const uint32_t i1 = mesh.Indices[i + 1];
            const uint32_t i2 = mesh.Indices[i + 2];
            if (i0 >= mesh.Vertices.size() || i1 >= mesh.Vertices.size() || i2 >= mesh.Vertices.size()) continue;

            const Vector3& v0 = mesh.Vertices[i0];
            const Vector3& v1 = mesh.Vertices[i1];
            const Vector3& v2 = mesh.Vertices[i2];

            const Vector3 edge1 = v1 - v0;
            const Vector3 edge2 = v2 - v0;
            const Vector3 pvec = dir.cross(edge2);
            const float det = edge1.dot(pvec);
            if (std::fabs(det) < kEpsilon) continue;

            const float invDet = 1.0f / det;
            const Vector3 tvec = origin - v0;
            const float u = tvec.dot(pvec) * invDet;
            if (u < 0.0f || u > 1.0f) continue;

            const Vector3 qvec = tvec.cross(edge1);
            const float v = dir.dot(qvec) * invDet;
            if (v < 0.0f || (u + v) > 1.0f) continue;

            const float distance = edge2.dot(qvec) * invDet;
            if (distance >= 0.0f && distance <= maxDistance && distance < closest) {
                closest = distance;
                hit = true;
            }
        }
    }

    if (!hit) return false;
    hitPoint = origin + (dir * closest);
    return true;
}

void DrawMapMeshDebugEsp(ImDrawList* draw) {
    if (!draw || !g_Menu.debug_map_mesh_esp) return;

    constexpr float kMaxDrawDistanceCm = 18000.0f;
    constexpr float kMaxDrawDistanceSq = kMaxDrawDistanceCm * kMaxDrawDistanceCm;
    constexpr size_t kMaxDrawTriangles = 2600;
    constexpr size_t kMeshStep = 1;

    size_t meshCount = 0;
    size_t totalTriangles = 0;
    size_t drawnTriangles = 0;
    bool lockBusy = false;

    std::shared_lock<std::shared_mutex> lock(GameData.Actors.ClonedMapMutex, std::try_to_lock);
    if (!lock.owns_lock()) {
        lockBusy = true;
    } else {
        meshCount = GameData.Actors.ClonedMapMeshes.size();
        for (size_t m = 0; m < GameData.Actors.ClonedMapMeshes.size(); m += kMeshStep) {
            const TriangleMeshData& mesh = GameData.Actors.ClonedMapMeshes[m];
            if (mesh.Vertices.empty() || mesh.Indices.size() < 3) continue;

            totalTriangles += mesh.Indices.size() / 3;

            Vector3 center{0.0f, 0.0f, 0.0f};
            const size_t sampleCount = (std::min)(mesh.Vertices.size(), static_cast<size_t>(8));
            for (size_t i = 0; i < sampleCount; ++i) center = center + mesh.Vertices[i];
            if (sampleCount > 0) center = center * (1.0f / static_cast<float>(sampleCount));
            const Vector3 delta = center - G_CameraLocation;
            const float distSq = (delta.x * delta.x) + (delta.y * delta.y) + (delta.z * delta.z);
            if (!G_CameraLocation.IsZero() && distSq > kMaxDrawDistanceSq) continue;

            for (size_t i = 0; i + 2 < mesh.Indices.size() && drawnTriangles < kMaxDrawTriangles; i += 3) {
                const uint32_t i0 = mesh.Indices[i];
                const uint32_t i1 = mesh.Indices[i + 1];
                const uint32_t i2 = mesh.Indices[i + 2];
                if (i0 >= mesh.Vertices.size() || i1 >= mesh.Vertices.size() || i2 >= mesh.Vertices.size()) continue;

                Vector2 s0{}, s1{}, s2{};
                if (!telemetryContext::WorldToScreen(mesh.Vertices[i0], s0) ||
                    !telemetryContext::WorldToScreen(mesh.Vertices[i1], s1) ||
                    !telemetryContext::WorldToScreen(mesh.Vertices[i2], s2)) {
                    continue;
                }

                const ImU32 edge = IM_COL32(0, 235, 255, 145);
                const ImU32 fill = IM_COL32(0, 160, 255, 18);
                draw->AddTriangleFilled(ImVec2(s0.x, s0.y), ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), fill);
                draw->AddTriangle(ImVec2(s0.x, s0.y), ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), edge, 1.0f);
                ++drawnTriangles;
            }

            if (drawnTriangles >= kMaxDrawTriangles) break;
        }
    }

    char status[160] = {};
    std::snprintf(status, sizeof(status), "MapMesh: %zu meshes | %zu tris | drawn %zu%s",
        meshCount, totalTriangles, drawnTriangles, lockBusy ? " | lock busy" : "");
    // draw->AddText(ImGui::GetFont(), 14.0f, ImVec2(21.0f, 121.0f), IM_COL32(0, 0, 0, 220), status);
    // draw->AddText(ImGui::GetFont(), 14.0f, ImVec2(20.0f, 120.0f), IM_COL32(0, 235, 255, 245), status);

    if (!G_MapMeshDebugStatus.empty()) {
        // draw->AddText(ImGui::GetFont(), 13.0f, ImVec2(21.0f, 139.0f), IM_COL32(0, 0, 0, 220), G_MapMeshDebugStatus.c_str());
        // draw->AddText(ImGui::GetFont(), 13.0f, ImVec2(20.0f, 138.0f), IM_COL32(255, 220, 80, 245), G_MapMeshDebugStatus.c_str());
    }
}

Vector3 PredictProjectileEnd(const ItemData& item) {
    const float t = item.HasTrajectory
        ? std::clamp(item.TimeTillExplosion, 0.25f, 4.0f)
        : 0.0f;
    return t > 0.0f ? PredictProjectilePoint(item, t) : item.Position;
}

void DrawWorldCircle(ImDrawList* draw, const Vector3& center, float radius, ImU32 color) {
    if (radius <= 0.0f) return;

    constexpr int kSegments = 48;
    Vector2 prev{};
    bool hasPrev = false;
    for (int i = 0; i <= kSegments; ++i) {
        const float a = (static_cast<float>(i) / static_cast<float>(kSegments)) * 6.28318530718f;
        const Vector3 world = center + Vector3{ std::cos(a) * radius, std::sin(a) * radius, 0.0f };
        Vector2 screen{};
        const bool visible = telemetryContext::WorldToScreen(world, screen);
        if (visible && hasPrev) {
            draw->AddLine(ImVec2(prev.x, prev.y), ImVec2(screen.x, screen.y), color, 1.6f);
        }
        prev = screen;
        hasPrev = visible;
    }
}

void DrawProjectileThreat(ImDrawList* draw, const ItemData& item, const Vector2& itemScreen, ImU32 color) {
    const Vector3 endWorld = PredictProjectileEnd(item);
    Vector2 endScreen{};
    const bool hasEnd = telemetryContext::WorldToScreen(endWorld, endScreen);

    if (item.HasTrajectory) {
        std::vector<ImVec2> points;
        const float duration = std::clamp(item.TimeTillExplosion, 0.25f, 4.0f);
        constexpr int kSteps = 18;
        points.reserve(kSteps + 1);
        for (int i = 0; i <= kSteps; ++i) {
            const float t = duration * (static_cast<float>(i) / static_cast<float>(kSteps));
            Vector2 screen{};
            if (telemetryContext::WorldToScreen(PredictProjectilePoint(item, t), screen)) {
                points.emplace_back(screen.x, screen.y);
            }
        }
        if (points.size() >= 2) {
            draw->AddPolyline(points.data(), static_cast<int>(points.size()), IM_COL32(0, 0, 0, 150), 0, 3.6f);
            draw->AddPolyline(points.data(), static_cast<int>(points.size()), IM_COL32(255, 210, 70, 235), 0, 1.8f);
        }
    }

    DrawWorldCircle(draw, endWorld, item.BlastRadius, IM_COL32(255, 45, 45, 180));

    if (hasEnd) {
        draw->AddCircleFilled(ImVec2(endScreen.x, endScreen.y), 4.0f, IM_COL32(255, 40, 40, 220), 16);
        draw->AddCircle(ImVec2(endScreen.x, endScreen.y), 8.0f, IM_COL32(255, 40, 40, 160), 18, 1.4f);
        draw->AddLine(ImVec2(itemScreen.x, itemScreen.y), ImVec2(endScreen.x, endScreen.y),
            IM_COL32(255, 210, 70, 120), 1.0f);
    }

    char label[96] = {};
    if (item.TimeTillExplosion > 0.03f && item.TimeTillExplosion < 120.0f) {
        std::snprintf(label, sizeof(label), "%s %.1fs [%dm]", item.Name.c_str(), item.TimeTillExplosion,
            static_cast<int>(item.Distance));
    } else {
        std::snprintf(label, sizeof(label), "%s [%dm]", item.Name.c_str(), static_cast<int>(item.Distance));
    }
    const float fontSize = g_Menu.loot_distance_font_size + 1.0f;
    const ImVec2 size = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
    const ImVec2 pos(itemScreen.x - size.x * 0.5f, itemScreen.y - 14.0f);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 190), label);
    draw->AddText(ImGui::GetFont(), fontSize, pos, color, label);
}

bool IsHeldThrowableName(const std::string& name) {
    return name.find(skCrypt("Nade")) != std::string::npos ||
           name.find(skCrypt("Grenade")) != std::string::npos ||
           name.find(skCrypt("Molotov")) != std::string::npos ||
           name.find(skCrypt("Bomb")) != std::string::npos ||
           name == skCrypt("C4") ||
           name == skCrypt("Decoy");
}

float HeldThrowableSpeed(const std::string& name) {
    if (name.find(skCrypt("Molotov")) != std::string::npos) return 1050.0f;
    if (name.find(skCrypt("Smoke")) != std::string::npos) return 1150.0f;
    if (name.find(skCrypt("Bluezone")) != std::string::npos) return 1050.0f;
    if (name.find(skCrypt("Sticky")) != std::string::npos) return 900.0f;
    return 1250.0f;
}

float HeldThrowableBlastRadius(const std::string& name) {
    if (name.find(skCrypt("Molotov")) != std::string::npos) return 350.0f;
    if (name.find(skCrypt("Bluezone")) != std::string::npos) return 650.0f;
    if (name.find(skCrypt("C4")) != std::string::npos) return 2500.0f;
    if (name.find(skCrypt("Sticky")) != std::string::npos) return 650.0f;
    return 550.0f;
}

Vector3 CameraForward() {
    const Vector3 rot = !G_LocalControlRotation.IsZero() ? G_LocalControlRotation : G_CameraRotation;
    const float pitchRad = rot.x * 0.017453292519943f;
    const float yawRad = rot.y * 0.017453292519943f;
    const float cp = std::cos(pitchRad);
    return Vector3{
        static_cast<float>(std::cos(yawRad) * cp),
        static_cast<float>(std::sin(yawRad) * cp),
        static_cast<float>(std::sin(pitchRad))
    }.normalized();
}

void DrawHeldThrowablePrediction(ImDrawList* draw) {
    if (!draw || !g_Menu.esp_grenade_prediction || !IsHeldThrowableName(G_LocalWeaponName)) return;
    if (G_CameraLocation.IsZero() && G_LocalPlayerPos.IsZero()) return;

    constexpr float kGravityCm = 980.0f;
    const Vector3 forward = CameraForward();
    if (forward.IsZero()) return;

    const Vector3 playerOrigin = !G_LocalPlayerPos.IsZero() ? G_LocalPlayerPos : G_CameraLocation;
    const Vector3 start = playerOrigin + Vector3{0.0f, 0.0f, 115.0f} + (forward * 70.0f);
    const float speed = HeldThrowableSpeed(G_LocalWeaponName);
    const Vector3 velocity = forward * speed;
    constexpr float duration = 6.0f;
    constexpr int steps = 72;
    const float fallbackGroundZ = !G_LocalPlayerPos.IsZero() ? (G_LocalPlayerPos.z + 6.0f) : (start.z - 115.0f);

    std::vector<ImVec2> points;
    points.reserve(steps + 1);
    Vector3 previousWorld = start;
    Vector3 landingWorld = start;
    bool hitLanding = false;
    for (int i = 0; i <= steps; ++i) {
        const float t = duration * (static_cast<float>(i) / static_cast<float>(steps));
        Vector3 world = start + (velocity * t) + Vector3{0.0f, 0.0f, -0.5f * kGravityCm * t * t};

        if (i > 0) {
            Vector3 hitPoint{};
            if (RaycastSceneHit(previousWorld, world, hitPoint)) {
                world = hitPoint;
                hitLanding = true;
            } else if (i > 2 && previousWorld.z > fallbackGroundZ && world.z <= fallbackGroundZ) {
                const float denom = previousWorld.z - world.z;
                const float alpha = denom > 0.001f ? std::clamp((previousWorld.z - fallbackGroundZ) / denom, 0.0f, 1.0f) : 1.0f;
                world = previousWorld + ((world - previousWorld) * alpha);
                world.z = fallbackGroundZ;
                hitLanding = true;
            }
        }

        Vector2 screen{};
        if (telemetryContext::WorldToScreen(world, screen)) {
            points.emplace_back(screen.x, screen.y);
        }
        landingWorld = world;
        if (hitLanding) break;
        previousWorld = world;
    }

    if (points.size() < 2) return;

    draw->AddPolyline(points.data(), static_cast<int>(points.size()), IM_COL32(0, 0, 0, 170), 0, 4.4f);
    draw->AddPolyline(points.data(), static_cast<int>(points.size()), IM_COL32(255, 226, 90, 240), 0, 2.1f);

    DrawWorldCircle(draw, landingWorld, HeldThrowableBlastRadius(G_LocalWeaponName), IM_COL32(255, 70, 45, 155));
    Vector2 endScreen{};
    if (telemetryContext::WorldToScreen(landingWorld, endScreen)) {
        draw->AddCircleFilled(ImVec2(endScreen.x, endScreen.y), 4.5f, IM_COL32(255, 55, 40, 230), 16);
        draw->AddCircle(ImVec2(endScreen.x, endScreen.y), 9.0f, IM_COL32(255, 55, 40, 170), 18, 1.5f);

        char label[96] = {};
        std::snprintf(label, sizeof(label), "%s landing", G_LocalWeaponName.c_str());
        const float fontSize = g_Menu.loot_distance_font_size + 1.0f;
        const ImVec2 size = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
        const ImVec2 pos(endScreen.x - size.x * 0.5f, endScreen.y + 10.0f);
        draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 190), label);
        draw->AddText(ImGui::GetFont(), fontSize, pos, IM_COL32(255, 226, 90, 245), label);
    }
}

} // namespace

void OverlayMenu::RenderLootEsp(ImDrawList* draw) {
    if (!draw || !esp_toggle) return;
            DrawMapMeshDebugEsp(draw);
            DrawHeldThrowablePrediction(draw);

            std::vector<ItemData> allLoot = LootSourceMerge::BuildAllLoot();

            struct VehicleDrawKey {
                std::string IconName;
                Vector2 Screen;
            };
            std::vector<VehicleDrawKey> seenVehicles;
            auto HasDuplicateVehicle = [&](const std::string& iconName, const Vector2& screen) {
                constexpr float kDuplicateRadiusSq = 36.0f * 36.0f;
                for (const auto& seen : seenVehicles) {
                    if (seen.IconName != iconName) continue;
                    const float dx = seen.Screen.x - screen.x;
                    const float dy = seen.Screen.y - screen.y;
                    if ((dx * dx) + (dy * dy) <= kDuplicateRadiusSq) return true;
                }
                return false;
            };
            auto MarkVehicleSeen = [&](const std::string& iconName, const Vector2& screen) {
                seenVehicles.push_back({ iconName, screen });
            };

            std::vector<LootClusterRenderer::Entry> lootEntries;
            for (const auto& item : allLoot) {
                if (item.Distance <= 0) continue;

                bool should_draw = false;
                ImU32 col = IM_COL32(200, 200, 200, 255); // Default Loot

                if (item.RenderType == ItemRenderType::Vehicle) {
                    if (g_Menu.esp_vehicles &&
                        !VehicleResolver::ShouldHideAtDistance(item.Distance) &&
                        item.Distance < static_cast<float>(g_Menu.vehicle_max_dist) &&
                        ShouldDrawVehicle(*this, item.Name)) {
                        should_draw = true; col = IM_COL32(0, 255, 255, 255);
                    }
                } else if (item.RenderType == ItemRenderType::AirDrop) {
                    if (g_Menu.esp_airdrops) { should_draw = true; col = IM_COL32(255, 50, 50, 255); }
                } else if (item.RenderType == ItemRenderType::DeadBox) {
                    if (g_Menu.esp_deadboxes && item.Distance < 200.0f) { should_draw = true; col = IM_COL32(255, 140, 0, 255); }
                } else if (item.RenderType == ItemRenderType::Projectile) {
                    if (g_Menu.esp_projectile_tracer) {
                        should_draw = true; col = IM_COL32(255, 0, 0, 255); // BRIGHT RED FOR DANGER
                    }
                } else { // Generic items
                    if (g_Menu.esp_items && item.Distance > 5.0f && item.Distance < static_cast<float>(g_Menu.loot_max_dist)) {
                        const std::string& id = item.Name; // e.g. Item_Weapon_AK47_C

                        // --- 1. WEAPONS: ASSAULT RIFLES ---
                        if      (id == skCrypt("Item_Weapon_HK416_C")) { if(g_Menu.loot_weapon_hk416) { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_AK47_C"))  { if(g_Menu.loot_weapon_ak47)  { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_BerylM762_C")) { if(g_Menu.loot_weapon_beryl) { should_draw = true; col = IM_COL32(255, 120, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_SCAR-L_C")) { if(g_Menu.loot_weapon_scar)  { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_AUG_C"))    { if(g_Menu.loot_weapon_aug)   { should_draw = true; col = IM_COL32(255, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_Groza_C"))  { if(g_Menu.loot_weapon_groza) { should_draw = true; col = IM_COL32(255, 100, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_ACE32_C"))  { if(g_Menu.loot_weapon_ace32) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_FAMASG2_C")) { if(g_Menu.loot_weapon_famas) { should_draw = true; col = IM_COL32(255, 100, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_G36C_C"))   { if(g_Menu.loot_weapon_g36c)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_QBZ95_C"))  { if(g_Menu.loot_weapon_qbz)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_K2_C"))     { if(g_Menu.loot_weapon_k2)    { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Mk47Mutant_C")) { if(g_Menu.loot_weapon_mutant) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_M16A4_C"))  { if(g_Menu.loot_weapon_m16)   { should_draw = true; } }

                        // --- 2. WEAPONS: SNIPERS & DMRS ---
                        else if (id == skCrypt("Item_Weapon_AWM_C"))    { if(g_Menu.loot_weapon_awm)   { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_M24_C"))    { if(g_Menu.loot_weapon_m24)   { should_draw = true; col = IM_COL32(255, 150, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_Kar98k_C")) { if(g_Menu.loot_weapon_kar98) { should_draw = true; col = IM_COL32(255, 150, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_Mosin_C"))  { if(g_Menu.loot_weapon_mosin) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Win1894_C")) { if(g_Menu.loot_weapon_win94) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Mk14_C"))   { if(g_Menu.loot_weapon_mk14)  { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_FNFal_C"))  { if(g_Menu.loot_weapon_slr)   { should_draw = true; col = IM_COL32(0, 255, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_SKS_C"))    { if(g_Menu.loot_weapon_sks)   { should_draw = true; col = IM_COL32(0, 255, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_Mk12_C"))   { if(g_Menu.loot_weapon_mk12)  { should_draw = true; col = IM_COL32(0, 255, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_Dragunov_C")) { if(g_Menu.loot_weapon_dragunov) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_Mini14_C")) { if(g_Menu.loot_weapon_mini14) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_QBU88_C"))  { if(g_Menu.loot_weapon_qbu)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_VSS_C"))    { if(g_Menu.loot_weapon_vss)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_L6_C"))     { if(g_Menu.loot_weapon_all)   { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }

                        // --- 3. WEAPONS: SMG & LMG ---
                        else if (id == skCrypt("Item_Weapon_P90_C"))    { if(g_Menu.loot_weapon_p90)   { should_draw = true; col = IM_COL32(255, 255, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_UMP_C"))    { if(g_Menu.loot_weapon_ump)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Vector_C")) { if(g_Menu.loot_weapon_vector){ should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_UZI_C"))    { if(g_Menu.loot_weapon_uzi)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_MP5K_C"))   { if(g_Menu.loot_weapon_mp5)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_MP9_C"))    { if(g_Menu.loot_weapon_mp9)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_JS9_C"))    { if(g_Menu.loot_weapon_js9)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_BizonPP19_C")) { if(g_Menu.loot_weapon_bizon) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Thompson_C")) { if(g_Menu.loot_weapon_thompson) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_MG3_C"))    { if(g_Menu.loot_weapon_mg3)   { should_draw = true; col = IM_COL32(255, 100, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_M249_C"))   { if(g_Menu.loot_weapon_m249)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_DP28_C"))   { if(g_Menu.loot_weapon_dp28)  { should_draw = true; } }

                        // --- 4. WEAPONS: SHOTGUNS & HANDGUNS ---
                        else if (id == skCrypt("Item_Weapon_DP12_C"))   { if(g_Menu.loot_weapon_dp12)  { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_Saiga12_C")) { if(g_Menu.loot_weapon_s12k)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_OriginS12_C")) { if(g_Menu.loot_weapon_saiga) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Berreta686_C")) { if(g_Menu.loot_weapon_db)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_DesertEagle_C")) { if(g_Menu.loot_weapon_deagle) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_vz61Skorpion_C")) { if(g_Menu.loot_weapon_skorpion) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_M1911_C"))  { if(g_Menu.loot_weapon_m1911) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_M9_C"))     { if(g_Menu.loot_weapon_p92)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Rhino_C"))  { if(g_Menu.loot_weapon_rhino) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_NagantM1895_C")) { if(g_Menu.loot_weapon_nagant) { should_draw = true; } }

                        // --- 5. AMMO: INDIVIDUAL FILTERS ---
                        else if (id == skCrypt("Item_Ammo_556mm_C"))    { if(g_Menu.loot_ammo_556) { should_draw = true; col = IM_COL32(100, 255, 100, 220); } }
                        else if (id == skCrypt("Item_Ammo_762mm_C"))    { if(g_Menu.loot_ammo_762) { should_draw = true; col = IM_COL32(255, 120, 100, 220); } }
                        else if (id == skCrypt("Item_Ammo_300Magnum_C")) { if(g_Menu.loot_ammo_300) { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }
                        else if (id == skCrypt("Item_Ammo_9mm_C"))      { if(g_Menu.loot_ammo_9mm) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_45ACP_C"))    { if(g_Menu.loot_ammo_45)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_12Guage_C"))  { if(g_Menu.loot_ammo_12g) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_12GuageSlug_C")) { if(g_Menu.loot_ammo_slug) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_57mm_C"))     { if(g_Menu.loot_ammo_57)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_40mm_C"))     { if(g_Menu.loot_ammo_40)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_Bolt_C"))     { if(g_Menu.loot_ammo_bolt) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_Flare_C"))    { if(g_Menu.loot_ammo_flare) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_Mortar_C"))   { if(g_Menu.loot_ammo_mortar) { should_draw = true; } }

                        // --- 6. ATTACHMENTS: SCOPES ---
                        else if (id.find(skCrypt("DotSight")) != std::string::npos) { if(g_Menu.loot_scope_reddot) { should_draw = true; } }
                        else if (id.find(skCrypt("Holosight")) != std::string::npos) { if(g_Menu.loot_scope_holo) { should_draw = true; } }
                        else if (id.find(skCrypt("Aimpoint")) != std::string::npos) { if(g_Menu.loot_scope_2x) { should_draw = true; } }
                        else if (id.find(skCrypt("Scope3x")) != std::string::npos) { if(g_Menu.loot_scope_3x) { should_draw = true; } }
                        else if (id.find(skCrypt("ACOG")) != std::string::npos) { if(g_Menu.loot_scope_4x) { should_draw = true; } }
                        else if (id.find(skCrypt("Scope6x")) != std::string::npos) { if(g_Menu.loot_scope_6x) { should_draw = true; } }
                        else if (id.find(skCrypt("CQBSS")) != std::string::npos) { if(g_Menu.loot_scope_8x) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id.find(skCrypt("PM2")) != std::string::npos) { if(g_Menu.loot_scope_15x) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id.find(skCrypt("Thermal")) != std::string::npos) { if(g_Menu.loot_scope_thermal) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }

                        // --- 7. ATTACHMENTS: MUZZLES ---
                        else if (id.find(skCrypt("Compensator")) != std::string::npos) { if(g_Menu.loot_muzzle_comp) { should_draw = true; } }
                        else if (id.find(skCrypt("FlashHider")) != std::string::npos) { if(g_Menu.loot_muzzle_flash) { should_draw = true; } }
                        else if (id.find(skCrypt("Suppressor")) != std::string::npos) { if(g_Menu.loot_muzzle_supp) { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }
                        else if (id.find(skCrypt("Choke")) != std::string::npos) { if(g_Menu.loot_muzzle_choke) { should_draw = true; } }

                        // --- 8. ATTACHMENTS: GRIPS & STOCKS ---
                        else if (id.find(skCrypt("Foregrip")) != std::string::npos) { if(g_Menu.loot_grip_vertical) { should_draw = true; } }
                        else if (id.find(skCrypt("AngledForeGrip")) != std::string::npos) { if(g_Menu.loot_grip_angled) { should_draw = true; } }
                        else if (id.find(skCrypt("HalfGrip")) != std::string::npos) { if(g_Menu.loot_grip_half) { should_draw = true; } }
                        else if (id.find(skCrypt("ThumbGrip")) != std::string::npos) { if(g_Menu.loot_grip_thumb) { should_draw = true; } }
                        else if (id.find(skCrypt("Lightweight")) != std::string::npos) { if(g_Menu.loot_grip_light) { should_draw = true; } }
                        else if (id.find(skCrypt("Stock_AR_Heavy")) != std::string::npos) { if(g_Menu.loot_stock_heavy) { should_draw = true; } }
                        else if (id.find(skCrypt("CheekPad")) != std::string::npos) { if(g_Menu.loot_stock_cheek) { should_draw = true; } }

                        // --- 9. ATTACHMENTS: MAGAZINES ---
                        else if (id.find(skCrypt("ExtendedQuickDraw")) != std::string::npos) { if(g_Menu.loot_mag_ext_quick) { should_draw = true; } }
                        else if (id.find(skCrypt("Extended_")) != std::string::npos) { if(g_Menu.loot_mag_ext) { should_draw = true; } }
                        else if (id.find(skCrypt("QuickDraw_")) != std::string::npos) { if(g_Menu.loot_mag_quick) { should_draw = true; } }

                        // --- 10. THROWABLES & TACTICAL ---
                        else if (id == skCrypt("Item_Weapon_Grenade_C")) { if(g_Menu.loot_throw_frag) { should_draw = true; col = IM_COL32(255, 50, 50, 255); } }
                        else if (id == skCrypt("Item_Weapon_SmokeBomb_C")) { if(g_Menu.loot_throw_smoke) { should_draw = true; col = IM_COL32(200, 200, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_FlashBang_C")) { if(g_Menu.loot_throw_flash) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Molotov_C")) { if(g_Menu.loot_throw_molotov) { should_draw = true; col = IM_COL32(255, 150, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_C4_C"))      { if(g_Menu.loot_throw_c4)    { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_StickyGrenade_C")) { if(g_Menu.loot_throw_sticky) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_BluezoneGrenade_C")) { if(g_Menu.loot_throw_bz) { should_draw = true; col = IM_COL32(0, 100, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_DecoyGrenade_C")) { if(g_Menu.loot_throw_decoy) { should_draw = true; } }

                        // --- 11. GEAR: ARMOR & HELMETS & PACKS ---
                        else if (id.find(skCrypt("Lv3")) != std::string::npos) { if(g_Menu.loot_armor_lv3 || g_Menu.loot_helmet_lv3 || g_Menu.loot_backpack_lv3) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id.find(skCrypt("Lv2")) != std::string::npos) { if(g_Menu.loot_armor_lv2 || g_Menu.loot_helmet_lv2 || g_Menu.loot_backpack_lv2) { should_draw = true; col = IM_COL32(0, 200, 255, 255); } }
                        else if (id == skCrypt("Item_Back_BlueBlocker")) { if(g_Menu.loot_utility_jammer) { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }

                        // --- 12. RECOVERY: MEDS & BOOSTS ---
                        else if (id == skCrypt("Item_Heal_MedKit_C") || id == skCrypt("Item_Heal_FirstAid_C")) { if(g_Menu.loot_meds_healing) { should_draw = true; col = IM_COL32(100, 255, 100, 255); } }
                        else if (id.find(skCrypt("Item_Boost")) != std::string::npos) { if(g_Menu.loot_meds_boosts) { should_draw = true; col = IM_COL32(255, 255, 0, 255); } }

                        // --- 13. SPECIAL: GHILLIE & KEYS & REPAIR ---
                        else if (id == skCrypt("Item_Ghillie_01_C")) { if(g_Menu.loot_ghillie_arctic) { should_draw = true; col = IM_COL32(255, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Ghillie_02_C")) { if(g_Menu.loot_ghillie_desert) { should_draw = true; col = IM_COL32(255, 200, 100, 255); } }
                        else if (id == skCrypt("Item_Ghillie_03_C")) { if(g_Menu.loot_ghillie_jungle) { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }
                        else if (id == skCrypt("Item_Ghillie_04_C")) { if(g_Menu.loot_ghillie_forest) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ghillie_05_C")) { if(g_Menu.loot_ghillie_mossy)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ghillie_06_C")) { if(g_Menu.loot_ghillie_brown)  { should_draw = true; } }
                        else if (id.find(skCrypt("Key")) != std::string::npos || id.find(skCrypt("Keycard")) != std::string::npos) { if(g_Menu.loot_key_security || g_Menu.loot_key_secret) { should_draw = true; col = IM_COL32(255, 215, 0, 255); } }
                        else if (id.find(skCrypt("Repair_Kit")) != std::string::npos) { if(g_Menu.loot_repair_armor || g_Menu.loot_repair_helmet) { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }

                        // --- 14. CATCH-ALL & IMPORTANT ---
                        else if (g_Menu.loot_weapon_all || item.IsImportant) should_draw = true;
                    }
                }

                Vector2 itemScreen;
                if (telemetryContext::WorldToScreen(item.Position, itemScreen)) {
                    std::string resolvedIconName = EntityAliases::ResolveItemAsset(item.Name);
                    bool duplicateVehicle = false;
                    if (item.RenderType == ItemRenderType::Vehicle) {
                        const std::string vehicleResolverName =
                            item.ClassName.empty() ? item.Name : item.ClassName;
                        resolvedIconName = VehicleResolver::Resolve(vehicleResolverName).IconName;
                        duplicateVehicle = HasDuplicateVehicle(resolvedIconName, itemScreen);
                    }

                    if (g_Menu.debug_loot_resolver) {
                        LootDebugRenderer::Draw(draw, item, itemScreen, resolvedIconName,
                            duplicateVehicle, should_draw && !duplicateVehicle);
                    }

                    if (should_draw) {
                        if (item.RenderType == ItemRenderType::Projectile) {
                            DrawProjectileThreat(draw, item, itemScreen, col);
                            continue;
                        }

                        TextureInfo* icon = nullptr;
                        float iconSize = g_Menu.item_icon_size;
                        if (g_Menu.esp_icons) {
                           if (item.RenderType == ItemRenderType::Vehicle) {
                               if (duplicateVehicle) continue;
                               MarkVehicleSeen(resolvedIconName, itemScreen);
                               icon = OverlayTextures::GetVehicleIcon(resolvedIconName);
                               iconSize = g_Menu.vehicle_icon_size;
                           } else {
                               icon = OverlayTextures::GetItemIcon(item.Name);
                           }
                        } else if (item.RenderType == ItemRenderType::Vehicle) {
                            if (duplicateVehicle) continue;
                            MarkVehicleSeen(resolvedIconName, itemScreen);
                            iconSize = g_Menu.vehicle_icon_size;
                        }

                        const bool groupable =
                            item.RenderType == ItemRenderType::Loot && icon && icon->SRV;
                        lootEntries.push_back({ item.Name, itemScreen, item.Distance, col, icon, groupable, item.IsImportant, iconSize });
                    }
                }
            }

            LootClusterRenderer::Draw(draw, lootEntries, {
                g_Menu.item_icon_size,
                g_Menu.vehicle_icon_size,
                g_Menu.item_group_icon_size,
                g_Menu.loot_distance_font_size
            });
}
