// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

	TextureView* m_hzbTexture = nullptr; ///< Optional.

	Sampler* m_sampler = nullptr;

	RenderingTechnique m_renderingTechinuqe = RenderingTechnique::kCount;

	class
	{
	public:
		BufferOffsetRange m_mdiDrawCountsBuffer;
		BufferOffsetRange m_drawIndexedIndirectArgsBuffer;
		BufferOffsetRange m_renderableInstancesBuffer;
	} m_legacy; ///< Legacy vertex flow

	class
	{
	public:
		BufferOffsetRange m_taskShaderIndirectArgsBuffer;
		BufferOffsetRange m_taskShaderPayloadsBuffer;
	} m_mesh;

	class
	{
	public:
		BufferOffsetRange m_meshletInstancesBuffer;
		BufferOffsetRange m_drawIndirectArgsBuffer;
	} m_softwareMesh;

	void fillMdi(const GpuVisibilityOutput& visOut)
	{
		m_legacy.m_mdiDrawCountsBuffer = visOut.m_legacy.m_mdiDrawCountsBuffer;
		m_legacy.m_drawIndexedIndirectArgsBuffer = visOut.m_legacy.m_drawIndexedIndirectArgsBuffer;
		m_legacy.m_renderableInstancesBuffer = visOut.m_legacy.m_renderableInstancesBuffer;
		m_mesh.m_taskShaderIndirectArgsBuffer = visOut.m_mesh.m_taskShaderIndirectArgsBuffer;
		m_mesh.m_taskShaderPayloadsBuffer = visOut.m_mesh.m_taskShaderPayloadBuffer;
	}

	void fill(const GpuMeshletVisibilityOutput& visOut)
	{
		m_softwareMesh.m_meshletInstancesBuffer = visOut.m_meshletInstancesBuffer;
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
