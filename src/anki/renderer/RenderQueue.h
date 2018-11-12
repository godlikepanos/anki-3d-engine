// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/resource/RenderingKey.h>
#include <anki/ui/Canvas.h>

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
	StagingGpuMemoryManager* m_stagingGpuAllocator ANKI_DBG_NULLIFY;
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
	U64 m_mergeKey;
	F32 m_distanceFromCamera; ///< Don't set this

	RenderableQueueElement()
	{
	}
};

static_assert(
	std::is_trivially_destructible<RenderableQueueElement>::value == true, "Should be trivially destructible");

/// Point light render queue element.
class PointLightQueueElement final
{
public:
	U64 m_uuid;
	Vec3 m_worldPosition;
	F32 m_radius;
	Vec3 m_diffuseColor;
	Array<RenderQueue*, 6> m_shadowRenderQueues;
	const void* m_userData;
	RenderQueueDrawCallback m_drawCallback;

	UVec2 m_atlasTiles; ///< Renderer internal.
	F32 m_atlasTileSize; ///< Renderer internal.

	PointLightQueueElement()
	{
	}

	Bool hasShadow() const
	{
		return m_shadowRenderQueues[0] != nullptr;
	}
};

static_assert(
	std::is_trivially_destructible<PointLightQueueElement>::value == true, "Should be trivially destructible");

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
	const void* m_userData;
	RenderQueueDrawCallback m_drawCallback;

	SpotLightQueueElement()
	{
	}

	Bool hasShadow() const
	{
		return m_shadowRenderQueue != nullptr;
	}
};

static_assert(std::is_trivially_destructible<SpotLightQueueElement>::value == true, "Should be trivially destructible");

/// Normally the visibility tests don't perform tests on the reflection probes because probes dont change that often.
/// This callback will be used by the renderer to inform a reflection probe that on the next frame it will be rendererd.
/// In that case the probe should fill the render queues.
using ReflectionProbeQueueElementFeedbackCallback = void (*)(Bool fillRenderQueuesOnNextFrame, void* userData);

/// Reflection probe render queue element.
class ReflectionProbeQueueElement final
{
public:
	ReflectionProbeQueueElementFeedbackCallback m_feedbackCallback;
	RenderQueueDrawCallback m_drawCallback;
	void* m_userData;
	U64 m_uuid;
	Vec3 m_worldPosition;
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
	Array<RenderQueue*, 6> m_renderQueues;
	U32 m_textureArrayIndex; ///< Renderer internal.

	ReflectionProbeQueueElement()
	{
	}
};

static_assert(
	std::is_trivially_destructible<ReflectionProbeQueueElement>::value == true, "Should be trivially destructible");

/// Lens flare render queue element.
class LensFlareQueueElement final
{
public:
	Vec3 m_worldPosition;
	Vec2 m_firstFlareSize;
	Vec4 m_colorMultiplier;
	/// Totaly unsafe but we can't have a smart ptr in here since there will be no deletion.
	const TextureView* m_textureView;
	const void* m_userData;
	RenderQueueDrawCallback m_drawCallback;

	LensFlareQueueElement()
	{
	}
};

static_assert(std::is_trivially_destructible<LensFlareQueueElement>::value == true, "Should be trivially destructible");

/// Decal render queue element.
class DecalQueueElement final
{
public:
	const void* m_userData;
	RenderQueueDrawCallback m_drawCallback;
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
	Bool8 m_isBox;

	FogDensityQueueElement()
	{
	}
};

static_assert(
	std::is_trivially_destructible<FogDensityQueueElement>::value == true, "Should be trivially destructible");

/// A callback to fill a coverage buffer.
using FillCoverageBufferCallback = void (*)(void* userData, F32* depthValues, U32 width, U32 height);

/// The render queue. This is what the renderer is fed to render.
class RenderQueue : public RenderingMatrices
{
public:
	WeakArray<RenderableQueueElement> m_renderables; ///< Deferred shading or shadow renderables.
	WeakArray<RenderableQueueElement> m_earlyZRenderables; ///< Some renderables that will be used for Early Z pass.
	WeakArray<RenderableQueueElement> m_forwardShadingRenderables;
	WeakArray<PointLightQueueElement> m_pointLights;
	WeakArray<PointLightQueueElement*> m_shadowPointLights; ///< Points to elements in m_pointLights.
	WeakArray<SpotLightQueueElement> m_spotLights;
	WeakArray<SpotLightQueueElement*> m_shadowSpotLights; ///< Points to elements in m_spotLights.
	WeakArray<ReflectionProbeQueueElement> m_reflectionProbes;
	WeakArray<LensFlareQueueElement> m_lensFlares;
	WeakArray<DecalQueueElement> m_decals;
	WeakArray<FogDensityQueueElement> m_fogDensityVolumes;
	WeakArray<UiQueueElement> m_uis;

	/// Applies only if the RenderQueue holds shadow casters. It's the max timesamp of all shadow casters
	Timestamp m_shadowRenderablesLastUpdateTimestamp = 0;

	F32 m_cameraNear;
	F32 m_cameraFar;

	FillCoverageBufferCallback m_fillCoverageBufferCallback = nullptr;
	void* m_fillCoverageBufferCallbackUserData = nullptr;

	U countAllRenderables() const;
};

static_assert(std::is_trivially_destructible<RenderQueue>::value == true, "Should be trivially destructible");
/// @}

} // end namespace anki
