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
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Shaders/Include/GpuVisibilityTypes.h>
#include <AnKi/Window/Input.h>

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

	m_objectPickingRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR32_Uint, "ObjectPicking");
	m_objectPickingRtDescr.bake();

	m_objectPickingDepthRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kD32_Sfloat, "ObjectPickingDepth");
	m_objectPickingDepthRtDescr.bake();

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

		UVec3 m_padding;
		U32 m_depthFailureVisualization;
	} consts;
	consts.m_viewProjMat = ctx.m_matrices.m_viewProjection;
	consts.m_camTrf = ctx.m_matrices.m_cameraTransform;
	consts.m_depthFailureVisualization = !(m_options & DbgOption::kDepthTest);
	cmdb.setFastConstants(&consts, sizeof(consts));

	cmdb.bindSrv(1, 0, getClusterBinning().getPackedObjectsBuffer(type));
	cmdb.bindSrv(2, 0, getRenderer().getPrimaryNonRenderableVisibility().getVisibleIndicesBuffer(type));

	cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
	cmdb.bindSrv(3, 0, TextureView(&image.getTexture(), TextureSubresourceDesc::all()));
	cmdb.bindSrv(4, 0, TextureView(&m_spotLightImage->getTexture(), TextureSubresourceDesc::all()));

	cmdb.draw(PrimitiveTopology::kTriangles, 6, objCount);
}

void Dbg::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Dbg);

	if(!isEnabled())
	{
		return;
	}

	// Debug visualization
	if(!!(m_options & (DbgOption::kDbgScene)))
	{
		populateRenderGraphMain(ctx);
	}

	// Object picking
	if(!!(m_options & DbgOption::kObjectPicking))
	{
		populateRenderGraphObjectPicking(ctx);
	}
}

void Dbg::populateRenderGraphMain(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Debug");

	GraphicsRenderPassTargetDesc colorRti(m_runCtx.m_rt);
	colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
	GraphicsRenderPassTargetDesc depthRti(getGBuffer().getDepthRt());
	depthRti.m_loadOperation = RenderTargetLoadOperation::kLoad;
	depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
	pass.setRenderpassInfo({colorRti}, &depthRti);

	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kRtvDsvWrite);
	pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvPixel | TextureUsageBit::kRtvDsvRead);

	const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
	if(visOut.m_dependency.isValid())
	{
		pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kSrvGeometry);
	}

	const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();
	if(fvisOut.m_dependency.isValid())
	{
		pass.newBufferDependency(fvisOut.m_dependency, BufferUsageBit::kSrvGeometry);
	}

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(Dbg);
		ANKI_ASSERT(!!(m_options & DbgOption::kDbgScene));

		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		// Set common state
		cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		cmdb.setDepthWrite(false);

		cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
		cmdb.setDepthCompareOperation(!!(m_options & DbgOption::kDepthTest) ? CompareOperation::kLess : CompareOperation::kAlways);
		cmdb.setLineWidth(2.0f);

		rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());

		// Common code for boxes stuff
		{
			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "RenderableBoxes");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			class Constants
			{
			public:
				Vec4 m_color;
				Mat4 m_viewProjMat;

				UVec3 m_padding;
				U32 m_depthFailureVisualization;
			} consts;
			consts.m_color = Vec4(1.0f, 1.0f, 0.0f, 1.0f);
			consts.m_viewProjMat = ctx.m_matrices.m_viewProjection;
			consts.m_depthFailureVisualization = !(m_options & DbgOption::kDepthTest);

			cmdb.setFastConstants(&consts, sizeof(consts));
			cmdb.bindVertexBuffer(0, BufferView(m_cubeVertsBuffer.get()), sizeof(Vec3));
			cmdb.setVertexAttribute(VertexAttributeSemantic::kPosition, 0, Format::kR32G32B32_Sfloat, 0);
			cmdb.bindIndexBuffer(BufferView(m_cubeIndicesBuffer.get()), IndexType::kU16);
		}

		// GBuffer AABBs
		const U32 gbufferAllAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
		if(!!(m_options & DbgOption::kBoundingBoxes) && gbufferAllAabbCount)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView());

			const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
			cmdb.bindSrv(2, 0, visOut.m_visibleAaabbIndicesBuffer);

			cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, gbufferAllAabbCount);
		}

		// Forward shading renderables
		const U32 forwardAllAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
		if(!!(m_options & DbgOption::kBoundingBoxes) && forwardAllAabbCount)
		{
			cmdb.bindSrv(1, 0, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView());

			const GpuVisibilityOutput& visOut = getForwardShading().getGpuVisibilityOutput();
			cmdb.bindSrv(2, 0, visOut.m_visibleAaabbIndicesBuffer);

			cmdb.drawIndexed(PrimitiveTopology::kLines, 12 * 2, forwardAllAabbCount);
		}

		// Draw non-renderables
		if(!!(m_options & DbgOption::kIcons))
		{
			drawNonRenderable(GpuSceneNonRenderableObjectType::kLight, GpuSceneArrays::Light::getSingleton().getElementCount(), ctx,
							  *m_pointLightImage, cmdb);
			drawNonRenderable(GpuSceneNonRenderableObjectType::kDecal, GpuSceneArrays::Decal::getSingleton().getElementCount(), ctx, *m_decalImage,
							  cmdb);
			drawNonRenderable(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe,
							  GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount(), ctx, *m_giProbeImage, cmdb);
			drawNonRenderable(GpuSceneNonRenderableObjectType::kReflectionProbe, GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount(),
							  ctx, *m_reflectionImage, cmdb);
		}

		// Physics
		if(!!(m_options & DbgOption::kPhysics))
		{
			class MyPhysicsDebugDrawerInterface final : public PhysicsDebugDrawerInterface
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

			PhysicsWorld::getSingleton().debugDraw(drawerInterface);

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
	});
}

