#pragma once
#include <embree4/rtcore.h>
#include <embree4/rtcore_ray.h>
#include <mutex>
#include <shared_mutex>  // Them ho tro read-write lock
#include "math.h"
#include <vector>
#include <memory>
#include <set>
#include <features/Physx.h>
#include <Utils/Utils.h>
#include <atomic>


namespace Physics {

    using namespace GamePhysX;
    using namespace std;

    static void embreeErrorFunction(void* userPtr, RTCError code, const char* str) {
        //Utils::Log(2, "Embree Error [%d]: %s", code, str);
    }

    // RAII guard for geometry release
    class GeometryGuard {
    public:
        GeometryGuard(RTCGeometry geom) : geometry(geom) {}
        ~GeometryGuard() {
            if (geometry) rtcReleaseGeometry(geometry);
        }
    private:
        RTCGeometry geometry;
    };

    template <typename T, typename Hash>
    class VisibleScene
    {
    public:
        using KeyExtractor = T(*)(const TriangleMeshData&);

        VisibleScene(KeyExtractor keyExtractor) :
            getKey(keyExtractor), isShuttingDown(false) {
            try {
                this->device = rtcNewDevice(nullptr);
                if (!this->device) {
                    return;
                }

                rtcSetDeviceErrorFunction(device, embreeErrorFunction, nullptr);
                this->scene = rtcNewScene(device);
                if (!this->scene) {
                    rtcReleaseDevice(device);
                    device = nullptr;
                    return;
                }

                rtcSetDeviceProperty(device, RTC_DEVICE_PROPERTY_TASKING_SYSTEM, 1); // USE TBB
                rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_LOW);
                rtcSetSceneFlags(scene, RTC_SCENE_FLAG_DYNAMIC);
                rtcCommitScene(this->scene);
            }
            catch (...) {
                if (scene) {
                    rtcReleaseScene(scene);
                    scene = nullptr;
                }
                if (device) {
                    rtcReleaseDevice(device);
                    device = nullptr;
                }
            }
        }

        ~VisibleScene() {
            isShuttingDown = true;
            try {
                std::unique_lock<std::shared_mutex> lock(sceneRWMutex);
                mesh_datas.clear();
                geometries.clear();
                if (scene) {
                    rtcReleaseScene(scene);
                    scene = nullptr;
                }
                if (device) {
                    rtcReleaseDevice(device);
                    device = nullptr;
                }
            }
            catch (...) {}
        }

        void UpdateMesh(const vector<TriangleMeshData>& willAddMeshs, const std::set<T>& RemoveKey) {
            if (isShuttingDown) return;
            std::unique_lock<std::shared_mutex> lock(sceneRWMutex);
            if (!scene || !device) return;

            try {
                for (const auto& key : RemoveKey) {
                    if (geometry_id_map.find(key) != geometry_id_map.end()) {
                        auto geometry_id = geometry_id_map[key];
                        auto geometry = rtcGetGeometry(scene, geometry_id);
                        if (geometry) {
                            rtcDisableGeometry(geometry);
                            disabled_geometry_ids.insert(geometry_id);
                        }
                        geometry_id_map.erase(key);
                    }
                }

                if (!mesh_datas.empty()) {
                    mesh_datas.erase(
                        remove_if(mesh_datas.begin(), mesh_datas.end(),
                            [this, &RemoveKey](const shared_ptr<TriangleMeshData>& mesh) {
                                return RemoveKey.find(this->getKey(*mesh)) != RemoveKey.end();
                            }
                        ),
                        mesh_datas.end()
                    );
                }

                for (const auto& mesh : willAddMeshs) {
                    if (mesh.Vertices.size() == 0 || mesh.Indices.size() == 0) continue;

                    RTCGeometry geom = nullptr;
                    bool should_release = false;
                    uint32_t geometry_id = 0;
                    auto mesh_copy = make_shared<TriangleMeshData>(mesh);
                    mesh_datas.push_back(mesh_copy);

                    if (!disabled_geometry_ids.empty()) {
                        geometry_id = *disabled_geometry_ids.begin();
                        disabled_geometry_ids.erase(disabled_geometry_ids.begin());
                        geom = rtcGetGeometry(scene, geometry_id);
                        if (geom) rtcEnableGeometry(geom);
                    }
                    else {
                        geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
                        should_release = true;
                    }

                    if (!geom) continue;

                    float* vertices = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), mesh.Vertices.size());
                    if (!vertices) {
                        handleGeometryError(geom, should_release, geometry_id);
                        continue;
                    }

                    for (size_t i = 0; i < mesh.Vertices.size(); i++) {
                        vertices[i * 3] = mesh.Vertices[i].x;
                        vertices[i * 3 + 1] = mesh.Vertices[i].y;
                        vertices[i * 3 + 2] = mesh.Vertices[i].z;
                    }

                    auto bufferSize = mesh.Indices.size() / 3;
                    unsigned int* indices = (unsigned int*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned int), bufferSize);
                    if (!indices) {
                        handleGeometryError(geom, should_release, geometry_id);
                        continue;
                    }

                    memcpy(indices, mesh.Indices.data(), mesh.Indices.size() * sizeof(uint32_t));
                    rtcSetGeometryUserData(geom, mesh_copy.get());

