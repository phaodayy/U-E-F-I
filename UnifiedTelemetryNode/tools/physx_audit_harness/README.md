# PhysX Audit Harness

Standalone offline harness for testing PhysX cooking, heightfields, query filters, and raycast cost on local datasets. It does not attach to any external process.

## Build

```powershell
cmake -S tools/physx_audit_harness -B build/physx_audit_harness -DPHYSX_ROOT=D:\SDKs\PhysX
cmake --build build/physx_audit_harness --config Release
```

Copy the PhysX runtime DLLs next to the built executable if your SDK uses dynamic libraries.

## Examples

Cook and raycast an OBJ twice with cache enabled:

```powershell
.\physx_audit_harness.exe --obj .\samples\level.obj --repeat 2 --rays 20000 --cache on
```

Compare without cache:

```powershell
.\physx_audit_harness.exe --obj .\samples\level.obj --repeat 2 --rays 20000 --cache off
```

Load a signed 16-bit RAW heightfield:

```powershell
.\physx_audit_harness.exe --raw .\samples\terrain_i16.raw --raw-width 513 --raw-height 513 --height-scale 0.25 --row-scale 1.0 --col-scale 1.0 --rays 20000
```

Use filter masks:

```powershell
.\physx_audit_harness.exe --obj .\samples\level.obj --group static --ray-mask static,dynamic --rays 10000
```

## Collision Groups

- `static`: walls, buildings, terrain meshes.
- `dynamic`: vehicles or movable props in a synthetic test scene.
- `transparent`: foliage/fences/soft occluders for filter testing.

