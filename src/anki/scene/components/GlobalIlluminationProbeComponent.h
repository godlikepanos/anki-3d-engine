// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Reflection probe component.
class GlobalIlluminationProbeComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::GLOBAL_ILLUMINATION_PROBE;

	GlobalIlluminationProbeComponent(U64 uuid)
		: SceneComponent(CLASS_TYPE)
		, m_uuid(uuid)
	{
	}

	void setBoundingBox(const Vec4& min, const Vec4& max)
	{
		m_aabbMin = min.xyz();
		m_aabbMax = max.xyz();
	}

	Vec4 getAlignedBoundingBoxMin() const
	{
		return m_aabbMin.xyz0();
	}

	Vec4 getAlignedBoundingBoxMax() const
	{
		return m_aabbMax.xyz0();
	}

	/// Returns true if it's marked for update this frame.
	Bool getMarkedForRendering() const
	{
		return m_markedForRendering;
	}

	/// Get the cell position that will be rendered this frame.
	Vec4 getRenderPosition() const
	{
		ANKI_ASSERT(m_renderPosition > m_aabbMin && m_renderPosition < m_aabbMax);
		ANKI_ASSERT(m_markedForRendering);
		return m_renderPosition.xyz0();
	}

	void setupGlobalIlluminationProbeQueueElement(GlobalIlluminationProbeQueueElement& el) const
	{
		ANKI_ASSERT(m_aabbMin < m_aabbMax);
		// TODO
	}

private:
	U64 m_uuid;
	Vec3 m_aabbMin = Vec3(+1.0f);
	Vec3 m_aabbMax = Vec3(-1.0f);
	Vec3 m_renderPosition = Vec3(0.0f);
	F32 m_cellSize = 1.0f;
	Bool m_markedForRendering = false;

	static void reflectionProbeQueueElementFeedbackCallback(Bool fillRenderQueuesOnNextFrame, void* userData)
	{
		ANKI_ASSERT(userData);
		static_cast<GlobalIlluminationProbeComponent*>(userData)->m_markedForRendering = fillRenderQueuesOnNextFrame;
	}

	static void debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}
};
/// @}

} // end namespace anki
