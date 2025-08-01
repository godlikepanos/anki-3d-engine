# DirectX 12 Agility SDK Redistributable NuGet Package

This package contains a copy of the DirectX 12 Agility SDK redistributable runtime and its associated development headers. 

For help getting started and other information for the Agility SDK, please see:

https://aka.ms/directx12agility

The included licenses apply to the following files:

- **LICENSE.txt** : applies to all files under `build/native/bin/`
- **LICENSE-CODE.txt** : applies to all files under `build/native/include/`

## Changelog

### Version 1.616.1:
 
- Runtime fix: D3D12EnableExperimentalFeatures() was allowing D3D12ExperimentalShaderModels
  - This made it look like Shader Model 6.9 could work even though it's only supported in the 1.717.x preview
  - Now asking for D3D12ExperimentalShaderModels returns E_NOINTERFACE.
- Debug Layer fixes:
  - Enhanced Barrier placed resource validation fix for false errors
  - For D3D12_BARRIER_SUBRESOURCE_RANGE, the Debug Layer was not handling the NumMipLevels=0 special case (treat IndexOrFirstMipLevel as a subresource index)
  - Debug Layer updated to notice that RD/DS initialization can be accomplished by RenderPass Clear/Discard
- GPU based validation (GBV):
  - Fix for incorrect texture state/layout validation in cases where GBV needed to insert LOD calculation

### Version 1.717.0
- Everything in 1.616.0 plus:
- Shader Model 6.9 preview, adding:
  - Cooperative Vectors
  - Shader Execution Reordering
  - Support for RayQuery using Opacity Micromaps
    - Normal TraceRay doens't require SM 6.9, so the 1.616.0 release works for it
- D3D Video Encoding updates

In this preview, mesh nodes is disabled, but could return in a future preview and/or retail release.

### Version 1.616.0
- Release Opacity Micromap support
  - Just missing RayQuery+OMM suport that needs Shader Model 6.9 which is in the above preview
- Tier 4 Tiled Resources

### Version 1.716.1
- Fix meta command validation
- Add Raw UAV/SRV validation for tight aligned buffers
- Fix a bug where AlignmentRestriction hint wasn’t being passed to drivers
- Fix placed resource initialization validation bug related to plane validation
- Fix a bug where GBV patching will crash when initializing if the caller didn’t request a debug info parser

### Version 1.716.0
- Application Specific Driver State
- RecreateAt GPUVA
- Runtime Bypass
- Shader hash bypass 
- Tight Alignment of Resources
- Multiple video features
  - Encode subregion notifications
  - Encode output stats
  - Encode GPU texture input map
  - Encode GPU texture/CPU buffer dirty maps/rects
  - Encode GPU texture/CPU buffer motion vector hints

In this preview, mesh nodes is disabled, but could return in a future preview and/or retail release.

### Verision 1.615.0
- Shader hash bypass officially supports applications opting to bypass shader hash validation

### Version 1.715.0
- Preview of mesh nodes in work graphs

### Version 1.714.0
- Preview release of DirectSR
  - Supports both native and built-in Super Resolution techniques ("variants")

### Version 1.614.0
- Enabled R9G9B9E5_SHAREDEXP format for Render Target and Unordered Access Views

### Version 1.613.3

- Same as 1.613.2, with minor updates shown at the end of this list:
- Work Graphs
- Generic Programs in State objects
- Shader Model 6.8
  - Work Graphs support
  - Start Vertex/Instance Location
  - Wave Size Range
  - Expanded Comparison Sampling
- GPU Upload Heaps (requires preview or future OS, or current OS in developer mode)
- Incrementing Constants in ExecuteIndirect
- Minor updates for .3:
   - Honor root signature associations added to export in collections when the export is imported into generic program in an executable state object.
   - Propagate root signatures to exports that have bindings and no root signatures within the scope of a generic program, like what happens with PSOs.
   - Other generic programs related bug fixes, additional validation and optimizations.
   - For state objects, minor fixes to subobject association logic in edge cases involving collections to more precisely match spec wording.
   - GPU Upload Heaps can now be used with a current OS in developer mode, in addition to preview and future OS that don't require developer mode.

### Version 1.613.2

- Same as 1.613.1, with minor updates shown at the end of this list:
- Work Graphs
- Generic Programs in State objects
- Shader Model 6.8
  - Work Graphs support
  - Start Vertex/Instance Location
  - Wave Size Range
  - Expanded Comparison Sampling
- GPU Upload Heaps (requires preview or future OS)
- Incrementing Constants in ExecuteIndirect
- Minor updates for .2:
   - Close validation hole: Prevent use of features in incompatible state object types, such as defining generic program components (like rast state) in a raytracing pipeline.
   - Debug layer fixes around lifetime management of program identifiers (e.g. the handles for the new generic programs)
   - ExecuteIndirect validation was too strict: buffer size only needs to be big enough for the number of commands * stride, _minus the unused space at end of last command_

### Version 1.613.1

- Same as 1.613.0, with minor updates shown at the end of this list:
- Work Graphs
- Generic Programs in State objects
- Shader Model 6.8
  - Work Graphs support
  - Start Vertex/Instance Location
  - Wave Size Range
  - Expanded Comparison Sampling
- GPU Upload Heaps (requires preview or future OS)
- Incrementing Constants in ExecuteIndirect
- Minor updates for .1:
   - In d3d12.h: missing const on input pointer members: D3D12_NODE_CPU_INPUT.pRecords and D3D12_MULTI_NODE_CPU_INPUT.pNodeInputs
   - Debug layer validation fixes for D3D12_DISPATCH_MODE_MULTI_NODE_CPU_INPUT
   - Some missing validation for generic programs (to line up with existing PSO path)
   - Allow generic programs or work graphs only in executable state objects, not collections

