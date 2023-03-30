// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Pack the visible clusterer objects.
class PackVisibleClusteredObjects : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	BufferHandle getClusteredObjectsRenderGraphHandle() const
	{
		return m_allClustererObjectsHandle;
	}

	void bindClusteredObjectBuffer(CommandBufferPtr& cmdb, U32 set, U32 binding, ClusteredObjectType type) const
	{
		cmdb->bindStorageBuffer(set, binding, m_allClustererObjects, m_structureBufferOffsets[type],
								kClusteredObjectSizes[type] * kMaxVisibleClusteredObjects[type]);
	}

private:
	BufferPtr m_allClustererObjects;
	BufferHandle m_allClustererObjectsHandle;

	ShaderProgramResourcePtr m_packProg;
	Array<ShaderProgramPtr, U32(ClusteredObjectType::kCount)> m_grProgs;

	Array<U32, U32(ClusteredObjectType::kCount)> m_structureBufferOffsets = {};

	U32 m_threadGroupSize = 0;

	template<typename TClustererType, ClusteredObjectType kType, typename TRenderQueueElement>
	void dispatchType(WeakArray<TRenderQueueElement> array, const RenderQueue& rqueue, CommandBufferPtr& cmdb);
};
/// @}

} // end namespace anki
