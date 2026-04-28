#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <Common/Data.h>
#include <Utils/Utils.h>

class SceneExporter {
public:
    static void ExportToObj(const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            Utils::Log(2, "[EXPORTER] Cannot open file for OBJ export!");
            return;
        }

        int vertexOffset = 1; // OBJ indices start at 1

        auto writeMeshes = [&](const std::vector<std::shared_ptr<TriangleMeshData>>& meshes, const char* groupName) {
            file << "g " << groupName << "\n";
            for (const auto& mesh_ptr : meshes) {
                if (!mesh_ptr) continue;
                const auto& mesh = *mesh_ptr;
                // Toa do: Unreal (X: Forward, Y: Right, Z: Up). 
                // Ghi truc tiep de khi import vao 3D Viewer se thay y nguyen ban cach raycast dang lam viec.
                for (const auto& v : mesh.Vertices) {
                    file << "v " << v.x << " " << v.y << " " << v.z << "\n";
                }
                for (size_t i = 0; i + 2 < mesh.Indices.size(); i += 3) {
                    file << "f " << (mesh.Indices[i] + vertexOffset) << " " 
                                 << (mesh.Indices[i+1] + vertexOffset) << " " 
                                 << (mesh.Indices[i+2] + vertexOffset) << "\n";
                }
                vertexOffset += mesh.Vertices.size();
            }
        };

        int staticCount = 0, rigidCount = 0, heightCount = 0;

        if (GameData.DynamicLoadScene) {
            auto meshes = GameData.DynamicLoadScene->GetAllMeshesPtr();
            staticCount = meshes.size();
            writeMeshes(meshes, "DynamicLoad_Static");
        }
        if (GameData.DynamicRigidScene) {
            auto meshes = GameData.DynamicRigidScene->GetAllMeshesPtr();
            rigidCount = meshes.size();
            writeMeshes(meshes, "DynamicRigid_Vehicles");
        }
        if (GameData.HeightFieldScene) {
            auto meshes = GameData.HeightFieldScene->GetAllMeshesPtr();
            heightCount = meshes.size();
            writeMeshes(meshes, "HeightField_Terrain");
        }

        Utils::Log(1, "[EXPORTER] Meshes Loaded - Static: %d | Vehicles: %d | Terrain: %d", staticCount, rigidCount, heightCount);

        // Ghi ra Camera hien tai
        CameraData cam = Data::GetCamera();
        FVector camAbs = cam.Location + GameData.Radar.WorldOriginLocation;
        file << "g Camera_Position\n";
        file << "v " << camAbs.X << " " << camAbs.Y << " " << camAbs.Z << "\n";
        file << "v " << camAbs.X+20 << " " << camAbs.Y << " " << camAbs.Z << "\n";
        file << "v " << camAbs.X << " " << camAbs.Y+20 << " " << camAbs.Z << "\n";
        file << "f " << vertexOffset << " " << vertexOffset+1 << " " << vertexOffset+2 << "\n";
        vertexOffset += 3;

        // Ghi ra cac Vehicles dang duoc render
        auto vehicles = Data::GetVehicles();
        file << "g Vehicles_Spheres\n";
        for (const auto& pair : vehicles) {
            const VehicleInfo& veh = pair.second;
            if (veh.Location.IsNearlyZero() || veh.Distance > 1000.f) continue;
            
            FVector vehCenter = {veh.Location.X, veh.Location.Y, veh.Location.Z + 100.0f};
            // Mo phong mot hinh thap dai dien cho xe
            file << "v " << vehCenter.X << " " << vehCenter.Y << " " << vehCenter.Z + 250.0f << "\n";
            file << "v " << vehCenter.X + 250.0f << " " << vehCenter.Y - 250.0f << " " << vehCenter.Z - 250.0f << "\n";
            file << "v " << vehCenter.X - 250.0f << " " << vehCenter.Y - 250.0f << " " << vehCenter.Z - 250.0f << "\n";
            file << "v " << vehCenter.X << " " << vehCenter.Y + 250.0f << " " << vehCenter.Z - 250.0f << "\n";

            file << "f " << vertexOffset << " " << vertexOffset+1 << " " << vertexOffset+2 << "\n";
            file << "f " << vertexOffset << " " << vertexOffset+2 << " " << vertexOffset+3 << "\n";
            file << "f " << vertexOffset << " " << vertexOffset+3 << " " << vertexOffset+1 << "\n";
            file << "f " << vertexOffset+1 << " " << vertexOffset+3 << " " << vertexOffset+2 << "\n";
            vertexOffset += 4;
        }

        file.close();
        Utils::Log(1, "[EXPORTER] Exported 3D Scene to: %s", filepath.c_str());
    }
};
