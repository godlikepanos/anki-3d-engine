// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Collision/ConvexHullShape.h>
#include <AnKi/Physics2/PhysicsWorld.h>

namespace anki {

Dbg::Dbg()
{
}

Dbg::~Dbg()
{
}

Error Dbg::init()
{
	// RT descr
	m_rtDescr = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
															  Format::kR8G8B8A8_Unorm, "Dbg");
	m_rtDescr.bake();

	ResourceManager& rsrcManager = ResourceManager::getSingleton();
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/GiProbe.ankitex", m_giProbeImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/LightBulb.ankitex", m_pointLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/SpotLight.ankitex", m_spotLightImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/GreenDecal.ankitex", m_decalImage));
	ANKI_CHECK(rsrcManager.loadResource("EngineAssets/Mirror.ankitex", m_reflectionImage));

	ANKI_CHECK(rsrcManager.loadResource("ShaderBinaries/Dbg.ankiprogbin", m_dbgProg));

	{
		BufferInitInfo buffInit("Dbg cube verts");
		buffInit.m_size = sizeof(Vec3) * 8;
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
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
		buffInit.m_usage = BufferUsageBit::kVertexOrIndex;
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
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
	variantInitInfo.addMutation("DEPTH_FAIL_VISUALIZATION", U32(m_ditheredDepthTestOn != 0));
	variantInitInfo.addMutation("OBJECT_TYPE", U32(type));
	variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "Bilboards");
	const ShaderProgramResourceVariant* variant;
	m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
	cmdb.bindShaderProgram(&variant->getProgram());

	class Constants
	{
	public:
		Mat4 m_viewProjMat;
		Mat3x4 m_camTrf;
	} consts;
	consts.m_viewProjMat = ctx.m_matrices.m_viewProjection;
	consts.m_camTrf = ctx.m_matrices.m_cameraTransform;
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindSrv(1, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(type));
	cmdb.bindSrv(2, 0, getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type));

	cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
	cmdb.bindSrv(3, 0, TextureView(&image.getTexture(), TextureSubresourceDesc::all()));
	cmdb.bindSrv(4, 0, TextureView(&m_spotLightImage->getTexture(), TextureSubresourceDesc::all()));

	cmdb.draw(PrimitiveTopology::kTriangles, 6, objCount);
}

