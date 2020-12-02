// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/resource/RenderingKey.h>
#include <anki/ui/Canvas.h>
#include <anki/shaders/include/ClusteredShadingTypes.h>
#include <anki/shaders/include/ModelTypes.h>

namespace anki
{

/// @addtogroup renderer
/// @{

class RenderingMatrices
{
public:
	Mat4 m_cameraTransform;
	Mat4 m_viewMatrix;
	Mat4 m_projectionMatrix;
	Mat4 m_viewProjectionMatrix;
	Mat4 m_previousViewProjectionMatrix;
};

/// Some options that can be used as hints in debug drawcalls.
enum class RenderQueueDebugDrawFlag : U32
{
	DEPTH_TEST_ON,
	DITHERED_DEPTH_TEST_ON,
	COUNT
};

/// Context that contains variables for drawing and will be passed to RenderQueueDrawCallback.
class RenderQueueDrawContext final : public RenderingMatrices
{
public:
	RenderingKey m_key;
	CommandBufferPtr m_commandBuffer;
	SamplerPtr m_sampler; ///< A trilinear sampler with anisotropy.
	StagingGpuMemoryManager* m_stagingGpuAllocator ANKI_DEBUG_CODE(= nullptr);
	StackAllocator<U8> m_frameAllocator;
	Bool m_debugDraw; ///< If true the drawcall should be drawing some kind of debug mesh.
	BitSet<U(RenderQueueDebugDrawFlag::COUNT), U32> m_debugDrawFlags = {false};
};

/// Draw callback for drawing.
using RenderQueueDrawCallback = void (*)(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);

/// Render queue element that contains info on items that populate the G-buffer or the forward shading buffer etc.
class RenderableQueueElement final
{
public:
	RenderQueueDrawCallback m_callback;
	const void* m_userData;

	/// Elements with the same m_mergeKey and same m_callback may be merged and the m_callback will be called once.
	/// Unless m_mergeKey is zero.
	U64 m_mergeKey;

	F32 m_distanceFromCamera; ///< Don't set this

	RenderableQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<RenderableQueueElement>::value == true,
			  "Should be trivially destructible");

/// Context that contains variables for the GenericGpuComputeJobQueueElement.
class GenericGpuComputeJobQueueElementContext final : public RenderingMatrices
{
public:
	CommandBufferPtr m_commandBuffer;
	StagingGpuMemoryManager* m_stagingGpuAllocator ANKI_DEBUG_CODE(= nullptr);
};

/// Callback for GenericGpuComputeJobQueueElement.
using GenericGpuComputeJobQueueElementCallback = void (*)(GenericGpuComputeJobQueueElementContext& ctx,
														  const void* userData);

/// It has enough info to execute generic compute on the GPU.
class GenericGpuComputeJobQueueElement final
{
public:
	GenericGpuComputeJobQueueElementCallback m_callback;
	const void* m_userData;

	GenericGpuComputeJobQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<GenericGpuComputeJobQueueElement>::value == true,
			  "Should be trivially destructible");

/// Point light render queue element.
class PointLightQueueElement final
{
public:
	U64 m_uuid;
	Vec3 m_worldPosition;
	F32 m_radius;
	Vec3 m_diffuseColor;
	Array<RenderQueue*, 6> m_shadowRenderQueues;
	RenderQueueDrawCallback m_debugDrawCallback;
	const void* m_debugDrawCallbackUserData;

	Array<Vec2, 6> m_shadowAtlasTileOffsets; ///< Renderer internal.
	F32 m_shadowAtlasTileSize; ///< Renderer internal.
	U8 m_shadowLayer; ///< Renderer internal.

	PointLightQueueElement()
	{
	}

	Bool hasShadow() const
	{
		return m_shadowRenderQueues[0] != nullptr;
	}
};

static_assert(std::is_trivially_destructible<PointLightQueueElement>::value == true,
			  "Should be trivially destructible");

/// Spot light render queue element.
class SpotLightQueueElement final
{
public:
	U64 m_uuid;
	Mat4 m_worldTransform;
	Mat4 m_textureMatrix;
	F32 m_distance;
	F32 m_outerAngle;
	F32 m_innerAngle;
	Vec3 m_diffuseColor;
	RenderQueue* m_shadowRenderQueue;
	RenderQueueDrawCallback m_debugDrawCallback;
	const void* m_debugDrawCallbackUserData;

	U8 m_shadowLayer; ///< Renderer internal.

	SpotLightQueueElement()
	{
	}

