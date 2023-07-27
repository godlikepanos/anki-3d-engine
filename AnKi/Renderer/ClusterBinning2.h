// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Bins clusterer objects to the clusterer.
class ClusterBinning2 : public RendererObject
{
public:
	ClusterBinning2();

	~ClusterBinning2();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	ShaderProgramResourcePtr m_jobSetupProg;
	ShaderProgramPtr m_jobSetupGrProg;

	ShaderProgramResourcePtr m_binningProg;
	Array<ShaderProgramPtr, U32(GpuSceneNonRenderableObjectType::kCount)> m_binningGrProgs;

	ShaderProgramResourcePtr m_packingProg;
	Array<ShaderProgramPtr, U32(GpuSceneNonRenderableObjectType::kCount)> m_packingGrProgs;

	class
	{
	public:
		BufferHandle m_clustersHandle;
		BufferOffsetRange m_clustersBuffer;

		Array<BufferHandle, U32(GpuSceneNonRenderableObjectType::kCount)> m_packedObjectsHandles;
		Array<BufferOffsetRange, U32(GpuSceneNonRenderableObjectType::kCount)> m_packedObjectsBuffers;
	} m_runCtx;
};
/// @}

} // end namespace anki
