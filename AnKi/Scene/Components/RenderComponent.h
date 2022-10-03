// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

/// @addtogroup scene
/// @{

enum class RenderComponentFlag : U8
{
	kNone = 0,
	kCastsShadow = 1 << 0,
	kForwardShading = 1 << 1,
	kSortLast = 1 << 2, ///< Push it last when sorting the visibles.
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RenderComponentFlag)

using FillRayTracingInstanceQueueElementCallback = void (*)(U32 lod, const void* userData,
															RayTracingInstanceQueueElement& el);

/// Render component interface. Implemented by renderable scene nodes
class RenderComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(RenderComponent)

public:
	RenderComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId())
	{
	}

	Bool isEnabled() const
	{
		return m_callback != nullptr;
	}

	RenderComponentFlag getFlags() const
	{
		return m_flags;
	}

	void setFlags(RenderComponentFlag flags)
	{
		m_flags = flags;
	}

	void setFlagsFromMaterial(const MaterialResourcePtr& mtl)
	{
		RenderComponentFlag flags = !!(mtl->getRenderingTechniques() & RenderingTechniqueBit::kForward)
										? RenderComponentFlag::kForwardShading
										: RenderComponentFlag::kNone;
		flags |= (mtl->castsShadow()) ? RenderComponentFlag::kCastsShadow : RenderComponentFlag::kNone;
		setFlags(flags);
	}

	void initRaster(RenderQueueDrawCallback callback, const void* userData, U64 mergeKey)
	{
		ANKI_ASSERT(callback != nullptr);
		ANKI_ASSERT(userData != nullptr);
		ANKI_ASSERT(mergeKey != kMaxU64);
		m_callback = callback;
		m_userData = userData;
		m_mergeKey = mergeKey;
	}

	void initRayTracing(FillRayTracingInstanceQueueElementCallback callback, const void* userData)
	{
		m_rtCallback = callback;
		m_rtCallbackUserData = userData;
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const
	{
		ANKI_ASSERT(m_callback != nullptr);
		el.m_callback = m_callback;
		ANKI_ASSERT(m_userData != nullptr);
		el.m_userData = m_userData;
		ANKI_ASSERT(m_mergeKey != kMaxU64);
		el.m_mergeKey = m_mergeKey;
		el.m_distanceFromCamera = -1.0f;
		el.m_lod = kMaxU8;
	}

	void setupRayTracingInstanceQueueElement(U32 lod, RayTracingInstanceQueueElement& el) const
	{
		ANKI_ASSERT(m_rtCallback);
		m_rtCallback(lod, m_rtCallbackUserData, el);
	}

	Bool getSupportsRayTracing() const
	{
		return m_rtCallback != nullptr;
	}

	/// Helper function.
	static void allocateAndSetupUniforms(const MaterialResourcePtr& mtl, const RenderQueueDrawContext& ctx,
										 ConstWeakArray<Mat3x4> transforms, ConstWeakArray<Mat3x4> prevTransforms,
										 StagingGpuMemoryPool& alloc);

private:
	RenderQueueDrawCallback m_callback = nullptr;
	const void* m_userData = nullptr;
	U64 m_mergeKey = kMaxU64;
	FillRayTracingInstanceQueueElementCallback m_rtCallback = nullptr;
	const void* m_rtCallbackUserData = nullptr;
	RenderComponentFlag m_flags = RenderComponentFlag::kNone;
};
/// @}

} // end namespace anki
