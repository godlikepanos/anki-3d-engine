// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Scene.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Collision/ConvexHullShape.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

BoolCVar g_dbgCVar(CVarSubsystem::kRenderer, "Dbg", false, "Enable or not debug visualization");
static BoolCVar g_dbgPhysicsCVar(CVarSubsystem::kRenderer, "DbgPhysics", false, "Enable or not physics debug visualization");

Dbg::Dbg()
{
}

Dbg::~Dbg()
{
}

Error Dbg::init()
{
	ANKI_R_LOGV("Initializing DBG");

	// RT descr
	m_rtDescr = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
															  Format::kR8G8B8A8_Unorm, "Dbg");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kClear;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::kDontCare;
	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_fbDescr.bake();

	ResourceManager& rsrcManager = ResourceManager::getSingleton();
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/GiProbe.ankitex", m_giProbeImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/LightBulb.ankitex", m_pointLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/SpotLight.ankitex", m_spotLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/GreenDecal.ankitex", m_decalImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Mirror.ankitex", m_reflectionImage));

	ANKI_CHECK(rsrcManager.loadResource("ShaderBinaries/DbgRenderables.ankiprogbin", m_renderablesProg));
	ANKI_CHECK(rsrcManager.loadResource("ShaderBinaries/DbgBillboard.ankiprogbin", m_nonRenderablesProg));

	{
		BufferInitInfo buffInit("Dbg cube verts");
		buffInit.m_size = sizeof(Vec3) * 8;
		buffInit.m_usage = BufferUsageBit::kVertex;
		buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
		m_cubeVertsBuffer = GrManager::getSingleton().newBuffer(buffInit);

		Vec3* verts = static_cast<Vec3*>(m_cubeVertsBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

		constexpr F32 kSize = 1.0f;
		verts[0] = Vec3(kSize, kSize, kSize); // front top right
		verts[1] = Vec3(-kSize, kSize, kSize); // front top left
		verts[2] = Vec3(-kSize, -kSize, kSize); // front bottom left
		verts[3] = Vec3(kSize, -kSize, kSize); // front bottom right
		verts[4] = Vec3(kSize, kSize, -kSize); // back top right
		verts[5] = Vec3(-kSize, kSize, -kSize); // back top left
		verts[6] = Vec3(-kSize, -kSize, -kSize); // back bottom left
		verts[7] = Vec3(kSize, -kSize, -kSize); // back bottom right

		m_cubeVertsBuffer->unmap();

		constexpr U kIndexCount = 12 * 2;
		buffInit.setName("Dbg cube indices");
		buffInit.m_usage = BufferUsageBit::kIndex;
		buffInit.m_size = kIndexCount * sizeof(U16);
		m_cubeIndicesBuffer = GrManager::getSingleton().newBuffer(buffInit);
		U16* indices = static_cast<U16*>(m_cubeIndicesBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

		U c = 0;
		indices[c++] = 0;
		indices[c++] = 1;
		indices[c++] = 1;
		indices[c++] = 2;
		indices[c++] = 2;
		indices[c++] = 3;
		indices[c++] = 3;
		indices[c++] = 0;

		indices[c++] = 4;
		indices[c++] = 5;
		indices[c++] = 5;
		indices[c++] = 6;
		indices[c++] = 6;
		indices[c++] = 7;
		indices[c++] = 7;
		indices[c++] = 4;

		indices[c++] = 0;
		indices[c++] = 4;
		indices[c++] = 1;
		indices[c++] = 5;
		indices[c++] = 2;
		indices[c++] = 6;
		indices[c++] = 3;
		indices[c++] = 7;

		m_cubeIndicesBuffer->unmap();

		ANKI_ASSERT(c == kIndexCount);
	}

	return Error::kNone;
}

void Dbg::drawNonRenderable(GpuSceneNonRenderableObjectType type, U32 objCount, const RenderingContext& ctx, const ImageResource& image,
							CommandBuffer& cmdb)
{
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_nonRenderablesProg);
	variantInitInfo.addMutation("DEPTH_FAIL_VISUALIZATION", U32(m_ditheredDepthTestOn != 0));
	variantInitInfo.addMutation("OBJECT_TYPE", U32(type));
	const ShaderProgramResourceVariant* variant;
	m_nonRenderablesProg->getOrCreateVariant(variantInitInfo, variant);
	cmdb.bindShaderProgram(&variant->getProgram());

	class Constants
	{
	public:
		Mat4 m_viewProjMat;
		Mat3x4 m_camTrf;
	} unis;
	unis.m_viewProjMat = ctx.m_matrices.m_viewProjection;
	unis.m_camTrf = ctx.m_matrices.m_cameraTransform;
	cmdb.setPushConstants(&unis, sizeof(unis));

	cmdb.bindUavBuffer(0, 2, getRenderer().getClusterBinning().getPackedObjectsBuffer(type));
	cmdb.bindUavBuffer(0, 3, getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type));

	cmdb.bindSampler(0, 4, getRenderer().getSamplers().m_trilinearRepeat.get());
	cmdb.bindTexture(0, 5, &image.getTextureView());
	cmdb.bindTexture(0, 6, &m_spotLightImage->getTextureView());

	cmdb.draw(PrimitiveTopology::kTriangles, 6, objCount);
}

