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

bool ApplyLootDraw(bool enabled, ImU32 drawColor, ImU32& col, bool& shouldDraw) {
    if (enabled) {
        shouldDraw = true;
        col = drawColor;
    }
    return true;
}

bool TryResolveGearOrUtilityLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    const ImU32 utilityColor = IM_COL32(0, 190, 255, 255);
    const ImU32 lv2Color = IM_COL32(0, 200, 255, 255);
    const ImU32 lv3Color = IM_COL32(255, 0, 255, 255);

    if (id == skCrypt("Item_Back_BlueBlocker")) {
        return ApplyLootDraw(menu.loot_utility_jammer, IM_COL32(0, 255, 255, 255), col, shouldDraw);
    }
    if (id == skCrypt("Item_Weapon_Drone_C")) {
        return ApplyLootDraw(menu.loot_utility_drone, IM_COL32(0, 255, 255, 255), col, shouldDraw);
    }
    if (id == skCrypt("Item_Weapon_Spotter_Scope_C")) {
        return ApplyLootDraw(menu.loot_utility_scope, IM_COL32(0, 255, 255, 255), col, shouldDraw);
    }
    if (id == skCrypt("Item_Bluechip_C")) {
        return ApplyLootDraw(menu.loot_utility_bluechip, utilityColor, col, shouldDraw);
    }
    if (id == skCrypt("Item_Revival_Transmitter_C")) {
        return ApplyLootDraw(menu.loot_utility_vtransmitter, utilityColor, col, shouldDraw);
    }
    if (id == skCrypt("Item_BulletproofShield_C")) {
        return ApplyLootDraw(menu.loot_utility_shield, utilityColor, col, shouldDraw);
    }
    if (id == skCrypt("Item_EmergencyPickup_C")) {
        return ApplyLootDraw(menu.loot_utility_emergency, utilityColor, col, shouldDraw);
    }
    if (id == skCrypt("Item_JerryCan_C")) {
        return ApplyLootDraw(menu.loot_utility_jerrycan, IM_COL32(255, 170, 50, 255), col, shouldDraw);
    }
    if (id == skCrypt("Item_Tiger_SelfRevive_C")) {
        return ApplyLootDraw(menu.loot_utility_selfrevive, utilityColor, col, shouldDraw);
    }
    if (id == skCrypt("InstantRevivalKit_C")) {
        return ApplyLootDraw(menu.loot_utility_instantrevive, utilityColor, col, shouldDraw);
    }

    const bool lv1 = id.find(skCrypt("Lv1")) != std::string::npos;
    const bool lv2 = id.find(skCrypt("Lv2")) != std::string::npos;
    const bool lv3 = id.find(skCrypt("Lv3")) != std::string::npos;
    const ImU32 gearColor = lv3 ? lv3Color : lv2Color;

    if (id.find(skCrypt("Item_Head")) != std::string::npos || id.find(skCrypt("Helmet")) != std::string::npos) {
        return ApplyLootDraw((lv1 && menu.loot_helmet_lv1) || (lv2 && menu.loot_helmet_lv2) || (lv3 && menu.loot_helmet_lv3),
            gearColor, col, shouldDraw);
    }
    if (id.find(skCrypt("Item_Armor")) != std::string::npos || id.find(skCrypt("Vest")) != std::string::npos) {
        return ApplyLootDraw((lv1 && menu.loot_armor_lv1) || (lv2 && menu.loot_armor_lv2) || (lv3 && menu.loot_armor_lv3),
            gearColor, col, shouldDraw);
    }
    if (id.find(skCrypt("Item_Back")) != std::string::npos) {
        return ApplyLootDraw((lv1 && menu.loot_backpack_lv1) || (lv2 && menu.loot_backpack_lv2) || (lv3 && menu.loot_backpack_lv3),
            gearColor, col, shouldDraw);
    }

    return false;
}
bool TryResolveKeyOrRepairLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    const ImU32 keyColor = IM_COL32(255, 215, 0, 255);
    const ImU32 repairColor = IM_COL32(0, 255, 0, 255);

    if (id.find(skCrypt("Tiger_Key")) != std::string::npos) return ApplyLootDraw(menu.loot_key_taego, keyColor, col, shouldDraw);
    if (id.find(skCrypt("Neon_Coin")) != std::string::npos) return ApplyLootDraw(menu.loot_key_neon_coin, keyColor, col, shouldDraw);
    if (id.find(skCrypt("Neon_Gold")) != std::string::npos) return ApplyLootDraw(menu.loot_key_neon_gold, keyColor, col, shouldDraw);
    if (id.find(skCrypt("DihorOtok_Key")) != std::string::npos) return ApplyLootDraw(menu.loot_key_vikendi, keyColor, col, shouldDraw);
    if (id.find(skCrypt("Chimera_Key")) != std::string::npos) return ApplyLootDraw(menu.loot_key_chimera, keyColor, col, shouldDraw);
    if (id.find(skCrypt("Heaven_Key")) != std::string::npos) return ApplyLootDraw(menu.loot_key_haven, keyColor, col, shouldDraw);
    if (id.find(skCrypt("Secuity_Keycard")) != std::string::npos || id.find(skCrypt("Security_Keycard")) != std::string::npos) {
        return ApplyLootDraw(menu.loot_key_security, keyColor, col, shouldDraw);
    }
    if (id.find(skCrypt("SecretRoom")) != std::string::npos || id.find(skCrypt("BTSecretRoom")) != std::string::npos) {
        return ApplyLootDraw(menu.loot_key_secret, keyColor, col, shouldDraw);
    }
    if (id.find(skCrypt("Key")) != std::string::npos || id.find(skCrypt("Keycard")) != std::string::npos) {
        return ApplyLootDraw(menu.loot_key_security || menu.loot_key_secret || menu.loot_key_taego ||
            menu.loot_key_vikendi || menu.loot_key_chimera || menu.loot_key_haven, keyColor, col, shouldDraw);
    }

    if (id.find(skCrypt("Armor_Repair_Kit")) != std::string::npos) return ApplyLootDraw(menu.loot_repair_armor, repairColor, col, shouldDraw);
    if (id.find(skCrypt("Helmet_Repair_Kit")) != std::string::npos) return ApplyLootDraw(menu.loot_repair_helmet, repairColor, col, shouldDraw);
    if (id.find(skCrypt("Vehicle_Repair_Kit")) != std::string::npos) return ApplyLootDraw(menu.loot_repair_vehicle, repairColor, col, shouldDraw);
    if (id.find(skCrypt("Repair_Kit")) != std::string::npos) {
        return ApplyLootDraw(menu.loot_repair_armor || menu.loot_repair_helmet || menu.loot_repair_vehicle,
            repairColor, col, shouldDraw);
    }

    return false;
}

