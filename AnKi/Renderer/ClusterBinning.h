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
class ClusterBinning : public RendererObject
{
public:
	ClusterBinning();

	~ClusterBinning();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	const BufferOffsetRange& getClusteredShadingConstants() const
	{
		return m_runCtx.m_clusterConstBuffer;
	}

	const BufferOffsetRange& getPackedObjectsBuffer(GpuSceneNonRenderableObjectType type) const
	{
		return m_runCtx.m_packedObjectsBuffers[type];
	}

	BufferHandle getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType type) const
	{
		return m_runCtx.m_packedObjectsHandles[type];
	}

	const BufferOffsetRange& getClustersBuffer() const
	{
		return m_runCtx.m_clustersBuffer;
	}

	BufferHandle getClustersBufferHandle() const
	{
		return m_runCtx.m_clustersHandle;
	}

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

		BufferOffsetRange m_clusterConstBuffer;
		ClusteredShadingConstants* m_constsCpu = nullptr;
		RenderingContext* m_rctx = nullptr;
	} m_runCtx;

	void writeClusterConstsInternal();
};
/// @}

} // end namespace anki