void Dbg::run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Dbg);
	ANKI_ASSERT(g_dbgCVar.get());

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	// Set common state
	cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
	cmdb.setDepthWrite(false);

	cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
	cmdb.setDepthCompareOperation((m_depthTestOn) ? CompareOperation::kLess : CompareOperation::kAlways);
	cmdb.setLineWidth(2.0f);

	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
	rgraphCtx.bindTexture(0, 1, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

	// GBuffer renderables
	{
		const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_renderablesProg);
		variantInitInfo.addMutation("DEPTH_FAIL_VISUALIZATION", U32(m_ditheredDepthTestOn != 0));
		const ShaderProgramResourceVariant* variant;
		m_renderablesProg->getOrCreateVariant(variantInitInfo, variant);
		cmdb.bindShaderProgram(&variant->getProgram());

		class Constants
		{
		public:
			Vec4 m_color;
			Mat4 m_viewProjMat;
		} unis;
		unis.m_color = Vec4(1.0f, 0.0f, 1.0f, 1.0f);
		unis.m_viewProjMat = ctx.m_matrices.m_viewProjection;

		cmdb.setPushConstants(&unis, sizeof(unis));
		cmdb.bindVertexBuffer(0, m_cubeVertsBuffer.get(), 0, sizeof(Vec3));
		cmdb.setVertexAttribute(0, 0, Format::kR32G32B32_Sfloat, 0);
		cmdb.bindIndexBuffer(m_cubeIndicesBuffer.get(), 0, IndexType::kU16);

		cmdb.bindUavBuffer(0, 2, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferOffsetRange());
		cmdb.bindUavBuffer(0, 3, getRenderer().getGBuffer().getVisibleAabbsBuffer());

		cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, allAabbCount);
	}

	// Forward shading renderables
	{
		const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();

		cmdb.bindUavBuffer(0, 2, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferOffsetRange());
		cmdb.bindUavBuffer(0, 3, getRenderer().getForwardShading().getVisibleAabbsBuffer());

		cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, allAabbCount);
	}

	// Draw non-renderables
	drawNonRenderable(GpuSceneNonRenderableObjectType::kLight, GpuSceneArrays::Light::getSingleton().getElementCount(), ctx, *m_pointLightImage,
					  cmdb);
	drawNonRenderable(GpuSceneNonRenderableObjectType::kDecal, GpuSceneArrays::Decal::getSingleton().getElementCount(), ctx, *m_decalImage, cmdb);
	drawNonRenderable(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe,
					  GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount(), ctx, *m_giProbeImage, cmdb);
	drawNonRenderable(GpuSceneNonRenderableObjectType::kReflectionProbe, GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount(), ctx,
					  *m_reflectionImage, cmdb);

	// Restore state
	cmdb.setDepthCompareOperation(CompareOperation::kLess);
}

void Dbg::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Dbg);

	if(!g_dbgCVar.get())
	{
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Debug");

	pass.setWork(1, [this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(rgraphCtx, ctx);
	});

	pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rt}, getRenderer().getGBuffer().getDepthRt());

	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
	pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSampledFragment | TextureUsageBit::kFramebufferRead);
}

} // end namespace anki
