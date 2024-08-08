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
		BufferView m_dispatchMeshIndirectArgsBuffer;
		BufferView m_indirectDrawArgs;

		BufferView m_meshletInstancesBuffer;

		BufferView m_firstMeshletBuffer;
	} m_mesh;

	void fill(const GpuVisibilityOutput& visOut)
	{
		m_legacy.m_mdiDrawCountsBuffer = visOut.m_legacy.m_mdiDrawCountsBuffer;
		m_legacy.m_renderableInstancesBuffer = visOut.m_legacy.m_renderableInstancesBuffer;
		m_legacy.m_drawIndexedIndirectArgsBuffer = visOut.m_legacy.m_drawIndexedIndirectArgsBuffer;
		m_legacy.m_bucketRenderableInstanceRanges = visOut.m_legacy.m_bucketIndirectArgsRanges;
		m_mesh.m_dispatchMeshIndirectArgsBuffer = visOut.m_mesh.m_dispatchMeshIndirectArgsBuffer;
		m_mesh.m_indirectDrawArgs = visOut.m_mesh.m_drawIndirectArgs;
		m_mesh.m_meshletInstancesBuffer = visOut.m_mesh.m_meshletInstancesBuffer;
		m_mesh.m_firstMeshletBuffer = visOut.m_mesh.m_firstMeshletBuffer;
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