	Bool hasShadow() const
	{
		return m_shadowRenderQueue != nullptr;
	}
};

static_assert(std::is_trivially_destructible<SpotLightQueueElement>::value == true, "Should be trivially destructible");

/// Directional light render queue element.
class DirectionalLightQueueElement final
{
public:
	Array<Mat4, MAX_SHADOW_CASCADES> m_textureMatrices;
	Array<RenderQueue*, MAX_SHADOW_CASCADES> m_shadowRenderQueues;
	RenderQueueDrawCallback m_drawCallback;
	const void* m_drawCallbackUserData;
	U64 m_uuid; ///< Zero means that there is no dir light
	Vec3 m_diffuseColor;
	Vec3 m_direction;
	F32 m_effectiveShadowDistance;
	F32 m_shadowCascadesDistancePower;
	U8 m_shadowCascadeCount; ///< Zero means that it doesn't case any shadows
	U8 m_shadowLayer; ///< Renderer internal.

	DirectionalLightQueueElement()
	{
	}

	ANKI_USE_RESULT Bool isEnabled() const
	{
		return m_uuid != 0;
	}

	ANKI_USE_RESULT Bool hasShadow() const
	{
		return isEnabled() && m_shadowCascadeCount > 0;
	}
};

static_assert(std::is_trivially_destructible<DirectionalLightQueueElement>::value == true,
			  "Should be trivially destructible");

/// Normally the visibility tests don't perform tests on the reflection probes because probes dont change that often.
/// This callback will be used by the renderer to inform a reflection probe that on the next frame it will be rendererd.
/// In that case the visibility tests should fill the render queues of the probe.
using ReflectionProbeQueueElementFeedbackCallback = void (*)(Bool fillRenderQueuesOnNextFrame, void* userData);

/// Reflection probe render queue element.
class ReflectionProbeQueueElement final
{
public:
	U64 m_uuid;
	ReflectionProbeQueueElementFeedbackCallback m_feedbackCallback;
	void* m_feedbackCallbackUserData;
	RenderQueueDrawCallback m_debugDrawCallback;
	const void* m_debugDrawCallbackUserData;
	Array<RenderQueue*, 6> m_renderQueues;
	Vec3 m_worldPosition;
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
	U32 m_textureArrayIndex; ///< Renderer internal.

	ReflectionProbeQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<ReflectionProbeQueueElement>::value == true,
			  "Should be trivially destructible");

/// See ReflectionProbeQueueElementFeedbackCallback for its purpose.
using GlobalIlluminationProbeQueueElementFeedbackCallback = void (*)(Bool fillRenderQueuesOnNextFrame, void* userData,
																	 const Vec4& eyeWorldPosition);

// Probe for global illumination.
class GlobalIlluminationProbeQueueElement final
{
public:
	U64 m_uuid;
	GlobalIlluminationProbeQueueElementFeedbackCallback m_feedbackCallback;
	void* m_feedbackCallbackUserData;
	RenderQueueDrawCallback m_debugDrawCallback;
	const void* m_debugDrawCallbackUserData;
	Array<RenderQueue*, 6> m_renderQueues;
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
	UVec3 m_cellCounts;
	U32 m_totalCellCount;
	Vec3 m_cellSizes; ///< The cells might not be cubes so have different sizes per dimension.
	F32 m_fadeDistance;

	GlobalIlluminationProbeQueueElement()
	{
	}

	Bool operator<(const GlobalIlluminationProbeQueueElement& b) const
	{
		if(m_cellSizes.x() != b.m_cellSizes.x())
		{
			return m_cellSizes.x() < b.m_cellSizes.x();
		}
		else
		{
			return m_totalCellCount < b.m_totalCellCount;
		}
	}
};

static_assert(std::is_trivially_destructible<GlobalIlluminationProbeQueueElement>::value == true,
			  "Should be trivially destructible");

/// Lens flare render queue element.
class LensFlareQueueElement final
{
public:
	/// Totaly unsafe but we can't have a smart ptr in here since there will be no deletion.
	const TextureView* m_textureView;
	const void* m_userData;
	RenderQueueDrawCallback m_drawCallback;
	Vec3 m_worldPosition;
	Vec2 m_firstFlareSize;
	Vec4 m_colorMultiplier;

	LensFlareQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<LensFlareQueueElement>::value == true, "Should be trivially destructible");

/// Decal render queue element.
class DecalQueueElement final
{
public:
	RenderQueueDrawCallback m_debugDrawCallback;
	const void* m_debugDrawCallbackUserData;
	/// Totaly unsafe but we can't have a smart ptr in here since there will be no deletion.
	TextureView* m_diffuseAtlas;
	/// Totaly unsafe but we can't have a smart ptr in here since there will be no deletion.
	TextureView* m_specularRoughnessAtlas;
	Vec4 m_diffuseAtlasUv;
	Vec4 m_specularRoughnessAtlasUv;
	F32 m_diffuseAtlasBlendFactor;
	F32 m_specularRoughnessAtlasBlendFactor;
	Mat4 m_textureMatrix;
	Vec3 m_obbCenter;
	Vec3 m_obbExtend;
	Mat3 m_obbRotation;

	DecalQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<DecalQueueElement>::value == true, "Should be trivially destructible");

/// Draw callback for drawing.
using UiQueueDrawCallback = void (*)(CanvasPtr& canvas, void* userData);

/// UI element render queue element.
class UiQueueElement final
{
public:
	void* m_userData;
	UiQueueDrawCallback m_drawCallback;

	UiQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<UiQueueElement>::value == true, "Should be trivially destructible");

/// Fog density queue element.
class FogDensityQueueElement final
{
public:
	union
	{
		Vec3 m_aabbMin;
		Vec3 m_sphereCenter;
	};

	union
	{
		Vec3 m_aabbMax;
		F32 m_sphereRadius;
	};

	F32 m_density;
	Bool m_isBox;

	FogDensityQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<FogDensityQueueElement>::value == true,
			  "Should be trivially destructible");

/// A callback to fill a coverage buffer.
using FillCoverageBufferCallback = void (*)(void* userData, F32* depthValues, U32 width, U32 height);

/// Ray tracing queue element.
class RayTracingInstanceQueueElement final
{
public:
	AccelerationStructure* m_bottomLevelAccelerationStructure;
	ModelGpuDescriptor m_modelDescriptor;
	Array<U32, U(RayType::COUNT)> m_shaderGroupHandleIndices;

	/// This points to the GR objects that are m_modelDescriptor is referencing. Use this to add a refcount to avoid
	/// accidential deletions.
	Array<GrObject*, 8> m_grObjects;
	U32 m_grObjectCount;
};

static_assert(std::is_trivially_destructible<RayTracingInstanceQueueElement>::value == true,
			  "Should be trivially destructible");

/// The render queue. This is what the renderer is fed to render.
class RenderQueue : public RenderingMatrices
{
public:
	WeakArray<RenderableQueueElement> m_renderables; ///< Deferred shading or shadow renderables.
	WeakArray<RenderableQueueElement> m_earlyZRenderables; ///< Some renderables that will be used for Early Z pass.
	WeakArray<RenderableQueueElement> m_forwardShadingRenderables;
	WeakArray<PointLightQueueElement> m_pointLights; ///< Those who cast shadows are first.
	WeakArray<SpotLightQueueElement> m_spotLights; ///< Those who cast shadows are first.
	DirectionalLightQueueElement m_directionalLight;
	WeakArray<ReflectionProbeQueueElement> m_reflectionProbes;
	WeakArray<GlobalIlluminationProbeQueueElement> m_giProbes;
	WeakArray<LensFlareQueueElement> m_lensFlares;
	WeakArray<DecalQueueElement> m_decals;
	WeakArray<FogDensityQueueElement> m_fogDensityVolumes;
	WeakArray<UiQueueElement> m_uis;
	WeakArray<GenericGpuComputeJobQueueElement> m_genericGpuComputeJobs;
	WeakArray<RayTracingInstanceQueueElement> m_rayTracingInstances;

	/// Contains the ray tracing elements. The rest of the members are unused. It's separate to avoid multithreading
	/// bugs.
	RenderQueue* m_rayTracingQueue = nullptr;

	/// Applies only if the RenderQueue holds shadow casters. It's the max timesamp of all shadow casters
	Timestamp m_shadowRenderablesLastUpdateTimestamp = 0;

	F32 m_cameraNear;
	F32 m_cameraFar;
	F32 m_cameraFovX;
	F32 m_cameraFovY;
	F32 m_effectiveShadowDistance;

	FillCoverageBufferCallback m_fillCoverageBufferCallback = nullptr;
	void* m_fillCoverageBufferCallbackUserData = nullptr;

	RenderQueue()
	{
		zeroMemory(m_directionalLight);
	}

	PtrSize countAllRenderables() const;
};

static_assert(std::is_trivially_destructible<RenderQueue>::value == true, "Should be trivially destructible");
/// @}

} // end namespace anki