bool TryResolveWeaponLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    // AR
    if      (id == skCrypt("Item_Weapon_HK416_C") || id == skCrypt("Item_Weapon_Duncans_M416_C")) { if(menu.loot_weapon_hk416) { shouldDraw = true; col = IM_COL32(0, 255, 255, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_AK47_C") || id == skCrypt("Item_Weapon_Lunchmeats_AK47_C"))  { if(menu.loot_weapon_ak47)  { shouldDraw = true; col = IM_COL32(0, 255, 255, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_BerylM762_C")) { if(menu.loot_weapon_beryl) { shouldDraw = true; col = IM_COL32(255, 120, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_SCAR-L_C")) { if(menu.loot_weapon_scar)  { shouldDraw = true; col = IM_COL32(0, 255, 255, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_AUG_C"))    { if(menu.loot_weapon_aug)   { shouldDraw = true; col = IM_COL32(255, 255, 255, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Groza_C"))  { if(menu.loot_weapon_groza) { shouldDraw = true; col = IM_COL32(255, 100, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_ACE32_C"))  { if(menu.loot_weapon_ace32) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_FAMASG2_C")) { if(menu.loot_weapon_famas) { shouldDraw = true; col = IM_COL32(255, 100, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_G36C_C"))   { if(menu.loot_weapon_g36c)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_QBZ95_C"))  { if(menu.loot_weapon_qbz)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_K2_C"))     { if(menu.loot_weapon_k2)    { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Mk47Mutant_C")) { if(menu.loot_weapon_mutant) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_M16A4_C"))  { if(menu.loot_weapon_m16)   { shouldDraw = true; } return true; }
    
    // Snipers/DMRs
    else if (id == skCrypt("Item_Weapon_AWM_C"))    { if(menu.loot_weapon_awm)   { shouldDraw = true; col = IM_COL32(255, 0, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_M24_C"))    { if(menu.loot_weapon_m24)   { shouldDraw = true; col = IM_COL32(255, 150, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Kar98k_C")) { if(menu.loot_weapon_kar98) { shouldDraw = true; col = IM_COL32(255, 150, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Mosin_C"))  { if(menu.loot_weapon_mosin) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Win1894_C")) { if(menu.loot_weapon_win94) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Mk14_C"))   { if(menu.loot_weapon_mk14)  { shouldDraw = true; col = IM_COL32(255, 0, 255, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_FNFal_C"))  { if(menu.loot_weapon_slr)   { shouldDraw = true; col = IM_COL32(0, 255, 200, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_SKS_C"))    { if(menu.loot_weapon_sks)   { shouldDraw = true; col = IM_COL32(0, 255, 200, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Mk12_C"))   { if(menu.loot_weapon_mk12)  { shouldDraw = true; col = IM_COL32(0, 255, 200, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Dragunov_C")) { if(menu.loot_weapon_dragunov) { shouldDraw = true; col = IM_COL32(255, 0, 255, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Mini14_C")) { if(menu.loot_weapon_mini14) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_QBU88_C") || id == skCrypt("Item_Weapon_Mads_QBU88_C"))  { if(menu.loot_weapon_qbu)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_VSS_C"))    { if(menu.loot_weapon_vss)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_L6_C"))     { if(menu.loot_weapon_lynx)  { shouldDraw = true; col = IM_COL32(255, 0, 0, 255); } return true; }

    // SMG/LMG
    else if (id == skCrypt("Item_Weapon_P90_C"))    { if(menu.loot_weapon_p90)   { shouldDraw = true; col = IM_COL32(255, 255, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_UMP_C"))    { if(menu.loot_weapon_ump)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Vector_C")) { if(menu.loot_weapon_vector){ shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_UZI_C"))    { if(menu.loot_weapon_uzi)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_MP5K_C"))   { if(menu.loot_weapon_mp5)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_MP9_C"))    { if(menu.loot_weapon_mp9)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_JS9_C") || id == skCrypt("WeapJS9_C"))    { if(menu.loot_weapon_js9)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_BizonPP19_C")) { if(menu.loot_weapon_bizon) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Thompson_C")) { if(menu.loot_weapon_thompson) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_MG3_C"))    { if(menu.loot_weapon_mg3)   { shouldDraw = true; col = IM_COL32(255, 100, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_M249_C"))   { if(menu.loot_weapon_m249)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_DP28_C"))   { if(menu.loot_weapon_dp28)  { shouldDraw = true; } return true; }

    // Shotguns/Misc
    else if (id == skCrypt("Item_Weapon_DP12_C"))   { if(menu.loot_weapon_dp12)  { shouldDraw = true; col = IM_COL32(0, 255, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Saiga12_C")) { if(menu.loot_weapon_s12k)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_OriginS12_C")) { if(menu.loot_weapon_saiga) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Berreta686_C")) { if(menu.loot_weapon_db)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Winchester_C")) { if(menu.loot_weapon_s1897) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Sawnoff_C")) { if(menu.loot_weapon_sawedoff) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_DesertEagle_C")) { if(menu.loot_weapon_deagle) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_vz61Skorpion_C")) { if(menu.loot_weapon_skorpion) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_M1911_C"))  { if(menu.loot_weapon_m1911) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_M9_C"))     { if(menu.loot_weapon_p92)   { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_G18_C"))    { if(menu.loot_weapon_p18c)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Rhino_C"))  { if(menu.loot_weapon_rhino) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_NagantM1895_C")) { if(menu.loot_weapon_nagant) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_StunGun_C")) { if(menu.loot_weapon_stungun) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_FlareGun_C")) { if(menu.loot_weapon_flare) { shouldDraw = true; col = IM_COL32(255, 80, 40, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Crossbow_C")) { if(menu.loot_weapon_crossbow) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_PanzerFaust100M_C") || id.find(skCrypt("PanzerFaust")) != std::string::npos) { if(menu.loot_weapon_panzer) { shouldDraw = true; col = IM_COL32(255, 120, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_Mortar_C")) { if(menu.loot_weapon_mortar) { shouldDraw = true; col = IM_COL32(255, 130, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_M79_C")) { if(menu.loot_weapon_m79) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Ziplinegun_C")) { if(menu.loot_weapon_zipline) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_TacPack_C")) { if(menu.loot_weapon_tacpack) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_TraumaBag_C")) { if(menu.loot_weapon_trauma) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_IntegratedRepair_C")) { if(menu.loot_weapon_integrated_repair) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Pan_C")) { if(menu.loot_weapon_pan) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Cowbar_C")) { if(menu.loot_weapon_crowbar) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Machete_C")) { if(menu.loot_weapon_machete) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Sickle_C")) { if(menu.loot_weapon_sickle) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Pickaxe_C")) { if(menu.loot_weapon_pickaxe) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_SpikeTrap_C")) { if(menu.loot_weapon_spike) { shouldDraw = true; } return true; }
    
    return false;
}

