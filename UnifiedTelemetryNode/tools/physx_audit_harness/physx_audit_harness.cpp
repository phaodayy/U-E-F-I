#include "physx_audit_harness.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

using namespace physx;

namespace audit {
namespace {

constexpr std::uint64_t kFnvOffset = 14695981039346656037ull;
constexpr std::uint64_t kFnvPrime = 1099511628211ull;
constexpr float kSceneExtent = 5000.0f;
constexpr float kRayOriginZ = 1200.0f;
constexpr float kRayTargetZ = -800.0f;

bool finite(float value) {
    return std::isfinite(value);
}

void hashBytes(std::uint64_t& hash, const void* data, std::size_t size) {
    const auto* bytes = static_cast<const std::uint8_t*>(data);
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= kFnvPrime;
    }
}

std::vector<std::string> split(const std::string& value, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, delim)) {
        item.erase(std::remove_if(item.begin(), item.end(), [](unsigned char c) { return std::isspace(c) != 0; }), item.end());
        if (!item.empty()) out.push_back(item);
    }
    return out;
}

PxVec3 toPx(const Vec3& v) {
    return PxVec3(v.x, v.y, v.z);
}

class MaskQueryFilter final : public PxQueryFilterCallback {
public:
    explicit MaskQueryFilter(std::uint64_t* blocked) : blocked_(blocked) {}

    PxQueryHitType::Enum preFilter(const PxFilterData& filterData,
                                   const PxShape* shape,
                                   const PxRigidActor*,
                                   PxHitFlags&) override {
        const PxFilterData shapeData = shape ? shape->getQueryFilterData() : PxFilterData{};
        if ((shapeData.word0 & filterData.word0) == 0) {
            if (blocked_) ++(*blocked_);
            return PxQueryHitType::eNONE;
        }
        return PxQueryHitType::eBLOCK;
    }

    PxQueryHitType::Enum postFilter(const PxFilterData&,
                                    const PxQueryHit&) override {
        return PxQueryHitType::eBLOCK;
    }

private:
    std::uint64_t* blocked_ = nullptr;
};

} // namespace

MeshData ObjLoader::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open OBJ: " + path);
    }

    MeshData mesh;
    mesh.sourceName = path;

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("v ", 0) == 0) {
            std::istringstream ss(line.substr(2));
            Vec3 v{};
            ss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        } else if (line.rfind("f ", 0) == 0) {
            std::istringstream ss(line.substr(2));
            std::string token;
            std::vector<std::uint32_t> face;
            while (ss >> token) {
                const std::size_t slash = token.find('/');
                if (slash != std::string::npos) token.resize(slash);
                if (token.empty()) continue;
                int idx = std::stoi(token);
                if (idx < 0) idx = static_cast<int>(mesh.vertices.size()) + idx + 1;
                if (idx <= 0) continue;
                face.push_back(static_cast<std::uint32_t>(idx - 1));
            }
            if (face.size() >= 3) {
                for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                    mesh.indices.push_back(face[0]);
                    mesh.indices.push_back(face[i]);
                    mesh.indices.push_back(face[i + 1]);
                }
            }
        }
    }

    return mesh;
}

HeightFieldData RawHeightFieldLoader::LoadI16(const std::string& path,
                                              std::uint32_t rows,
                                              std::uint32_t columns,
                                              float rowScale,
                                              float columnScale,
                                              float heightScale) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("cannot open RAW heightfield: " + path);
    }

    HeightFieldData data;
    data.rows = rows;
    data.columns = columns;
    data.rowScale = rowScale;
    data.columnScale = columnScale;
    data.heightScale = heightScale;
    data.sourceName = path;
    data.samples.resize(static_cast<std::size_t>(rows) * columns);

    file.read(reinterpret_cast<char*>(data.samples.data()),
        static_cast<std::streamsize>(data.samples.size() * sizeof(std::int16_t)));
    if (file.gcount() != static_cast<std::streamsize>(data.samples.size() * sizeof(std::int16_t))) {
        throw std::runtime_error("RAW heightfield size does not match rows*columns*i16");
    }

    return data;
}

PhysXAuditHarness::PhysXAuditHarness() = default;

PhysXAuditHarness::~PhysXAuditHarness() {
    cleanup();
}

bool PhysXAuditHarness::initialize() {
    if (foundation_ && physics_ && cooking_ && scene_ && material_) return true;

    foundation_ = PxCreateFoundation(PX_FOUNDATION_VERSION, allocator_, errorCallback_);
    if (!foundation_) return false;

    PxTolerancesScale scale;
    physics_ = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation_, scale);
    if (!physics_) return false;

    cooking_ = PxCreateCooking(PX_PHYSICS_VERSION, *foundation_, PxCookingParams(scale));
    if (!cooking_) return false;

    dispatcher_ = PxDefaultCpuDispatcherCreate(2);
    if (!dispatcher_) return false;

    PxSceneDesc sceneDesc(scale);
    sceneDesc.gravity = PxVec3(0.0f, 0.0f, -9.81f);
    sceneDesc.cpuDispatcher = dispatcher_;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    scene_ = physics_->createScene(sceneDesc);
    material_ = physics_->createMaterial(0.5f, 0.5f, 0.1f);

    return scene_ && material_;
}