                    // Nang cap: Transparency Check (Loai bo luoi thep, hang rao, la cay)
                    rtcSetGeometryIntersectFilterFunction(geom, [](const RTCFilterFunctionNArguments* args) {
                        int* valid = args->valid;
                        if (!valid[0]) return;
                        
                        auto meshData = (TriangleMeshData*)args->geometryUserPtr;
                        if (!meshData) return;
                        
                        // Kiem tra thuoc tinh xuyen thau thong qua FilterData
                        // word0 = Collision Channel (0 = No collision, thuong la Foliage/Decor)
                        // word3 = Object Type Flags (gia tri thap = vat the nho/xuyen thau)
                        if (meshData->Type == PxGeometryType::eTRIANGLEMESH) {
                            // Bo qua neu la Foliage, ChainlinkFence, hoac vat the ko co collision
                            if (meshData->QueryFilterData.word0 == 0 && meshData->SimulationFilterData.word0 == 0) {
                                valid[0] = 0; // Ignore: vat the nay khong can chan tia raycast
                                return;
                            }
                            // Bo qua cac vat the co word3 qua thap (thuong la co, bui, decal)
                            if (meshData->QueryFilterData.word3 > 0 && meshData->QueryFilterData.word3 < 50) {
                                valid[0] = 0;
                                return;
                            }
                        }
                    });

                    rtcCommitGeometry(geom);

                    if (should_release) {
                        geometry_id = rtcAttachGeometry(scene, geom);
                        geometries.push_back(std::shared_ptr<void>(geom, [](RTCGeometry g) { if (g) rtcReleaseGeometry(g); }));
                    }
                    geometry_id_map.emplace(getKey(mesh), geometry_id);
                }
                rtcCommitScene(scene);
            }
            catch (...) {}
        }

        void handleGeometryError(RTCGeometry geom, bool should_release, uint32_t geometry_id) {
            if (should_release) rtcReleaseGeometry(geom);
            else {
                rtcDisableGeometry(geom);
                disabled_geometry_ids.insert(geometry_id);
            }
        }

        RTCRayHit Raycast(const FVector& origin, const FVector& target) {
            if (isShuttingDown) {
                RTCRayHit rayhit; rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID; return rayhit;
            }

            try {
                std::shared_lock<std::shared_mutex> lock(sceneRWMutex, std::try_to_lock);
                if (!lock.owns_lock()) {
                    RTCRayHit rayhit; rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID - 1; return rayhit;
                }

                if (!scene || !device) {
                    RTCRayHit rayhit; rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID; return rayhit;
                }

                RTCRay ray;
                memset(&ray, 0, sizeof(RTCRay));
                ray.org_x = origin.X; ray.org_y = origin.Y; ray.org_z = origin.Z;
                ray.dir_x = target.X - origin.X; ray.dir_y = target.Y - origin.Y; ray.dir_z = target.Z - origin.Z;

                float dir_length = std::sqrt(ray.dir_x * ray.dir_x + ray.dir_y * ray.dir_y + ray.dir_z * ray.dir_z);
                if (dir_length < 1e-6f) {
                    RTCRayHit rayhit; rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID; return rayhit;
                }

                ray.dir_x /= dir_length; ray.dir_y /= dir_length; ray.dir_z /= dir_length;
                ray.tnear = 0.1f; // Giam xuong 0.1 de khong bo qua vat the gan
                ray.tfar = dir_length; 
                ray.mask = -1; ray.flags = 0; ray.time = 0.0f; ray.id = 0;

                RTCRayHit rayhit;
                memset(&rayhit, 0, sizeof(RTCRayHit));
                rayhit.ray = ray;
                rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
                rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

                RTCRayQueryContext context; rtcInitRayQueryContext(&context);
                RTCIntersectArguments intersectArgs; rtcInitIntersectArguments(&intersectArgs);
                intersectArgs.context = &context;

                rtcIntersect1(scene, &rayhit, &intersectArgs);
                return rayhit;
            }
            catch (...) {
                RTCRayHit rayhit; rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID; return rayhit;
            }
        }

        TriangleMeshData* GetGeomeoryData(uint32_t geomId) {
            if (isShuttingDown) return nullptr;
            try {
                std::shared_lock<std::shared_mutex> lock(sceneRWMutex);
                if (!scene) return nullptr;
                auto geometry = rtcGetGeometry(scene, geomId);
                if (!geometry) return nullptr;
                return (TriangleMeshData*)rtcGetGeometryUserData(geometry);
            }
            catch (...) { return nullptr; }
        }

        std::vector<std::shared_ptr<TriangleMeshData>> GetAllMeshesPtr() const {
            std::shared_lock<std::shared_mutex> lock(sceneRWMutex);
            return mesh_datas;
        }

    private:
        RTCDevice device;
        RTCScene  scene;
        std::unordered_map<T, uint32_t, Hash> geometry_id_map = {};
        std::set<uint32_t> disabled_geometry_ids = {};
        vector<shared_ptr<TriangleMeshData>> mesh_datas = {};
        vector<shared_ptr<void>> geometries;
        KeyExtractor getKey;
        mutable std::shared_mutex sceneRWMutex;
        std::atomic<bool> isShuttingDown;
    };

} // namespace Physics
