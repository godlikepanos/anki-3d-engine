// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// @memberof RenderableDrawer.
class RenderableDrawerArguments
{
public:
	// The matrices are whatever the drawing needs. Sometimes they contain jittering and sometimes they don't.
	Mat3x4 m_viewMatrix;
	Mat3x4 m_cameraTransform;
	Mat4 m_viewProjectionMatrix;
	Mat4 m_previousViewProjectionMatrix;

	UVec4 m_viewport;

	TextureView m_hzbTexture; ///< Optional.

	Sampler* m_sampler = nullptr;

	RenderingTechnique m_renderingTechinuqe = RenderingTechnique::kCount;

	class
	{
	public:
		BufferView m_mdiDrawCountsBuffer;
		BufferView m_renderableInstancesBuffer;
		BufferView m_drawIndexedIndirectArgsBuffer;

		ConstWeakArray<InstanceRange> m_bucketRenderableInstanceRanges;
	} m_legacy; ///< Legacy vertex flow

	class
	{
	public:
		BufferView m_taskShaderIndirectArgsBuffer;
		BufferView m_meshletGroupInstancesBuffer;

		ConstWeakArray<InstanceRange> m_bucketMeshletGroupInstanceRanges;
	} m_mesh;

	class
	{
	public:
		BufferView m_meshletInstancesBuffer;
		BufferView m_drawIndirectArgsBuffer;

		ConstWeakArray<InstanceRange> m_bucketMeshletInstanceRanges;
	} m_softwareMesh;

	void fill(const GpuVisibilityOutput& visOut)
	{
		m_legacy.m_mdiDrawCountsBuffer = visOut.m_legacy.m_mdiDrawCountsBuffer;
		m_legacy.m_renderableInstancesBuffer = visOut.m_legacy.m_renderableInstancesBuffer;
		m_legacy.m_drawIndexedIndirectArgsBuffer = visOut.m_legacy.m_drawIndexedIndirectArgsBuffer;
		m_legacy.m_bucketRenderableInstanceRanges = visOut.m_legacy.m_bucketRenderableInstanceRanges;
		m_mesh.m_taskShaderIndirectArgsBuffer = visOut.m_mesh.m_taskShaderIndirectArgsBuffer;
		m_mesh.m_meshletGroupInstancesBuffer = visOut.m_mesh.m_meshletGroupInstancesBuffer;
		m_mesh.m_bucketMeshletGroupInstanceRanges = visOut.m_mesh.m_bucketMeshletGroupInstanceRanges;
	}

	void fill(const GpuMeshletVisibilityOutput& visOut)
	{
		ANKI_ASSERT(visOut.isFilled());
		m_softwareMesh.m_meshletInstancesBuffer = visOut.m_meshletInstancesBuffer;
		m_softwareMesh.m_bucketMeshletInstanceRanges = visOut.m_bucketMeshletInstanceRanges;
		m_softwareMesh.m_drawIndirectArgsBuffer = visOut.m_drawIndirectArgsBuffer;
	}
};

/// It uses visibility data to issue drawcalls.
class RenderableDrawer : public RendererObject
{
public:
	RenderableDrawer() = default;

	~RenderableDrawer();

	Error init();

	/// Draw using multidraw indirect.
	/// @note It's thread-safe.
	void drawMdi(const RenderableDrawerArguments& args, CommandBuffer& cmdb);

private:
	void setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb);
};
/// @}

} // end namespace anki
