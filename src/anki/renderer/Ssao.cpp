// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssao.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/scene/SceneGraph.h>
#include <anki/util/Functions.h>
#include <anki/misc/ConfigSet.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

const F32 HEMISPHERE_RADIUS = 3.0; // In game units
const U SAMPLES = 8;
const PixelFormat Ssao::RT_PIXEL_FORMAT(ComponentFormat::R8, TransformFormat::UNORM);

template<typename TVec>
static void genDisk(TVec* ANKI_RESTRICT arr, TVec* ANKI_RESTRICT arrEnd)
{
	ANKI_ASSERT(arr && arrEnd && arr != arrEnd);

	do
	{
		arr->x() = randRange(0.1, 1.0) * 2.0 - 1.0;
		arr->y() = randRange(0.1, 1.0) * 2.0 - 1.0;
	} while(++arr != arrEnd);
}

Error Ssao::createFb(FramebufferPtr& fb, TexturePtr& rt)
{
	// Set to bilinear because the blurring techniques take advantage of that
	m_r->createRenderTarget(m_width,
		m_height,
		RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR,
		1,
		rt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

Error Ssao::initInternal(const ConfigSet& config)
{
	m_blurringIterationsCount = config.getNumber("ssao.blurringIterationsCount");

	//
	// Init the widths/heights
	//
	m_width = m_r->getWidth() / SSAO_FRACTION;
	m_height = m_r->getHeight() / SSAO_FRACTION;

	ANKI_R_LOGI("Initializing SSAO. Size %ux%u", m_width, m_height);

	//
	// create FBOs
	//
	ANKI_CHECK(createFb(m_hblurFb, m_hblurRt));
	ANKI_CHECK(createFb(m_vblurFb, m_vblurRt));

	//
	// noise texture
	//
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_noiseTex));

	//
	// Kernel
	//
	StringAuto kernelStr(getAllocator());
	Array<Vec2, SAMPLES> kernel;

	genDisk(kernel.begin(), kernel.end());
	kernelStr.create("vec2[](");
	for(U i = 0; i < kernel.size(); i++)
	{
		StringAuto tmp(getAllocator());
		tmp.sprintf("vec2(%f, %f)%s", kernel[i].x(), kernel[i].y(), (i != kernel.getSize() - 1) ? ", " : ")");
		kernelStr.append(tmp);
	}

	//
	// Shaders
	//

	// main pass prog
	ANKI_CHECK(m_r->createShaderf("shaders/Ssao.frag.glsl",
		m_ssaoFrag,
		"#define NOISE_MAP_SIZE %u\n"
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n"
		"#define SAMPLES %u\n"
		"#define KERNEL_ARRAY %s\n"
		"#define RADIUS float(%f)\n",
		m_noiseTex->getWidth(),
		m_width,
		m_height,
		SAMPLES,
		&kernelStr[0],
		HEMISPHERE_RADIUS));

	m_r->createDrawQuadShaderProgram(m_ssaoFrag->getGrShader(), m_ssaoProg);

	// h blur
	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.frag.glsl";

	ANKI_CHECK(m_r->createShaderf(SHADER_FILENAME,
		m_hblurFrag,
		"#define HPASS\n"
		"#define COL_R\n"
		"#define TEXTURE_SIZE vec2(%f, %f)\n"
		"#define KERNEL_SIZE 11\n",
		F32(m_width),
		F32(m_height)));

	m_r->createDrawQuadShaderProgram(m_hblurFrag->getGrShader(), m_hblurProg);

	// v blur
	ANKI_CHECK(m_r->createShaderf(SHADER_FILENAME,
		m_vblurFrag,
		"#define VPASS\n"
		"#define COL_R\n"
		"#define TEXTURE_SIZE vec2(%f, %f)\n"
		"#define KERNEL_SIZE 9\n",
		F32(m_width),
		F32(m_height)));

	m_r->createDrawQuadShaderProgram(m_vblurFrag->getGrShader(), m_vblurProg);

	return ErrorCode::NONE;
}

Error Ssao::init(const ConfigSet& config)
{
	Error err = initInternal(config);

	if(err)
	{
		ANKI_R_LOGE("Failed to init PPS SSAO");
	}

	return err;
}

void Ssao::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_vblurRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Ssao::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_vblurRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Ssao::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// 1st pass
	//
	cmdb->beginRenderPass(m_vblurFb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->bindShaderProgram(m_ssaoProg);

	cmdb->bindTexture(0, 0, m_r->getDepthDownscale().m_qd.m_depthRt);
	cmdb->bindTexture(0, 1, m_r->getMs().m_rt2);
	cmdb->bindTexture(0, 2, m_noiseTex->getGrTexture());

	Vec4* unis = allocateAndBindUniforms<Vec4*>(sizeof(Vec4) * 2, cmdb, 0, 0);
	const FrustumComponent& frc = *ctx.m_frustumComponent;
	const Mat4& pmat = frc.getProjectionMatrix();
	*unis = frc.getProjectionParameters();
	++unis;
	*unis = Vec4(pmat(0, 0), pmat(1, 1), pmat(2, 2), pmat(2, 3));

	// Draw
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// Blurring passes
	//
	for(U i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		cmdb->setTextureSurfaceBarrier(m_vblurRt,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(m_hblurRt,
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->beginRenderPass(m_hblurFb);
		cmdb->bindShaderProgram(m_hblurProg);
		cmdb->bindTexture(0, 0, m_vblurRt);
		m_r->drawQuad(cmdb);
		cmdb->endRenderPass();

		// vpass
		cmdb->setTextureSurfaceBarrier(m_hblurRt,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->setTextureSurfaceBarrier(m_vblurRt,
			TextureUsageBit::NONE,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			TextureSurfaceInfo(0, 0, 0, 0));
		cmdb->beginRenderPass(m_vblurFb);
		cmdb->bindShaderProgram(m_vblurProg);
		cmdb->bindTexture(0, 0, m_hblurRt);
		m_r->drawQuad(cmdb);
		cmdb->endRenderPass();
	}
}

} // end namespace anki
