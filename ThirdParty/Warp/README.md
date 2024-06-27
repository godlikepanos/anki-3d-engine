# Direct3D WARP NuGet Package

This package contains a copy of D3D10Warp.dll with the latest features and fixes for testing and development purposes. 

The included **LICENSE.txt** applies to all files under `build/native/bin/`.

## Changelog

### Version 1.0.12

New features:

- Now including arm64 binaries

Bug fixes:

- Fix mesh shaders on Win11 retail D3D12
- Fix accessing static const arrays in generic programs
- Fix arm64 instruction encoding which could cause random crashes in shader execution
- Fix state objects exporting the same non-library shader with multiple names/aliases
- Fix handling of zeroing used by lifetime markers on older shader models
- Fix handling of lifetime markers in node shaders
- Handle a null global root signature in a work graph
- Fix mesh -> PS linkage for primitive outputs that were not marked nointerpolate in the PS input
- Fix back-to-back DispatchGraph without a barrier to correctly share the backing memory
- Fix a race condition in BVH builds which could cause corrupted acceleration structures

### Version 1.0.11

New features:

- Shader model 6.8 support
- Work graphs support
- ExecuteIndirect tier 1.1 support

### Version 1.0.10.1

- Fix to support empty ray tracing acceleration structures.
- Miscellaneous bug fixes.

### Version 1.0.9

New features: 

- Sampler feedback support

Bug fixes:
- Improve handling of GetDimensions shader instructions in control flow or with dynamic resource handles
- Fix JIT optimizations that can produce wrong results
- Fix assignment of variable inside of a loop which was uninitialized before entering
- Fix ExecuteIndirect to not modify the app-provided arg buffer
- Fix root signature visibility for amplification and mesh shaders
- Handle DXIL initializers for non-constant global variables
- Fix uninitialized memory causing potential issues with amplification shaders
- Fix deadlock from using the same shader across multiple queues with out-of-order waits
- Some fixes for handling of tiled resources

### Version 1.0.8

- DirectX Raytracing (DXR) 1.1 support.

### Version 1.0.7

- Miscellaneous fixes for 16- and 64-bit shader operations.

### Version 1.0.6

- Miscellaneous fixes and improvements to non-uniform control flow in DXIL shader compilation.
- Fixed casting between block-compressed and uncompressed texture formats.
- Improved conformance of exp2 implementation. 

### Version 1.0.5

- Improved support for non-uniform constant buffer reads. 
- Miscellaneous fp16 conformance fixes. 
- Inverted height support.

### Version 1.0.4

- Miscellaneous sampling conformance fixes. 

### Version 1.0.3

New features: 

- Mesh shader support. 

Bug fixes: 

- Fix a synchronization issue related to null/null aliasing barrier sequences.
- Fix a compatibility issue with older D3D12 Agility SDK versions lacking enhanced barrier support.

### Version 1.0.2

- Expanded support for 64-bit conversion operations. 
- Fixes issue where static samplers could incorrectly use non-normalized coordinates.

### Version 1.0.1

New features:

- Support for Shader Model 6.3-6.7 required features, including:
  - 64-bit loads, stores, and atomics on all available types of resources.
  - Descriptor heap indexing, aka Shader Model 6.6 dynamic resources.
  - Quad ops and sampling derivatives in compute shaders.
  - Advanced texture ops, featuring integer texture point sampling, raw gather, dynamic texel offsets, and MSAA UAVs.

- Support for many new Direct3D 12 Agility SDK features, including:
  - Enhanced barriers. 
  - Relaxed resource format casting. 
  - Independent front/back stencil refs and masks.
  - Triangle fans.
  - Relaxed buffer/texture copy pitches.
  - Independent D3D12 devices via ID3D12DeviceFactory::CreateDevice

Bug fixes:

- Various conformance and stability fixes.