// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/components/SceneComponent.h>
#include <anki/resource/MaterialResource.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

enum class RenderComponentFlag : U8
{
	NONE = 0,
	CASTS_SHADOW = 1 << 0,
	FORWARD_SHADING = 1 << 1,
	SORT_LAST = 1 << 2, ///< Push it last when sorting the visibles.
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RenderComponentFlag)

/// Render component interface. Implemented by renderable scene nodes
class RenderComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::RENDER;

	RenderComponent()
		: SceneComponent(CLASS_TYPE)
	{
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
		RenderComponentFlag flags =
			(mtl->isForwardShading()) ? RenderComponentFlag::FORWARD_SHADING : RenderComponentFlag::NONE;
		flags |= (mtl->castsShadow()) ? RenderComponentFlag::CASTS_SHADOW : RenderComponentFlag::NONE;
		setFlags(flags);
	}

	void setupRaster(RenderQueueDrawCallback callback, const void* userData, U64 mergeKey)
	{
		ANKI_ASSERT(callback != nullptr);
		ANKI_ASSERT(userData != nullptr);
		ANKI_ASSERT(mergeKey != MAX_U64);
		m_callback = callback;
		m_userData = userData;
		m_mergeKey = mergeKey;
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const
	{
		ANKI_ASSERT(el.m_callback != nullptr);
		el.m_callback = m_callback;
		ANKI_ASSERT(el.m_userData != nullptr);
		el.m_userData = m_userData;
		ANKI_ASSERT(el.m_mergeKey != MAX_U64);
		el.m_mergeKey = m_mergeKey;
	}

	/// Helper function.
	static void allocateAndSetupUniforms(const MaterialResourcePtr& mtl, const RenderQueueDrawContext& ctx,
										 ConstWeakArray<Mat4> transforms, ConstWeakArray<Mat4> prevTransforms,
										 StagingGpuMemoryManager& alloc);

private:
	RenderQueueDrawCallback m_callback ANKI_DEBUG_CODE(= nullptr);
	const void* m_userData ANKI_DEBUG_CODE(= nullptr);
	U64 m_mergeKey ANKI_DEBUG_CODE(= MAX_U64);
	RenderComponentFlag m_flags = RenderComponentFlag::NONE;
};
/// @}

} // end namespace anki