### Version 1.613.0

- Work Graphs
- Generic Programs in State objects
- Shader Model 6.8
  - Work Graphs support
  - Start Vertex/Instance Location
  - Wave Size Range
  - Expanded Comparison Sampling
- GPU Upload Heaps (requires preview or future OS)
- Incrementing Constants in ExecuteIndirect

### Version 1.611.1

* Make pContext mutable in ID3D12InfoQueue1::RegisterMessageCallback.
* Fix Linux build issue.
* Legacy transitions into/out-of UAV state includes D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE, 
  D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO and 
  D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE. This only applies to buffer barriers.
* Fix enhanced barrier SYNC_DRAW being incompatible with ACCESS_RENDER_TARGET
* Improve enhanced barrier validation for raytracing acceleration data structures

### Version 1.611.0

* Video AV1 Encode release

### Version 1.610.3

- Fix regression introduced in 1.608.0 as part of independent front/back stencil support. 
  If an app used the stencil mask in a way that only affected back facing geometry (even 
  using the old depth stencil desc that had just a single mask for front/back), 
  the D3D runtime would incorrectly set the stencil mask to 0 on certain drivers. Specifically 
  drivers older than the timeframe of the 1.608.0 release (Nov 2022).

### Version 1.610.2

- Minor fix for Non-Normalized Sampling: The 610 release had some stale logic
  that left Non-Normalized Sampling in "prerelease" mode, so it appeared as not available.

### Version 1.610.1

- Minor fix for RenderPasses: The 610 release had some stale logic
  that left RenderPasses in "prerelease" mode, so it appeared as not available.

### Version 1.610.0

- Updated RenderPass support for tile based rendering GPUs
- Several minor features for improved compatibility with Vulkan
  - Non-normalized sampling
  - Mismatched render target dimensions
  - Sample-frequency pixel shaders with no RTV/DSV
  - And a handful of even more minor features.
  - Fixes a regression introduced in 1.608.0 where the stencil mask could get cleared
    when specifying it in a way that only affected the back-facing geometry
  
### Version 1.608.3

- Fixed state corruption causing incorrect programmable sample positions validation during ExecuteCommandList
- Address incorrect Enhanced Barriers simultaneous-access validation
- Fix incorrect Enhanced Barriers sync/access validation
- Add missing string conversion for D3D12_BARRIER_SYNC_CLEAR_UNORDERED_ACCESS_VIEW

### Version 1.608.0

* Enhanced Barriers release
* Independent front/back stencil refs and masks 
* Triangle fans 
* Dynamic depth bias and IB strip cut 
  
### Version 1.706.4 (preview)

- Fixes a device creation failure on WARP (Microsoft Basic Render) in Windows Server 2022.

### Version 1.606.4

- Fixes a device creation failure on WARP (Microsoft Basic Render) in Windows Server 2022.

### Version 1.706.3 (preview)

- Enhanced Barriers Preview 2 with GBV support
- Adds `ID3D12DebugCommandList3::AssertTextureLayout` and ` ID3D12DebugCommandQueue1::AssertResourceAccess` methods. 
- Independent Front/Back Stencil Refs and Masks 
- Triangle fans are back

### Version 1.606.3

New features:

- Shader Model 6.7.
- d3dconfig: settings import/export.
- d3dconfig: option to allow application control over storage filters
- DRED: 'markers only breadcrumbs' stores breadcrumbs only for PIX markers and events.

Bug fixes:

- Various debug layer stability fixes.

### Version 1.602.0

New features:

- Relaxed buffer/texture copy alignment
- Support for copying between different dimensions of textures
- Delayed input layout and vertex buffer alignment validation
- Negative height viewports flip y-axis intepretation
- Alpha/InvAlpha blend factors

Bugfixes

- Fixes a crash using GBV with shader patch mode TRACKING_ONLY. 
- Fixes false debug validation output resulting from depth slice state being confused with stencil slice state. 
- Fixes a bug causing promoted COPY_DEST to not decay back to COMMON. 
- Report live objects when encountering device removed from a kernel memory failure.  
  
### Version 1.600.10

Fixes threading bug (intermittent crash) in runtime for apps doing multithreaded creation of raytracing state objects.

Ideally this fix would have been made in isolation on top of the previous SDK release.   Unfortunately this was not possible due to build infrastructure changes - hopefully a one-time issue.  What this means is that this SDK's runtime reflects the current state of the D3D12 codebase including code churn unrelated to the bug fix, such as support for preview features that are disabled here and exposed in a separate preview SDK branch.  Even though preview features are disabled, the codebase is still different.  So there is some risk of bugs/regressions that have not been noticed yet in internal testing.  In particular there is a reasonable chance that the churn in the debug layer codebase might yield some debug validation issues, but there could be issues lurking in the runtime as well.  If you report any issues you observe to us, we will try to address them with a follow-up SDK release.

Whether you choose to use this bugfix release, with its extra code churn of no value to you, really boils down to how important the specific fix is to you (if at all), and perhaps how much capacity you have to do test this combination of runtime and your app.

### Version 1.4.10 

Fixes a debug layer issue where some ResourceBarrier calls transitioning DEPTH_READ to DEPTH_WRITE were dropped

### Version 1.4.9

Contains support for DirectX 12 Ultimate and Shader Model 6.6