bool TryResolveAmmoLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    if      (id == skCrypt("Item_Ammo_556mm_C"))    { if(menu.loot_ammo_556) { shouldDraw = true; col = IM_COL32(100, 255, 100, 220); } return true; }
    else if (id == skCrypt("Item_Ammo_762mm_C"))    { if(menu.loot_ammo_762) { shouldDraw = true; col = IM_COL32(255, 120, 100, 220); } return true; }
    else if (id == skCrypt("Item_Ammo_300Magnum_C")) { if(menu.loot_ammo_300) { shouldDraw = true; col = IM_COL32(255, 0, 0, 255); } return true; }
    else if (id == skCrypt("Item_Ammo_9mm_C"))      { if(menu.loot_ammo_9mm) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_45ACP_C"))    { if(menu.loot_ammo_45)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_12Guage_C"))  { if(menu.loot_ammo_12g) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_12GuageSlug_C")) { if(menu.loot_ammo_slug) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_57mm_C"))     { if(menu.loot_ammo_57)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_40mm_C"))     { if(menu.loot_ammo_40)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_Bolt_C"))     { if(menu.loot_ammo_bolt) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_Flare_C"))    { if(menu.loot_ammo_flare) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_Mortar_C"))   { if(menu.loot_ammo_mortar) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ammo_ZiplinegunHook_C")) { if(menu.loot_ammo_zipline) { shouldDraw = true; } return true; }
    return false;
}