void Dbg::run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Dbg);
	ANKI_ASSERT(g_dbgSceneCVar || g_dbgPhysicsCVar);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	// Set common state
	cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
	cmdb.setDepthWrite(false);

	cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
	cmdb.setDepthCompareOperation((m_depthTestOn) ? CompareOperation::kLess : CompareOperation::kAlways);
	cmdb.setLineWidth(2.0f);

	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
	rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());

	// GBuffer renderables
	if(g_dbgSceneCVar)
	{
		const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
		variantInitInfo.addMutation("DEPTH_FAIL_VISUALIZATION", U32(m_ditheredDepthTestOn != 0));
		variantInitInfo.addMutation("OBJECT_TYPE", 0);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "Renderables");
		const ShaderProgramResourceVariant* variant;
		m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
		cmdb.bindShaderProgram(&variant->getProgram());

		class Constants
		{
		public:
			Vec4 m_color;
			Mat4 m_viewProjMat;
		} consts;
		consts.m_color = Vec4(1.0f, 0.0f, 1.0f, 1.0f);
		consts.m_viewProjMat = ctx.m_matrices.m_viewProjection;

		cmdb.setFastConstants(&consts, sizeof(consts));
		cmdb.bindVertexBuffer(0, BufferView(m_cubeVertsBuffer.get()), sizeof(Vec3));
		cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
		cmdb.bindIndexBuffer(BufferView(m_cubeIndicesBuffer.get()), IndexType::kU16);

		cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView());

		BufferView indicesBuff;
		BufferHandle dep;
		getRenderer().getGBuffer().getVisibleAabbsBuffer(indicesBuff, dep);
		cmdb.bindSrv(2, 0, indicesBuff);

		cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, allAabbCount);
	}

	// Forward shading renderables
	if(g_dbgSceneCVar)
	{
		const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();

		if(allAabbCount)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView());

			BufferView indicesBuff;
			BufferHandle dep;
			getRenderer().getForwardShading().getVisibleAabbsBuffer(indicesBuff, dep);
			cmdb.bindSrv(2, 0, indicesBuff);

			cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, allAabbCount);
		}
	}

	// Draw non-renderables
	if(g_dbgSceneCVar)
	{
		drawNonRenderable(GpuSceneNonRenderableObjectType::kLight, GpuSceneArrays::Light::getSingleton().getElementCount(), ctx, *m_pointLightImage,
						  cmdb);
		drawNonRenderable(GpuSceneNonRenderableObjectType::kDecal, GpuSceneArrays::Decal::getSingleton().getElementCount(), ctx, *m_decalImage, cmdb);
		drawNonRenderable(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe,
						  GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount(), ctx, *m_giProbeImage, cmdb);
		drawNonRenderable(GpuSceneNonRenderableObjectType::kReflectionProbe, GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount(), ctx,
						  *m_reflectionImage, cmdb);
	}

	// Physics
	if(g_dbgPhysicsCVar)
	{
		class MyPhysicsDebugDrawerInterface final : public v2::PhysicsDebugDrawerInterface
		{
		public:
			RendererDynamicArray<HVec4> m_positions;
			RendererDynamicArray<Array<U8, 4>> m_colors;

			void drawLines(ConstWeakArray<Vec3> lines, Array<U8, 4> color) override
			{
				static constexpr U32 kMaxVerts = 1024 * 100;

				for(const Vec3& pos : lines)
				{
					if(m_positions.getSize() >= kMaxVerts)
					{
						break;
					}

					m_positions.emplaceBack(HVec4(pos.xyz0()));
					m_colors.emplaceBack(color);
				}
			}
		} drawerInterface;

		v2::PhysicsWorld::getSingleton().debugDraw(drawerInterface);

		const U32 vertCount = drawerInterface.m_positions.getSize();
		if(vertCount)
		{
			HVec4* positions;
			const BufferView positionBuff =
				RebarTransientMemoryPool::getSingleton().allocate(drawerInterface.m_positions.getSizeInBytes(), sizeof(HVec4), positions);
			memcpy(positions, drawerInterface.m_positions.getBegin(), drawerInterface.m_positions.getSizeInBytes());

			U8* colors;
			const BufferView colorBuff =
				RebarTransientMemoryPool::getSingleton().allocate(drawerInterface.m_colors.getSizeInBytes(), sizeof(U8) * 4, colors);
			memcpy(colors, drawerInterface.m_colors.getBegin(), drawerInterface.m_colors.getSizeInBytes());

			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("DEPTH_FAIL_VISUALIZATION", U32(m_ditheredDepthTestOn != 0));
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "Lines");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR16G16B16A16_Sfloat, 0);
			cmdb.setVertexAttribute(VertexAttributeSemantic::kColor, 1, Format::kR8G8B8A8_Unorm, 0);
			cmdb.bindVertexBuffer(0, positionBuff, sizeof(HVec4));
			cmdb.bindVertexBuffer(1, colorBuff, sizeof(U8) * 4);

			cmdb.setFastConstants(&ctx.m_matrices.m_viewProjection, sizeof(ctx.m_matrices.m_viewProjection));

			cmdb.draw(PrimitiveTopology::kLines, vertCount);
		}
	}

	// Restore state
	cmdb.setDepthCompareOperation(CompareOperation::kLess);
	cmdb.setDepthWrite(true);
}

void Dbg::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Dbg);

	if(!g_dbgSceneCVar && !g_dbgPhysicsCVar)
	{
		return;
	}

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Debug");

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(rgraphCtx, ctx);
	});

	GraphicsRenderPassTargetDesc colorRti(m_runCtx.m_rt);
	colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
	GraphicsRenderPassTargetDesc depthRti(getRenderer().getGBuffer().getDepthRt());
	depthRti.m_loadOperation = RenderTargetLoadOperation::kLoad;
	depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
	pass.setRenderpassInfo({colorRti}, &depthRti);

	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kRtvDsvWrite);
	pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvPixel | TextureUsageBit::kRtvDsvRead);

	if(g_dbgSceneCVar)
	{
		BufferView indicesBuff;
		BufferHandle dep;
		getRenderer().getGBuffer().getVisibleAabbsBuffer(indicesBuff, dep);
		pass.newBufferDependency(dep, BufferUsageBit::kSrvGeometry);

		if(GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount())
		{
			getRenderer().getForwardShading().getVisibleAabbsBuffer(indicesBuff, dep);
			pass.newBufferDependency(dep, BufferUsageBit::kSrvGeometry);
		}
	}
}

} // end namespace anki