void Dbg::populateRenderGraphObjectPicking(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
	const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();

	U32 maxVisibleCount = 0;
	if(visOut.containsDrawcalls())
	{
		maxVisibleCount += U32(visOut.m_visibleAaabbIndicesBuffer.getRange() / sizeof(LodAndGpuSceneRenderableBoundingVolumeIndex));
	}
	if(fvisOut.containsDrawcalls())
	{
		maxVisibleCount += U32(fvisOut.m_visibleAaabbIndicesBuffer.getRange() / sizeof(LodAndGpuSceneRenderableBoundingVolumeIndex));
	}
	const BufferView drawIndirectArgsBuff =
		GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<DrawIndexedIndirectArgs>(maxVisibleCount);

	const BufferView drawCountBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<U32>(1);
	const BufferHandle bufferHandle = rgraph.importBuffer(drawCountBuff, BufferUsageBit::kNone);

	const BufferView lodAndRenderableIndicesBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<U32>(maxVisibleCount);

	// Zero draw count
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Object Picking: Zero");

		pass.newBufferDependency(bufferHandle, BufferUsageBit::kCopyDestination);

		pass.setWork([drawCountBuff, lodAndRenderableIndicesBuff, drawIndirectArgsBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			cmdb.zeroBuffer(drawCountBuff);
			cmdb.zeroBuffer(lodAndRenderableIndicesBuff);
			cmdb.zeroBuffer(drawIndirectArgsBuff);
		});
	}

	// Prepare pass
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Object Picking: Prepare");

		if(visOut.m_dependency.isValid())
		{
			pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kSrvCompute);
		}

		if(fvisOut.m_dependency.isValid())
		{
			pass.newBufferDependency(fvisOut.m_dependency, BufferUsageBit::kSrvCompute);
		}

		pass.newBufferDependency(bufferHandle, BufferUsageBit::kUavCompute);

		pass.setWork([this, drawIndirectArgsBuff, drawCountBuff, lodAndRenderableIndicesBuff](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kCompute, "PrepareRenderableUuids");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			cmdb.bindSrv(1, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindSrv(2, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());

			cmdb.bindUav(0, 0, drawIndirectArgsBuff);
			cmdb.bindUav(1, 0, drawCountBuff);
			cmdb.bindUav(2, 0, lodAndRenderableIndicesBuff);

			// Do GBuffer
			const GpuVisibilityOutput& visOut = getGBuffer().getVisibilityOutput();
			if(visOut.containsDrawcalls())
			{
				cmdb.bindSrv(0, 0, GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getBufferView());
				cmdb.bindSrv(3, 0, visOut.m_visibleAaabbIndicesBuffer);

				const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeGBuffer::getSingleton().getElementCount();
				cmdb.dispatchCompute((allAabbCount + 63) / 64, 1, 1);
			}

			// Do ForwardShading
			const GpuVisibilityOutput& fvisOut = getForwardShading().getGpuVisibilityOutput();
			if(fvisOut.containsDrawcalls())
			{
				cmdb.bindSrv(0, 0, GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getBufferView());
				cmdb.bindSrv(3, 0, fvisOut.m_visibleAaabbIndicesBuffer);

				const U32 allAabbCount = GpuSceneArrays::RenderableBoundingVolumeForward::getSingleton().getElementCount();
				cmdb.dispatchCompute((allAabbCount + 63) / 64, 1, 1);
			}
		});
	}

	// The render pass that draws the UUIDs to a buffer
	const RenderTargetHandle objectPickingRt = rgraph.newRenderTarget(m_objectPickingRtDescr);
	const RenderTargetHandle objectPickingDepthRt = rgraph.newRenderTarget(m_objectPickingDepthRtDescr);
	{
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Object Picking: Draw UUIDs");

		pass.newBufferDependency(bufferHandle, BufferUsageBit::kIndirectDraw);
		pass.newTextureDependency(objectPickingRt, TextureUsageBit::kRtvDsvWrite);
		pass.newTextureDependency(objectPickingDepthRt, TextureUsageBit::kRtvDsvWrite);

		GraphicsRenderPassTargetDesc colorRti(objectPickingRt);
		colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
		GraphicsRenderPassTargetDesc depthRti(objectPickingDepthRt);
		depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
		depthRti.m_clearValue.m_depthStencil.m_depth = 1.0;
		depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
		pass.setRenderpassInfo({colorRti}, &depthRti);

		pass.setWork(
			[this, lodAndRenderableIndicesBuff, &ctx, drawIndirectArgsBuff, drawCountBuff, maxVisibleCount](RenderPassWorkContext& rgraphCtx) {
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				// Set common state
				cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
				cmdb.setDepthCompareOperation(CompareOperation::kLess);

				ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
				variantInitInfo.addMutation("OBJECT_TYPE", 0);
				variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "RenderableUuids");
				const ShaderProgramResourceVariant* variant;
				m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
				cmdb.bindShaderProgram(&variant->getProgram());

				cmdb.bindIndexBuffer(UnifiedGeometryBuffer::getSingleton().getBufferView(), IndexType::kU16);

				cmdb.bindSrv(0, 0, lodAndRenderableIndicesBuff);
				cmdb.bindSrv(1, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
				cmdb.bindSrv(2, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
				cmdb.bindSrv(3, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());
				cmdb.bindSrv(4, 0, UnifiedGeometryBuffer::getSingleton().getBufferView(), Format::kR16G16B16A16_Unorm);

				cmdb.setFastConstants(&ctx.m_matrices.m_viewProjection, sizeof(ctx.m_matrices.m_viewProjection));

				cmdb.drawIndexedIndirectCount(PrimitiveTopology::kTriangles, drawIndirectArgsBuff, sizeof(DrawIndexedIndirectArgs), drawCountBuff,
											  maxVisibleCount);
			});
	}

	// Read the UUID RT to get the UUID that is over the mouse
	{
		U32 uuid;
		PtrSize dataOut;
		getRenderer().getReadbackManager().readMostRecentData(m_readback, &uuid, sizeof(uuid), dataOut);
		if(dataOut)
		{
			m_runCtx.m_objUuid = uuid;
		}
		else
		{
			m_runCtx.m_objUuid = 0;
		}

		const BufferView readbackBuff = getRenderer().getReadbackManager().allocateStructuredBuffer<U32>(m_readback, 1);

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Object Picking: Picking");

		pass.newTextureDependency(objectPickingRt, TextureUsageBit::kSrvCompute);

		pass.setWork([this, readbackBuff, objectPickingRt](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			ShaderProgramResourceVariantInitInfo variantInitInfo(m_dbgProg);
			variantInitInfo.addMutation("OBJECT_TYPE", 0);
			variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kCompute, "RenderableUuidsPick");
			const ShaderProgramResourceVariant* variant;
			m_dbgProg->getOrCreateVariant(variantInitInfo, variant);
			cmdb.bindShaderProgram(&variant->getProgram());

			rgraphCtx.bindSrv(0, 0, objectPickingRt);
			cmdb.bindUav(0, 0, readbackBuff);

			Vec2 mousePos = Input::getSingleton().getMousePositionNdc();
			mousePos.y() = -mousePos.y();
			mousePos = mousePos / 2.0f + 0.5f;
			mousePos *= Vec2(getRenderer().getInternalResolution() / 2);

			const UVec4 consts(UVec2(mousePos), 0u, 0u);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchCompute(1, 1, 1);
		});
	}
}

} // end namespace anki