bool TryResolveAttachmentLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    // Scopes
    if      (id.find(skCrypt("DotSight")) != std::string::npos) { if(menu.loot_scope_reddot) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Holosight")) != std::string::npos) { if(menu.loot_scope_holo) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Aimpoint")) != std::string::npos) { if(menu.loot_scope_2x) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Scope3x")) != std::string::npos) { if(menu.loot_scope_3x) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("ACOG")) != std::string::npos) { if(menu.loot_scope_4x) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Scope6x")) != std::string::npos) { if(menu.loot_scope_6x) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("CQBSS")) != std::string::npos) { if(menu.loot_scope_8x) { shouldDraw = true; col = IM_COL32(255, 0, 255, 255); } return true; }
    else if (id.find(skCrypt("PM2")) != std::string::npos) { if(menu.loot_scope_15x) { shouldDraw = true; col = IM_COL32(255, 0, 255, 255); } return true; }
    else if (id.find(skCrypt("Thermal")) != std::string::npos) { if(menu.loot_scope_thermal) { shouldDraw = true; col = IM_COL32(255, 0, 255, 255); } return true; }

    // Muzzles
    else if (id.find(skCrypt("Compensator")) != std::string::npos) { if(menu.loot_muzzle_comp) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("FlashHider")) != std::string::npos) { if(menu.loot_muzzle_flash) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Suppressor")) != std::string::npos) { if(menu.loot_muzzle_supp) { shouldDraw = true; col = IM_COL32(0, 255, 0, 255); } return true; }
    else if (id.find(skCrypt("Choke")) != std::string::npos) { if(menu.loot_muzzle_choke) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("MuzzleBrake")) != std::string::npos) { if(menu.loot_muzzle_brake) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Duckbill")) != std::string::npos) { if(menu.loot_muzzle_duckbill) { shouldDraw = true; } return true; }

    // Grips/Stocks
    else if (id.find(skCrypt("AngledForeGrip")) != std::string::npos) { if(menu.loot_grip_angled) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("HalfGrip")) != std::string::npos) { if(menu.loot_grip_half) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("ThumbGrip")) != std::string::npos) { if(menu.loot_grip_thumb) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Lightweight")) != std::string::npos) { if(menu.loot_grip_light) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("LaserPointer")) != std::string::npos) { if(menu.loot_grip_laser) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Crossbow")) != std::string::npos) { if(menu.loot_grip_crossbow_quiver) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Foregrip")) != std::string::npos) { if(menu.loot_grip_vertical) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Stock_AR_Heavy")) != std::string::npos) { if(menu.loot_stock_heavy) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Stock_AR_Composite")) != std::string::npos) { if(menu.loot_stock_tactical) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("CheekPad")) != std::string::npos) { if(menu.loot_stock_cheek) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("BulletLoops")) != std::string::npos) { if(menu.loot_stock_bullet_loops) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Stock_UZI")) != std::string::npos) { if(menu.loot_stock_uzi) { shouldDraw = true; } return true; }

    // Magazines
    else if (id.find(skCrypt("ExtendedQuickDraw")) != std::string::npos) { if(menu.loot_mag_ext_quick) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("Extended_")) != std::string::npos) { if(menu.loot_mag_ext) { shouldDraw = true; } return true; }
    else if (id.find(skCrypt("QuickDraw_")) != std::string::npos) { if(menu.loot_mag_quick) { shouldDraw = true; } return true; }
    
    return false;
}