void PhysXAuditHarness::cleanup() {
    for (PxRigidActor* actor : actors_) {
        if (actor) {
            if (scene_) scene_->removeActor(*actor);
            actor->release();
        }
    }
    actors_.clear();

    for (auto& item : meshCache_) {
        if (item.second.mesh) item.second.mesh->release();
    }
    meshCache_.clear();

    if (material_) { material_->release(); material_ = nullptr; }
    if (scene_) { scene_->release(); scene_ = nullptr; }
    if (dispatcher_) { dispatcher_->release(); dispatcher_ = nullptr; }
    if (cooking_) { cooking_->release(); cooking_ = nullptr; }
    if (physics_) { physics_->release(); physics_ = nullptr; }
    if (foundation_) { foundation_->release(); foundation_ = nullptr; }
}

std::uint32_t PhysXAuditHarness::groupMask(CollisionGroup group) {
    return static_cast<std::uint32_t>(group);
}

PxFilterData PhysXAuditHarness::filterDataFor(CollisionGroup group) {
    PxFilterData data;
    data.word0 = groupMask(group);
    data.word1 = groupMask(CollisionGroup::Static) |
        groupMask(CollisionGroup::Dynamic) |
        groupMask(CollisionGroup::Transparent);
    data.word2 = 0;
    data.word3 = 0;
    return data;
}

std::uint64_t PhysXAuditHarness::meshHash(const MeshData& mesh) {
    std::uint64_t hash = kFnvOffset;
    for (const Vec3& v : mesh.vertices) hashBytes(hash, &v, sizeof(v));
    for (std::uint32_t idx : mesh.indices) hashBytes(hash, &idx, sizeof(idx));
    return hash;
}

bool PhysXAuditHarness::validateMesh(const MeshData& mesh) {
    if (mesh.vertices.empty() || mesh.indices.empty()) return false;
    if ((mesh.indices.size() % 3) != 0) return false;
    for (const Vec3& v : mesh.vertices) {
        if (!finite(v.x) || !finite(v.y) || !finite(v.z)) return false;
    }
    const auto vertexCount = static_cast<std::uint32_t>(mesh.vertices.size());
    for (std::uint32_t idx : mesh.indices) {
        if (idx >= vertexCount) return false;
    }
    return true;
}

bool PhysXAuditHarness::validateHeightField(const HeightFieldData& heightField) {
    if (heightField.rows < 2 || heightField.columns < 2) return false;
    if (heightField.samples.size() != static_cast<std::size_t>(heightField.rows) * heightField.columns) return false;
    return finite(heightField.rowScale) && finite(heightField.columnScale) && finite(heightField.heightScale) &&
        heightField.rowScale > 0.0f && heightField.columnScale > 0.0f && heightField.heightScale != 0.0f;
}

PxTriangleMesh* PhysXAuditHarness::cookTriangleMesh(const MeshData& mesh, bool useCache) {
    ++cookStats_.meshCookRequests;
    const std::uint64_t hash = meshHash(mesh);
    if (useCache) {
        auto it = meshCache_.find(hash);
        if (it != meshCache_.end() && it->second.mesh) {
            ++it->second.refs;
            ++cookStats_.meshCookCacheHits;
            return it->second.mesh;
        }
    }

    ++cookStats_.meshCookCacheMisses;
    PxTriangleMeshDesc desc;
    desc.points.count = static_cast<PxU32>(mesh.vertices.size());
    desc.points.stride = sizeof(Vec3);
    desc.points.data = mesh.vertices.data();
    desc.triangles.count = static_cast<PxU32>(mesh.indices.size() / 3);
    desc.triangles.stride = 3 * sizeof(std::uint32_t);
    desc.triangles.data = mesh.indices.data();

    PxDefaultMemoryOutputStream out;
    if (!cooking_->cookTriangleMesh(desc, out)) return nullptr;

    PxDefaultMemoryInputData in(out.getData(), out.getSize());
    PxTriangleMesh* cooked = physics_->createTriangleMesh(in);
    if (cooked && useCache) meshCache_[hash] = { cooked, 1 };
    return cooked;
}

