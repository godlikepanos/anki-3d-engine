// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/Readback.h>
#include <AnKi/Gr.h>
#include <AnKi/Util/Enum.h>

namespace anki {

enum class DbgOption : U8
{
	kNone = 0,

	// Flags that draw something somewhere
	kBoundingBoxes = 1 << 0,
	kIcons = 1 << 1,
	kPhysics = 1 << 2,
	kObjectPicking = 1 << 3,
	kSelectedObjectOutline = 1 << 4,

	// Flags that affect how things are drawn
	kDepthTest = 1 << 5,

	// Agregate flags
	kGatherAabbs = kBoundingBoxes | kObjectPicking,
	kDbgScene = kBoundingBoxes | kIcons | kPhysics | kSelectedObjectOutline,
	kDbgEnabled = kDbgScene | kObjectPicking | kSelectedObjectOutline,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(DbgOption)

// Debugging visualization of the scene.
class Dbg : public RendererObject
{
public:
	Dbg();

	~Dbg();

	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	void setOptions(DbgOption options)
	{
		m_options = options;
	}

	void enableOptions(DbgOption options)
	{
		m_options |= options;
	}

	DbgOption getOptions() const
	{
		return m_options;
	}

	Bool isEnabled() const
	{
		return !!(m_options & DbgOption::kDbgEnabled);
	}

	U32 getObjectUuidAtMousePosition() const
	{
		ANKI_ASSERT(!!(m_options & DbgOption::kObjectPicking));
		return m_runCtx.m_objUuid;
	}

	void setGizmosTransform(const Transform& trf, Bool enableGizmos)
	{
		m_gizmos.m_trf = Mat3x4(trf.getOrigin().xyz(), trf.getRotation().getRotationPart());
		m_gizmos.m_enabled = enableGizmos;
	}

private:
	RenderTargetDesc m_rtDescr;
	RenderTargetDesc m_objectPickingRtDescr;
	RenderTargetDesc m_objectPickingDepthRtDescr;

	ImageResourcePtr m_giProbeImage;
	ImageResourcePtr m_pointLightImage;
	ImageResourcePtr m_spotLightImage;
	ImageResourcePtr m_decalImage;
	ImageResourcePtr m_reflectionImage;

	BufferPtr m_cubeVertsBuffer;
	BufferPtr m_cubeIndicesBuffer;

	ShaderProgramResourcePtr m_dbgProg;

	MultiframeReadbackToken m_readback;

	class
	{
	public:
		BufferPtr m_arrowPositions;
		BufferPtr m_arrowIndices;
		BufferPtr m_scalePositions;
		BufferPtr m_scaleIndices;
		BufferPtr m_ringPositions;
		BufferPtr m_ringIndices;

		Mat3x4 m_trf;
		Bool m_enabled = false;
	} m_gizmos;

	DbgOption m_options = DbgOption::kDepthTest;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderTargetHandle m_objectPickingRt;
		U32 m_objUuid = 0;
	} m_runCtx;

	void initGizmos();

	void populateRenderGraphMain(RenderingContext& ctx);

	void populateRenderGraphObjectPicking(RenderingContext& ctx);

	void drawNonRenderable(GpuSceneNonRenderableObjectType type, U32 objCount, const RenderingContext& ctx, const ImageResource& image,
						   CommandBuffer& cmdb);

	void drawGizmos(const Mat3x4& worldTransform, const RenderingContext& ctx, CommandBuffer& cmdb) const;

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[DebugRenderTargetRegister::kUintTex] = m_runCtx.m_objectPickingRt;
		drawStyle = DebugRenderTargetDrawStyle::kIntegerTexture;
	}
};

} // end namespace anki