bool TryResolveThrowableLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    if      (id == skCrypt("Item_Weapon_Grenade_C") || id == skCrypt("Item_Weapon_Grenade_Warmode_C")) { if(menu.loot_throw_frag) { shouldDraw = true; col = IM_COL32(255, 50, 50, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_SmokeBomb_C")) { if(menu.loot_throw_smoke) { shouldDraw = true; col = IM_COL32(200, 200, 200, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_FlashBang_C")) { if(menu.loot_throw_flash) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_Molotov_C")) { if(menu.loot_throw_molotov) { shouldDraw = true; col = IM_COL32(255, 150, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_C4_C"))      { if(menu.loot_throw_c4)    { shouldDraw = true; col = IM_COL32(255, 0, 0, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_StickyGrenade_C")) { if(menu.loot_throw_sticky) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Weapon_BluezoneGrenade_C") || id == skCrypt("Item_Weapon_BZGrenade_C")) { if(menu.loot_throw_bz) { shouldDraw = true; col = IM_COL32(0, 100, 255, 255); } return true; }
    else if (id == skCrypt("Item_Weapon_DecoyGrenade_C")) { if(menu.loot_throw_decoy) { shouldDraw = true; } return true; }
    return false;
}

bool TryResolveRecoveryLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    if (id == skCrypt("Item_Heal_MedKit_C") || id == skCrypt("Item_Heal_FirstAid_C")) { if(menu.loot_meds_healing) { shouldDraw = true; col = IM_COL32(100, 255, 100, 255); } return true; }
    else if (id == skCrypt("Item_Heal_Bandage_C")) { if(menu.loot_meds_bandage) { shouldDraw = true; col = IM_COL32(100, 255, 100, 255); } return true; }
    else if (id == skCrypt("Item_Heal_BattleReadyKit_C")) { if(menu.loot_meds_battle_ready) { shouldDraw = true; col = IM_COL32(100, 255, 100, 255); } return true; }
    else if (id.find(skCrypt("Item_Boost")) != std::string::npos) { if(menu.loot_meds_boosts) { shouldDraw = true; col = IM_COL32(255, 255, 0, 255); } return true; }
    return false;
}

