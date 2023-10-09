// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/LensFlare.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static NumericCVar<U8> g_lensFlareMaxSpritesPerFlareCVar(CVarSubsystem::kRenderer, "LensFlareMaxSpritesPerFlare", 8, 4, 255,
														 "Max sprites per lens flare");
static NumericCVar<U8> g_lensFlareMaxFlaresCVar(CVarSubsystem::kRenderer, "LensFlareMaxFlares", 16, 8, 255, "Max flare count");

Error LensFlare::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize lens flare pass");
	}

	return err;
}

Error LensFlare::initInternal()
{
	ANKI_R_LOGV("Initializing lens flare");

	ANKI_CHECK(initSprite());
	ANKI_CHECK(initOcclusion());

	return Error::kNone;
}

Error LensFlare::initSprite()
{
	m_maxSpritesPerFlare = g_lensFlareMaxSpritesPerFlareCVar.get();
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/LensFlareSprite.ankiprogbin", m_realProg, m_realGrProg));

	return Error::kNone;
}

Error LensFlare::initOcclusion()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/LensFlareUpdateIndirectInfo.ankiprogbin", m_updateIndirectBuffProg));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_updateIndirectBuffProg);
	variantInitInfo.addConstant("kInDepthMapSize",
								UVec2(getRenderer().getInternalResolution().x() / 2 / 2, getRenderer().getInternalResolution().y() / 2 / 2));
	const ShaderProgramResourceVariant* variant;
	m_updateIndirectBuffProg->getOrCreateVariant(variantInitInfo, variant);
	m_updateIndirectBuffGrProg.reset(&variant->getProgram());

	return Error::kNone;
}

void LensFlare::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(LensFlare);
	const U32 flareCount = SceneGraph::getSingleton().getComponentArrays().getLensFlares().getSize();
	if(flareCount == 0)
	{
		m_runCtx = {};
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create indirect buffer
	m_runCtx.m_indirectBuff = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(DrawIndirectArgs) * flareCount);
	m_runCtx.m_indirectBuffHandle = rgraph.importBuffer(BufferUsageBit::kNone, m_runCtx.m_indirectBuff);

	// Create the pass
	ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Lens flare indirect");

	rpass.newBufferDependency(m_runCtx.m_indirectBuffHandle, BufferUsageBit::kUavComputeWrite);
	rpass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSampledCompute,
							   DepthDownscale::kEighthInternalResolution);

	rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(LensFlare);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		const U32 flareCount = SceneGraph::getSingleton().getComponentArrays().getLensFlares().getSize();
		ANKI_ASSERT(flareCount > 0);

		cmdb.bindShaderProgram(m_updateIndirectBuffGrProg.get());

		cmdb.setPushConstants(&ctx.m_matrices.m_viewProjectionJitter, sizeof(ctx.m_matrices.m_viewProjectionJitter));

		// Write flare info
		Vec4* flarePositions = allocateAndBindUav<Vec4>(cmdb, 0, 0, flareCount);
		for(const LensFlareComponent& comp : SceneGraph::getSingleton().getComponentArrays().getLensFlares())
		{
			*flarePositions = Vec4(comp.getWorldPosition(), 1.0f);
			++flarePositions;
		}

		rgraphCtx.bindUavBuffer(0, 1, m_runCtx.m_indirectBuffHandle);
		// Bind neareset because you don't need high quality
		cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_nearestNearestClamp.get());
		rgraphCtx.bindTexture(0, 3, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kEighthInternalResolution);

		cmdb.dispatchCompute(flareCount, 1, 1);
	});
}

void LensFlare::runDrawFlares(const RenderingContext& ctx, CommandBuffer& cmdb)
{
	const U32 flareCount = SceneGraph::getSingleton().getComponentArrays().getLensFlares().getSize();

	if(flareCount == 0)
	{
		return;
	}

	cmdb.bindShaderProgram(m_realGrProg.get());
	cmdb.setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);
	cmdb.setDepthWrite(false);

	U32 count = 0;
	for(const LensFlareComponent& comp : SceneGraph::getSingleton().getComponentArrays().getLensFlares())
	{
		// Compute position
		Vec4 lfPos = Vec4(comp.getWorldPosition(), 1.0f);
		Vec4 posClip = ctx.m_matrices.m_viewProjectionJitter * lfPos;

		/*if(posClip.x() > posClip.w() || posClip.x() < -posClip.w() || posClip.y() > posClip.w()
			|| posClip.y() < -posClip.w())
		{
			// Outside clip
			ANKI_ASSERT(0 && "Check that before");
		}*/

		U32 c = 0;
		U32 spritesCount = max<U32>(1, m_maxSpritesPerFlare);

		// Get uniform memory
		LensFlareSprite* tmpSprites = allocateAndBindUav<LensFlareSprite>(cmdb, 0, 0, spritesCount);
		WeakArray<LensFlareSprite> sprites(tmpSprites, spritesCount);

		// misc
		Vec2 posNdc = posClip.xy() / posClip.w();

		// First flare
		sprites[c].m_posScale = Vec4(posNdc, comp.getFirstFlareSize() * Vec2(1.0f, getRenderer().getAspectRatio()));
		sprites[c].m_depthPad3 = Vec4(0.0f);
		const F32 alpha = comp.getColorMultiplier().w() * (1.0f - pow(absolute(posNdc.x()), 6.0f))
						  * (1.0f - pow(absolute(posNdc.y()), 6.0f)); // Fade the flare on the edges
		sprites[c].m_color = Vec4(comp.getColorMultiplier().xyz(), alpha);
		++c;

		// Render
		cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearRepeat.get());
		cmdb.bindTexture(0, 2, &comp.getImage().getTextureView());

		cmdb.drawIndirect(PrimitiveTopology::kTriangleStrip, 1, count * sizeof(DrawIndirectArgs) + m_runCtx.m_indirectBuff.m_offset,
						  m_runCtx.m_indirectBuff.m_buffer);

		++count;
	}

	// Restore state
	cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	cmdb.setDepthWrite(true);
}

} // end namespace anki
