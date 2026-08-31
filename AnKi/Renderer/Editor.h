// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Renderer/Utils/Readback.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

class EditorOptions
{
public:
	// Main pass:
	Bool m_lights : 1 = true;
	Bool m_decals : 1 = true;
	Bool m_particleEmitters : 1 = true;
	Bool m_cameras : 1 = true;
	Bool m_skyboxes : 1 = true;
	Bool m_reflectionProbes : 1 = true;
	Bool m_giProbes : 1 = true;
	Bool m_fogDensityVolumes : 1 = true;
	Bool m_scripts : 1 = true;
	Bool m_triggers : 1 = true;
	Bool m_physics : 1 = false;
	Bool m_indirectDiffuseProbes : 1 = false;
	Bool m_visibleRenderableBoundingVolumes : 1 = false;

	U8 m_indirectDiffuseProbesClipmap = 0;
	IndirectDiffuseClipmapsProbeType m_indirectDiffuseProbesClipmapType = IndirectDiffuseClipmapsProbeType::kIrradiance;
	F32 m_indirectDiffuseProbesClipmapColorScale = 1.0f;

	// Misc flags:
	Bool m_depthTest : 1 = true;
};

class EditorObjectPickingResult
{
public:
	U32 m_sceneNodeUuid = 0;

	// The axis the mouse is over. Can be 0, 1 or 2. kMaxU8 if the mouse is not over that gizmo
	U8 m_translationAxis = kMaxU8;
	U8 m_scaleAxis = kMaxU8;
	U8 m_rotationAxis = kMaxU8;

	Bool operator==(const EditorObjectPickingResult&) const = default;

	Bool isValid() const
	{
		return *this != EditorObjectPickingResult();
	}
};

// Does the debug drawing required by the editor and the object picking
class Editor : public RendererObject
{
public:
	Editor()
#if ANKI_WITH_EDITOR
		;
#else
	{
	}
#endif

	~Editor()
#if ANKI_WITH_EDITOR
		;
#else
	{
	}
#endif

	Error init()
#if ANKI_WITH_EDITOR
		;
#else
	{
		return Error::kNone;
	}
#endif

	void populateRenderGraph()
#if ANKI_WITH_EDITOR
		;
#else
	{
	}
#endif

	void enableGizmos(const Transform& trf, Bool enable)
	{
#if ANKI_WITH_EDITOR
		m_gizmos.m_trf = Mat3x4(trf.getOrigin().xyz, trf.getRotation().getRotationPart());
		m_gizmos.m_enabled = enable;
#endif
	}

	EditorOptions& getEditorOptions()
	{
#if ANKI_WITH_EDITOR
		return m_options;
#else
		ANKI_ASSERT(0);
		return *reinterpret_cast<EditorOptions*>(0);
#endif
	}

	const EditorOptions& getEditorOptions() const
	{
#if ANKI_WITH_EDITOR
		return m_options;
#else
		ANKI_ASSERT(0);
		return *reinterpret_cast<EditorOptions*>(0);
#endif
	}

	RenderTargetHandle getRt() const
	{
#if ANKI_WITH_EDITOR
		return m_runCtx.m_rt;
#else
		ANKI_ASSERT(0);
		return {};
#endif
	}

	const EditorObjectPickingResult& getObjectPickingResultAtMousePosition() const
	{
#if ANKI_WITH_EDITOR
		return m_runCtx.m_objPickingRes;
#else
		ANKI_ASSERT(0);
		return *reinterpret_cast<EditorObjectPickingResult*>(0);
#endif
	}

private:
#if ANKI_WITH_EDITOR
	RenderTargetDesc m_rtDescr;
	RenderTargetDesc m_objectPickingRtDescr;
	RenderTargetDesc m_objectPickingDepthRtDescr;

	ShaderProgramResourcePtr m_prog;

	ImageResourcePtr m_giProbeImage;
	ImageResourcePtr m_pointLightImage;
	ImageResourcePtr m_spotLightImage;
	ImageResourcePtr m_decalImage;
	ImageResourcePtr m_reflectionImage;
	ImageResourcePtr m_particlesImage;
	ImageResourcePtr m_cloudImage;
	ImageResourcePtr m_sunImage;
	ImageResourcePtr m_cameraImage;
	ImageResourcePtr m_skyboxImage;
	ImageResourcePtr m_scriptImage;
	ImageResourcePtr m_triggerImage;

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
		BufferPtr m_positionsBuff;
		BufferPtr m_indexBuff;
	} m_boxLines;

	MultiframeReadbackToken m_readback;

	EditorOptions m_options;

	class
	{
	public:
		RenderTargetHandle m_rt;
		RenderTargetHandle m_objectPickingRt;

		EditorObjectPickingResult m_objPickingRes;
	} m_runCtx;

	void initGizmos();
	void initBoxLines();

	// Draw the gizmos of the selected node. Doesn't set any pipeline state, the caller is responsible for that.
	void drawGizmos(Bool objectPicking, CommandBuffer& cmdb) const;

	// Draw a single camera facing quad that visualizes an object that has no geometry of its own.
	void drawBillboard(Vec3 worldPosition, Vec3 colorScale, const ImageResource& image, U32 sceneNodeUuid, Bool objectPicking,
					   RenderPassWorkContext& rgraphCtx) const;

	void drawSceneComponentIcons(Bool objectPicking, RenderPassWorkContext& rgraphCtx) const;

	void drawPhysics(CommandBuffer& cmdb) const;

	void drawRenderableBoxes(RenderPassWorkContext& rgraphCtx) const;

	void populateRenderGraphMain();

	void populateRenderObjectPicking();

	void getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
							  DebugRenderTargetDrawStyle& drawStyle) const override
	{
		handles[DebugRenderTargetRegister::kUintTex] = m_runCtx.m_objectPickingRt;
		drawStyle = DebugRenderTargetDrawStyle::kIntegerTexture;
	}
#endif // ANKI_WITH_EDITOR
};

} // end namespace anki