bool TryResolveSpecialLoot(const std::string& id, const OverlayMenu& menu, ImU32& col, bool& shouldDraw) {
    if      (id == skCrypt("Item_Ghillie_01_C")) { if(menu.loot_ghillie_arctic) { shouldDraw = true; col = IM_COL32(255, 255, 255, 255); } return true; }
    else if (id == skCrypt("Item_Ghillie_02_C")) { if(menu.loot_ghillie_desert) { shouldDraw = true; col = IM_COL32(255, 200, 100, 255); } return true; }
    else if (id == skCrypt("Item_Ghillie_03_C")) { if(menu.loot_ghillie_jungle) { shouldDraw = true; col = IM_COL32(0, 255, 0, 255); } return true; }
    else if (id == skCrypt("Item_Ghillie_04_C")) { if(menu.loot_ghillie_forest) { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ghillie_05_C")) { if(menu.loot_ghillie_mossy)  { shouldDraw = true; } return true; }
    else if (id == skCrypt("Item_Ghillie_06_C")) { if(menu.loot_ghillie_brown)  { shouldDraw = true; } return true; }
    return false;
}

} // namespace

void OverlayMenu::RenderLootEsp(ImDrawList* draw) {
    if (!draw || !esp_toggle) return;
            DrawMapMeshDebugEsp(draw);
            DrawHeldThrowablePrediction(draw);

            std::vector<ItemData> allLoot = LootSourceMerge::BuildAllLoot();

            struct VehicleDrawKey {
                std::string IconName;
                Vector3 Position;
                Vector2 Screen;
                float Distance = 0.0f;
            };
            std::vector<VehicleDrawKey> seenVehicles;
            auto HasDuplicateVehicle = [&](const Vector3& position, const Vector2& screen, float distance) {
                constexpr float kDuplicateScreenRadiusSq = 120.0f * 120.0f;
                constexpr float kDuplicateWorldRadiusCm = 1600.0f;
                constexpr float kDuplicateDistanceMeters = 8.0f;
                for (const auto& seen : seenVehicles) {
                    const float dx = seen.Screen.x - screen.x;
                    const float dy = seen.Screen.y - screen.y;
                    if ((dx * dx) + (dy * dy) > kDuplicateScreenRadiusSq) continue;
                    if (std::fabs(seen.Distance - distance) <= kDuplicateDistanceMeters ||
                        seen.Position.Distance(position) <= kDuplicateWorldRadiusCm) {
                        return true;
                    }
                }
                return false;
            };
            auto MarkVehicleSeen = [&](const std::string& iconName, const Vector3& position, const Vector2& screen, float distance) {
                seenVehicles.push_back({ iconName, position, screen, distance });
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
                        const std::string& id = item.Name;

                        if (TryResolveWeaponLoot(id, g_Menu, col, should_draw)) {}
                        else if (TryResolveAmmoLoot(id, g_Menu, col, should_draw)) {}
                        else if (TryResolveAttachmentLoot(id, g_Menu, col, should_draw)) {}
                        else if (TryResolveThrowableLoot(id, g_Menu, col, should_draw)) {}
                        else if (TryResolveGearOrUtilityLoot(id, g_Menu, col, should_draw)) {}
                        else if (TryResolveRecoveryLoot(id, g_Menu, col, should_draw)) {}
                        else if (TryResolveSpecialLoot(id, g_Menu, col, should_draw)) {}
                        else if (TryResolveKeyOrRepairLoot(id, g_Menu, col, should_draw)) {}
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
                        duplicateVehicle = HasDuplicateVehicle(item.Position, itemScreen, item.Distance);
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
                               MarkVehicleSeen(resolvedIconName, item.Position, itemScreen, item.Distance);
                               icon = OverlayTextures::GetVehicleIcon(resolvedIconName);
                               iconSize = g_Menu.vehicle_icon_size;
                           } else {
                               icon = OverlayTextures::GetItemIcon(item.Name);
                           }
                        } else if (item.RenderType == ItemRenderType::Vehicle) {
                            if (duplicateVehicle) continue;
                            MarkVehicleSeen(resolvedIconName, item.Position, itemScreen, item.Distance);
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
