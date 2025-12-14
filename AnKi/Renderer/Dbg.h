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

class DbgObjectPickingResult
{
public:
	U32 m_sceneNodeUuid = 0;
	U32 m_translationAxis = kMaxU32; // Can be 0, 1 or 2
	U32 m_scaleAxis = kMaxU32;
	U32 m_rotationAxis = kMaxU32;

	Bool operator==(const DbgObjectPickingResult&) const = default;

	Bool isValid() const
	{
		return *this != DbgObjectPickingResult();
	}
};

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

	const DbgObjectPickingResult& getObjectPickingResultAtMousePosition() const
	{
		ANKI_ASSERT(!!(m_options & DbgOption::kObjectPicking));
		return m_runCtx.m_objPickingRes;
	}

	void enableGizmos(const Transform& trf, Bool enableGizmos)
	{
		m_gizmos.m_trf = Mat3x4(trf.getOrigin().xyz, trf.getRotation().getRotationPart());
		m_gizmos.m_enabled = enableGizmos;
	}

	// Draw a debug solid cube in world space
	void enableDebugCube(Vec3 pos, F32 size, Vec4 color, Bool enable, Bool enableDepthTesting = true)
	{
		ANKI_ASSERT(size > kEpsilonf);
		m_debugPoint.m_position = (enable) ? pos : Vec3(kMaxF32);
		m_debugPoint.m_size = size;
		m_debugPoint.m_color = color;
		m_debugPoint.m_enableDepthTest = enableDepthTesting;
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

	ShaderProgramResourcePtr m_dbgProg;

	MultiframeReadbackToken m_readback;

	class
	{
	public:
		BufferPtr m_positionsBuff;
		BufferPtr m_indexBuff;
	} m_boxLines;

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

	class
	{
	public:
		Vec3 m_position = Vec3(kMaxF32);
		BufferPtr m_positionsBuff;
		F32 m_size = 1.0f;
		Vec4 m_color = Vec4(1.0f);
		Bool m_enableDepthTest = false;
	} m_debugPoint;

	DbgOption m_options = DbgOption::kDepthTest;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderTargetHandle m_objectPickingRt;

		DbgObjectPickingResult m_objPickingRes;
	} m_runCtx;

	void initGizmos();

	void populateRenderGraphMain(RenderingContext& ctx);

	void populateRenderGraphObjectPicking(RenderingContext& ctx);

	void drawNonRenderable(GpuSceneNonRenderableObjectType type, U32 objCount, const RenderingContext& ctx, const ImageResource& image,
						   CommandBuffer& cmdb);

	void drawGizmos(const Mat3x4& worldTransform, const RenderingContext& ctx, Bool objectPicking, CommandBuffer& cmdb) const;

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[DebugRenderTargetRegister::kUintTex] = m_runCtx.m_objectPickingRt;
		drawStyle = DebugRenderTargetDrawStyle::kIntegerTexture;
	}
};

} // end namespace anki
