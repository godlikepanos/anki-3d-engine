// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

// Bins clusterer objects to the clusterer.
class ClusterBinning : public RendererObject
{
public:
	ClusterBinning();

	~ClusterBinning();

	Error init();

	void populateRenderGraph();

	const BufferView& getPackedObjectsBuffer(GpuSceneNonRenderableObjectType type) const
	{
		return m_runCtx.m_packedObjectsBuffers[type];
	}

	const BufferView& getClustersBuffer() const
	{
		return m_runCtx.m_clustersBuffer;
	}

	BufferHandle getDependency() const
	{
		return m_runCtx.m_dep;
	}

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_jobSetupGrProg;
	Array<ShaderProgramPtr, U32(GpuSceneNonRenderableObjectType::kCount)> m_binningGrProgs;
	Array<ShaderProgramPtr, U32(GpuSceneNonRenderableObjectType::kCount)> m_packingGrProgs;

	class
	{
	public:
		BufferHandle m_dep;

		BufferView m_clustersBuffer;
		Array<BufferView, U32(GpuSceneNonRenderableObjectType::kCount)> m_packedObjectsBuffers;
	} m_runCtx;
};

} // end namespace anki
