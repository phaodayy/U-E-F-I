#include "physx_audit_harness.hpp"

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string objPath;
    std::string rawPath;
    std::uint32_t rawRows = 0;
    std::uint32_t rawColumns = 0;
    float rowScale = 1.0f;
    float columnScale = 1.0f;
    float heightScale = 1.0f;
    std::uint32_t rays = 10000;
    std::uint32_t repeat = 1;
    std::uint32_t seed = 1337;
    bool cache = true;
    audit::CollisionGroup group = audit::CollisionGroup::Static;
    std::string rayMask = "all";
};

std::string requireValue(int& i, int argc, char** argv) {
    if (i + 1 >= argc) throw std::runtime_error(std::string("missing value for ") + argv[i]);
    return argv[++i];
}

Options parseOptions(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--obj") opt.objPath = requireValue(i, argc, argv);
        else if (arg == "--raw") opt.rawPath = requireValue(i, argc, argv);
        else if (arg == "--raw-width") opt.rawColumns = static_cast<std::uint32_t>(std::stoul(requireValue(i, argc, argv)));
        else if (arg == "--raw-height") opt.rawRows = static_cast<std::uint32_t>(std::stoul(requireValue(i, argc, argv)));
        else if (arg == "--row-scale") opt.rowScale = std::stof(requireValue(i, argc, argv));
        else if (arg == "--col-scale") opt.columnScale = std::stof(requireValue(i, argc, argv));
        else if (arg == "--height-scale") opt.heightScale = std::stof(requireValue(i, argc, argv));
        else if (arg == "--rays") opt.rays = static_cast<std::uint32_t>(std::stoul(requireValue(i, argc, argv)));
        else if (arg == "--repeat") opt.repeat = static_cast<std::uint32_t>(std::stoul(requireValue(i, argc, argv)));
        else if (arg == "--seed") opt.seed = static_cast<std::uint32_t>(std::stoul(requireValue(i, argc, argv)));
        else if (arg == "--cache") {
            const std::string value = requireValue(i, argc, argv);
            opt.cache = (value == "on" || value == "true" || value == "1");
        } else if (arg == "--group") opt.group = audit::parseGroup(requireValue(i, argc, argv));
        else if (arg == "--ray-mask") opt.rayMask = requireValue(i, argc, argv);
        else if (arg == "--help" || arg == "-h") {
            std::cout
                << "physx_audit_harness\n"
                << "  --obj <file.obj>\n"
                << "  --raw <file.raw> --raw-width <n> --raw-height <n>\n"
                << "  --cache on|off --repeat <n> --rays <n>\n"
                << "  --group static|dynamic|transparent --ray-mask static,dynamic,transparent|all\n";
            std::exit(0);
        } else {
            throw std::runtime_error("unknown argument: " + arg);
        }
    }
    return opt;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Options opt = parseOptions(argc, argv);
        if (opt.objPath.empty() && opt.rawPath.empty()) {
            throw std::runtime_error("provide --obj or --raw");
        }

        audit::PhysXAuditHarness harness;
        if (!harness.initialize()) {
            throw std::runtime_error("failed to initialize PhysX");
        }

        if (!opt.objPath.empty()) {
            const audit::MeshData mesh = audit::ObjLoader::Load(opt.objPath);
            for (std::uint32_t i = 0; i < opt.repeat; ++i) {
                if (!harness.addTriangleMesh(mesh, opt.group, opt.cache)) {
                    throw std::runtime_error("failed to add OBJ mesh");
                }
            }
        }

        if (!opt.rawPath.empty()) {
            if (opt.rawRows == 0 || opt.rawColumns == 0) {
                throw std::runtime_error("--raw requires --raw-width and --raw-height");
            }
            const audit::HeightFieldData hf = audit::RawHeightFieldLoader::LoadI16(
                opt.rawPath, opt.rawRows, opt.rawColumns, opt.rowScale, opt.columnScale, opt.heightScale);
            if (!harness.addHeightField(hf, opt.group)) {
                throw std::runtime_error("failed to add RAW heightfield");
            }
        }

        const std::uint32_t rayMask = audit::parseGroupMask(opt.rayMask);
        const audit::RaycastStats rayStats = harness.runRaycastBatch(opt.rays, rayMask, opt.seed);
        const audit::CookStats& cook = harness.cookStats();

        std::cout << "actors=" << harness.actorCount()
                  << " cached_meshes=" << harness.cachedMeshCount() << "\n";
        std::cout << "cook_requests=" << cook.meshCookRequests
                  << " cache_hits=" << cook.meshCookCacheHits
                  << " cache_misses=" << cook.meshCookCacheMisses
                  << " heightfield_requests=" << cook.heightFieldCookRequests
                  << " cook_ms=" << cook.elapsedMs << "\n";
        std::cout << "rays=" << rayStats.rays
                  << " hits=" << rayStats.hits
                  << " filtered=" << rayStats.blockedByFilter
                  << " raycast_ms=" << rayStats.elapsedMs << "\n";
        if (rayStats.rays > 0) {
            std::cout << "avg_raycast_us=" << (rayStats.elapsedMs * 1000.0 / static_cast<double>(rayStats.rays)) << "\n";
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
