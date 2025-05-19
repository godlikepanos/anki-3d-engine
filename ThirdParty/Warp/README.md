# Direct3D WARP NuGet Package

This package contains a copy of D3D10Warp.dll with the latest features and fixes for testing and development purposes. 

The included **LICENSE.txt** applies to all files under `build/native/bin/`.

## Changelog

### Version 1.0.14.1

Fix x86 FMA fusing to correctly fuse `a*b-c` into `FMSUB` instead of `FMADD`.

### Version 1.0.14

#### Codegen improvements

x86/x64:

WARP will now detect and use the following x86 architecture extensions: SSE4.2, AVX, AVX2, FMA, F16C, BMI1, BMI2, and AVX512. Specifically:

- When AVX is supported, VEX encoding is used for applicable instructions to increase code density.
- When AVX2 and FMA are supported, WARP will use 256-bit registers for 64-bit DXIL data types instead of two 128-bit registers.
- When FMA is supported and precise ops are not requested (or when double-precision FMA instructions are used), WARP will use fused multiply-adds.
- When AVX512 is supported, WARP will use mask registers for boolean data types, as well as using the additional 16 vector registers for x64.
- Most instructions were audited for ways to be better-encoded using new instructions available in these instruction sets.
  Of note, AVX512 fills many gaps in comparisons and conversions.
- Large improvements were made to the codegen optimizer to simplify expressions involving constants among other optimizations.

arm64:

- Many operations that did not have an equivalent in SSE4.1 and below, but do in SSE4.1 and above, are updated to use more direct translations to the arm64 instruction set.
- Atomics are upgraded to use arm64 LSE.
- Note: FMA fusing is not yet applied broadly, but DXBC/DXIL double-precision FMA instructions now use fused instructions.

All:

- 64-bit atomics are now emitted directly into shaders instead of requiring function calls.

#### Other

- Raytracing support has been partially rewritten:
    - Acceleration structure traversal is JITed, benefitting from SIMD and all of the above codegen improvements.
	- Shader execution state machine is also JITed, reducing overhead and better aligning DXR1.0 with DXR1.1.
	- Recursive shader invocation overhead is significantly reduced.
- Fixed use of SM6.6 descriptor heap indexing in hit/miss/intersection shaders.
- Fixed PIX timing capture history buffer layout.
- Support for Visual Studio Graphics Analyzer's shader debugger has been removed.
- Handle sampling from an integer texture with a linear sampler by using point sampling instead of crashing.
- Fix calls to `GetDimensions()` on non-uniform resource handles.
- Fix a race condition in acceleration structure builds.
- Wave ops reading from inactive lanes now return 0 (to better match real hardware behavior).

### Version 1.0.13

New features:

- Variable Rate Shading (VRS) Tier 2 support
- Wide wave size emulation support
- Redesigned barriers, more likely to catch app bugs and with higher throughput
- Max virtual address space per resource increased to 40 bits, so resources/heaps can be larger than 4GiB
  - Note: There are some known issues here, but main scenarios should work. Please report bugs to the DirectX Discord (https://discord.com/invite/directx).
- WARP now claims feature level 12_2, aka DirectX 12 Ultimate!
- D3D12 video encoding/decoding support using Media Foundation, only on platforms where Media Foundation codec support is present, for H.264/H.265.

Bug fixes:

- Fix incompatibility between new WARP and old D3D12SDKLayers
- Fixes for work graphs where record reads are done under non-uniform control flow
- Fix a rare race condition which can deadlock work graph backing store initialization
- Fix the default blend state for a generic graphics program in state objects to handle multiple render targets
- Fix ICBs for work graphs (i.e. DXIL global constant data)

### Version 1.0.13-preview

- Work graphs tier 1.1: Mesh nodes

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