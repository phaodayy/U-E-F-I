#pragma once

#include <PxPhysicsAPI.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace audit {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct MeshData {
    std::vector<Vec3> vertices;
    std::vector<std::uint32_t> indices;
    std::string sourceName;
};

struct HeightFieldData {
    std::uint32_t rows = 0;
    std::uint32_t columns = 0;
    float rowScale = 1.0f;
    float columnScale = 1.0f;
    float heightScale = 1.0f;
    std::vector<std::int16_t> samples;
    std::string sourceName;
};

enum class CollisionGroup : std::uint32_t {
    Static = 1u << 0,
    Dynamic = 1u << 1,
    Transparent = 1u << 2,
};

struct RaycastStats {
    std::uint64_t rays = 0;
    std::uint64_t hits = 0;
    std::uint64_t blockedByFilter = 0;
    double elapsedMs = 0.0;
};

struct CookStats {
    std::uint64_t meshCookRequests = 0;
    std::uint64_t meshCookCacheHits = 0;
    std::uint64_t meshCookCacheMisses = 0;
    std::uint64_t heightFieldCookRequests = 0;
    double elapsedMs = 0.0;
};

class ObjLoader {
public:
    static MeshData Load(const std::string& path);
};

class RawHeightFieldLoader {
public:
    static HeightFieldData LoadI16(const std::string& path,
                                   std::uint32_t rows,
                                   std::uint32_t columns,
                                   float rowScale,
                                   float columnScale,
                                   float heightScale);
};

class PhysXAuditHarness {
public:
    PhysXAuditHarness();
    ~PhysXAuditHarness();

    PhysXAuditHarness(const PhysXAuditHarness&) = delete;
    PhysXAuditHarness& operator=(const PhysXAuditHarness&) = delete;

    bool initialize();
    void cleanup();

    bool addTriangleMesh(const MeshData& mesh, CollisionGroup group, bool useCache);
    bool addHeightField(const HeightFieldData& heightField, CollisionGroup group);

    RaycastStats runRaycastBatch(std::uint32_t rayCount, std::uint32_t rayMask, std::uint32_t seed);
    const CookStats& cookStats() const { return cookStats_; }
    std::size_t cachedMeshCount() const { return meshCache_.size(); }
    std::size_t actorCount() const { return actors_.size(); }

    static std::uint32_t groupMask(CollisionGroup group);

private:
    struct CachedMesh {
        physx::PxTriangleMesh* mesh = nullptr;
        std::uint64_t refs = 0;
    };

    static physx::PxFilterData filterDataFor(CollisionGroup group);
    static std::uint64_t meshHash(const MeshData& mesh);
    static bool validateMesh(const MeshData& mesh);
    static bool validateHeightField(const HeightFieldData& heightField);

    physx::PxTriangleMesh* cookTriangleMesh(const MeshData& mesh, bool useCache);

    physx::PxDefaultAllocator allocator_;
    physx::PxDefaultErrorCallback errorCallback_;
    physx::PxFoundation* foundation_ = nullptr;
    physx::PxPhysics* physics_ = nullptr;
    physx::PxCooking* cooking_ = nullptr;
    physx::PxScene* scene_ = nullptr;
    physx::PxMaterial* material_ = nullptr;
    physx::PxDefaultCpuDispatcher* dispatcher_ = nullptr;

    std::unordered_map<std::uint64_t, CachedMesh> meshCache_;
    std::vector<physx::PxRigidActor*> actors_;
    CookStats cookStats_;
};

std::uint32_t parseGroupMask(const std::string& text);
CollisionGroup parseGroup(const std::string& text);

} // namespace audit

