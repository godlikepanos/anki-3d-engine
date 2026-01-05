// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

ANKI_CVAR2(NumericCVar<U32>, Render, Clusterer, ZSplitCount, 64, 8, kMaxZsplitCount, "Clusterer number of Z splits")
ANKI_CVAR2(NumericCVar<F32>, Render, Clusterer, Far, 512.0f, 32.0f, 10.0f * 1000.0f, "The extend of the clusterer in meters")

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

	// Returns the length of the cluster frustum. It's less or equal to camera far.
	F32 computeClustererFar() const
	{
		return min<F32>(getRenderingContext().m_matrices.m_far, g_cvarRenderClustererFar);
	}

	const UVec2& getTileCounts() const
	{
		return m_tileCounts;
	}

	void fillClustererConstants(ClustererConstants& consts) const;

private:
	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_jobSetupGrProg;
	Array<ShaderProgramPtr, U32(GpuSceneNonRenderableObjectType::kCount)> m_binningGrProgs;
	Array<ShaderProgramPtr, U32(GpuSceneNonRenderableObjectType::kCount)> m_packingGrProgs;

	UVec2 m_tileCounts = UVec2(0u);

	class
	{
	public:
		BufferHandle m_dep;

		BufferView m_clustersBuffer;
		Array<BufferView, U32(GpuSceneNonRenderableObjectType::kCount)> m_packedObjectsBuffers;
	} m_runCtx;
};

} // end namespace anki
