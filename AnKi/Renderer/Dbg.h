// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/Readback.h>
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Gr.h>
#include <AnKi/Util/Enum.h>

namespace anki {

class DbgOptions
{
public:
	// Main pass:
	Bool m_renderableBoundingBoxes : 1 = false;
	Bool m_sceneGraphIcons : 1 = false;
	Bool m_physics : 1 = false;
	Bool m_indirectDiffuseProbes : 1 = false;

	U8 m_indirectDiffuseProbesClipmap = 0;
	IndirectDiffuseClipmapsProbeType m_indirectDiffuseProbesClipmapType = IndirectDiffuseClipmapsProbeType::kIrradiance;
	F32 m_indirectDiffuseProbesClipmapColorScale = 1.0f;

	// Object picking:
	Bool m_objectPicking : 1 = false;

	// Misc flags:
	Bool m_depthTest : 1 = true;

	Bool visibilityShouldGatherAabbs() const
	{
		return m_renderableBoundingBoxes || m_objectPicking;
	}

	Bool mainDbgPass() const
	{
		return m_renderableBoundingBoxes || m_sceneGraphIcons || m_physics || m_indirectDiffuseProbes;
	}

	Bool dgbEnabled() const
	{
		return mainDbgPass() || m_objectPicking;
	}
};

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

	void populateRenderGraph();

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	const DbgOptions& getOptions() const
	{
		return m_options;
	}

	DbgOptions& getOptions()
	{
		return m_options;
	}

	Bool isEnabled() const
	{
		return m_options.dgbEnabled();
	}

	const DbgObjectPickingResult& getObjectPickingResultAtMousePosition() const
	{
		ANKI_ASSERT(m_options.m_objectPicking);
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
	ImageResourcePtr m_particlesImage;
	ImageResourcePtr m_cloudImage;
	ImageResourcePtr m_sunImage;

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

	DbgOptions m_options;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderTargetHandle m_objectPickingRt;

		DbgObjectPickingResult m_objPickingRes;
	} m_runCtx;

	class InternalCtx;

	void initGizmos();

	void populateRenderGraphParticleEmitters(InternalCtx& ictx);
	void populateRenderGraphMain(InternalCtx& ictx);
	void populateRenderGraphObjectPicking(InternalCtx& ictx);

	void drawNonRenderable(GpuSceneNonRenderableObjectType type, U32 objCount, const ImageResource& image, Bool objectPicking,
						   RenderPassWorkContext& rgraphCtx);

	void drawParticleEmitters(const InternalCtx& ictx, Bool objectPicking, RenderPassWorkContext& rgraphCtx);

	void drawNonGpuSceneObjects(Bool objectPicking, RenderPassWorkContext& rgraphCtx);

	void drawIcons(const InternalCtx& ictx, Bool objectPicking, RenderPassWorkContext& rgraphCtx);

	void drawGizmos(const Mat3x4& worldTransform, Bool objectPicking, CommandBuffer& cmdb) const;

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[DebugRenderTargetRegister::kUintTex] = m_runCtx.m_objectPickingRt;
		drawStyle = DebugRenderTargetDrawStyle::kIntegerTexture;
	}
};

} // end namespace anki
