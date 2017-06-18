// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/core/Timestamp.h>
#include <anki/resource/RenderingKey.h>

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
};

class RenderQueueDrawContext final : public RenderingMatrices
{
public:
	RenderingKey m_key;
	CommandBufferPtr m_commandBuffer;
	StagingGpuMemoryManager* m_stagingGpuAllocator ANKI_DBG_NULLIFY;
};

/// Draw callback.
using RenderableQueueElementDrawCallback = void (*)(RenderQueueDrawContext& ctx, WeakArray<const void*> userData);

class RenderableQueueElement final
{
public:
	RenderableQueueElementDrawCallback m_callback;
	const void* m_userData;
	U64 m_mergeKey;
	F32 m_distanceFromCamera;
};

static_assert(
	std::is_trivially_destructible<RenderableQueueElement>::value == true, "Should be trivially destructible");

class PointLightQueueElement final
{
public:
	U64 m_uuid;
	Vec3 m_worldPosition;
	F32 m_radius;
	Vec3 m_diffuseColor;
	Vec3 m_specularColor;
	Array<RenderQueue*, 6> m_shadowRenderQueues;
	U32 m_textureArrayIndex; ///< Renderer internal.

	Bool hasShadow() const
	{
		return m_shadowRenderQueues[0] != nullptr;
	}
};

static_assert(
	std::is_trivially_destructible<PointLightQueueElement>::value == true, "Should be trivially destructible");

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
	Vec3 m_specularColor;
	RenderQueue* m_shadowRenderQueue;
	U32 m_textureArrayIndex; ///< Renderer internal.

	Bool hasShadow() const
	{
		return m_shadowRenderQueue != nullptr;
	}
};

static_assert(std::is_trivially_destructible<SpotLightQueueElement>::value == true, "Should be trivially destructible");

/// Normally the visibility tests don't perform tests on the reflection probes because probes dont's change that often.
/// This callback will be used by the renderer to inform a reflection probe that on the next frame it will be rendererd.
/// In that case the probe should fill the render queues.
using ReflectionProbeQueueElementFeedbackCallback = void (*)(Bool fillRenderQueuesOnNextFrame, void* userData);

class ReflectionProbeQueueElement final
{
public:
	ReflectionProbeQueueElementFeedbackCallback m_feedbackCallback;
	void* m_userData;
	U64 m_uuid;
	Vec3 m_worldPosition;
	F32 m_radius;
	Array<RenderQueue*, 6> m_renderQueues;
	U32 m_textureArrayIndex; ///< Renderer internal.
};

static_assert(
	std::is_trivially_destructible<ReflectionProbeQueueElement>::value == true, "Should be trivially destructible");

class LensFlareQueueElement final
{
public:
	Vec3 m_worldPosition;
	Vec2 m_firstFlareSize;
	Vec4 m_colorMultiplier;
	Texture* m_texture; ///< Totaly unsafe but we can't have a smart ptr in here since there will be no deletion.
};

static_assert(std::is_trivially_destructible<LensFlareQueueElement>::value == true, "Should be trivially destructible");

class DecalQueueElement final
{
public:
	Texture* m_diffuseAtlas;
	Texture* m_normalRoughnessAtlas;
	Vec4 m_diffuseAtlasUv;
	Vec4 m_normalRoughnessAtlasUv;
	F32 m_diffuseAtlasBlendFactor;
	F32 m_normalRoughnessAtlasBlendFactor;
	Mat4 m_textureMatrix;
	Vec3 m_obbCenter;
	Vec3 m_obbExtend;
	Mat3 m_obbRotation;
};

static_assert(std::is_trivially_destructible<DecalQueueElement>::value == true, "Should be trivially destructible");

/// The render queue.
class RenderQueue : public RenderingMatrices
{
public:
	WeakArray<RenderableQueueElement> m_renderables; ///< Deferred shading or shadow renderables.
	WeakArray<RenderableQueueElement> m_forwardShadingRenderables;
	WeakArray<PointLightQueueElement> m_pointLights;
	WeakArray<PointLightQueueElement*> m_shadowPointLights; ///< Points to elements in m_pointLights.
	WeakArray<SpotLightQueueElement> m_spotLights;
	WeakArray<SpotLightQueueElement*> m_shadowSpotLights; ///< Points to elements in m_spotLights.
	WeakArray<ReflectionProbeQueueElement> m_reflectionProbes;
	WeakArray<LensFlareQueueElement> m_lensFlares;
	WeakArray<DecalQueueElement> m_decals;

	/// Applies only if the RenderQueue holds shadow casters. It's the timesamp that modified
	Timestamp m_shadowRenderablesLastUpdateTimestamp = 0;

	F32 m_cameraNear;
	F32 m_cameraFar;
};
/// @}

} // end namespace anki