bool PhysXAuditHarness::addTriangleMesh(const MeshData& mesh, CollisionGroup group, bool useCache) {
    if (!initialize() || !validateMesh(mesh)) return false;

    const auto start = std::chrono::high_resolution_clock::now();
    PxTriangleMesh* triMesh = cookTriangleMesh(mesh, useCache);
    const auto end = std::chrono::high_resolution_clock::now();
    cookStats_.elapsedMs += std::chrono::duration<double, std::milli>(end - start).count();
    if (!triMesh) return false;

    PxTriangleMeshGeometry geometry(triMesh);
    PxShape* shape = physics_->createShape(geometry, *material_, true);
    if (!shape) return false;
    shape->setSimulationFilterData(filterDataFor(group));
    shape->setQueryFilterData(filterDataFor(group));

    PxRigidStatic* actor = physics_->createRigidStatic(PxTransform(PxIdentity));
    if (!actor) {
        shape->release();
        return false;
    }

    actor->attachShape(*shape);
    shape->release();
    scene_->addActor(*actor);
    actors_.push_back(actor);
    return true;
}

bool PhysXAuditHarness::addHeightField(const HeightFieldData& heightField, CollisionGroup group) {
    if (!initialize() || !validateHeightField(heightField)) return false;

    ++cookStats_.heightFieldCookRequests;
    std::vector<PxHeightFieldSample> samples(heightField.samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
        samples[i].height = heightField.samples[i];
        samples[i].materialIndex0 = 0;
        samples[i].materialIndex1 = 0;
        samples[i].clearTessFlag();
    }

    PxHeightFieldDesc desc;
    desc.nbRows = heightField.rows;
    desc.nbColumns = heightField.columns;
    desc.samples.data = samples.data();
    desc.samples.stride = sizeof(PxHeightFieldSample);

    const auto start = std::chrono::high_resolution_clock::now();
    PxHeightField* field = cooking_->createHeightField(desc, physics_->getPhysicsInsertionCallback());
    const auto end = std::chrono::high_resolution_clock::now();
    cookStats_.elapsedMs += std::chrono::duration<double, std::milli>(end - start).count();
    if (!field) return false;

    PxHeightFieldGeometry geometry(field, PxMeshGeometryFlags(),
        heightField.heightScale, heightField.rowScale, heightField.columnScale);
    PxShape* shape = physics_->createShape(geometry, *material_, true);
    field->release();
    if (!shape) return false;
    shape->setSimulationFilterData(filterDataFor(group));
    shape->setQueryFilterData(filterDataFor(group));

    PxRigidStatic* actor = physics_->createRigidStatic(PxTransform(PxIdentity));
    if (!actor) {
        shape->release();
        return false;
    }

    actor->attachShape(*shape);
    shape->release();
    scene_->addActor(*actor);
    actors_.push_back(actor);
    return true;
}

RaycastStats PhysXAuditHarness::runRaycastBatch(std::uint32_t rayCount, std::uint32_t rayMask, std::uint32_t seed) {
    RaycastStats stats;
    if (!scene_ || rayCount == 0) return stats;

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> xy(-kSceneExtent, kSceneExtent);

    PxQueryFilterData filterData;
    filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;
    filterData.data.word0 = rayMask;
    MaskQueryFilter queryFilter(&stats.blockedByFilter);

    const auto start = std::chrono::high_resolution_clock::now();
    for (std::uint32_t i = 0; i < rayCount; ++i) {
        PxVec3 origin(xy(rng), xy(rng), kRayOriginZ);
        PxVec3 target(xy(rng), xy(rng), kRayTargetZ);
        PxVec3 dir = target - origin;
        const float dist = dir.magnitude();
        if (dist <= 0.001f || !std::isfinite(dist)) continue;
        dir.normalize();

        PxRaycastBuffer hit;
        const bool status = scene_->raycast(origin, dir, dist, hit,
            PxHitFlag::eDEFAULT, filterData, &queryFilter);
        ++stats.rays;
        if (status) ++stats.hits;
    }
    const auto end = std::chrono::high_resolution_clock::now();
    stats.elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
    return stats;
}

CollisionGroup parseGroup(const std::string& text) {
    if (text == "static") return CollisionGroup::Static;
    if (text == "dynamic") return CollisionGroup::Dynamic;
    if (text == "transparent") return CollisionGroup::Transparent;
    throw std::runtime_error("unknown group: " + text);
}

std::uint32_t parseGroupMask(const std::string& text) {
    if (text.empty() || text == "all") {
        return PhysXAuditHarness::groupMask(CollisionGroup::Static) |
            PhysXAuditHarness::groupMask(CollisionGroup::Dynamic) |
            PhysXAuditHarness::groupMask(CollisionGroup::Transparent);
    }

    std::uint32_t mask = 0;
    for (const std::string& item : split(text, ',')) {
        mask |= PhysXAuditHarness::groupMask(parseGroup(item));
    }
    return mask;
}

} // namespace audit
