cmake_minimum_required(VERSION 3.20)
project(physx_audit_harness LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PHYSX_ROOT "" CACHE PATH "Path to the NVIDIA PhysX SDK root")
if (NOT PHYSX_ROOT)
    message(FATAL_ERROR "Set PHYSX_ROOT to your local PhysX SDK root")
endif()

add_executable(physx_audit_harness
    main.cpp
    physx_audit_harness.cpp
    physx_audit_harness.hpp
)

target_include_directories(physx_audit_harness PRIVATE
    "${PHYSX_ROOT}/include"
    "${PHYSX_ROOT}/pxshared/include"
)

if (MSVC)
    target_compile_options(physx_audit_harness PRIVATE /W4 /EHsc /permissive- /utf-8)
    target_compile_definitions(physx_audit_harness PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

if (WIN32)
    target_link_directories(physx_audit_harness PRIVATE
        "${PHYSX_ROOT}/lib"
        "${PHYSX_ROOT}/bin/win.x86_64.vc143.mt/debug"
        "${PHYSX_ROOT}/bin/win.x86_64.vc143.mt/release"
        "${PHYSX_ROOT}/physx/bin/win.x86_64.vc143.mt/debug"
        "${PHYSX_ROOT}/physx/bin/win.x86_64.vc143.mt/release"
    )
endif()

target_link_libraries(physx_audit_harness PRIVATE
    PhysX_64
    PhysXCommon_64
    PhysXFoundation_64
    PhysXCooking_64
    PhysXExtensions_static_64
)
