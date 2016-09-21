// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssao.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/HalfDepth.h>
#include <anki/scene/SceneGraph.h>
#include <anki/util/Functions.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

const U NOISE_TEX_SIZE = 4;
const U KERNEL_SIZE = 16;

static void genKernel(Vec3* ANKI_RESTRICT arr, Vec3* ANKI_RESTRICT arrEnd)
{
	ANKI_ASSERT(arr && arrEnd && arr != arrEnd);

	do
	{
		// Calculate the normal
		arr->x() = randRange(-1.0f, 1.0f);
		arr->y() = randRange(-1.0f, 1.0f);
		arr->z() = randRange(0.0f, 1.0f);
		arr->normalize();

		// Adjust the length
		(*arr) *= randRange(0.0f, 1.0f);
	} while(++arr != arrEnd);
}

static void genNoise(Vec4* ANKI_RESTRICT arr, Vec4* ANKI_RESTRICT arrEnd)
{
	ANKI_ASSERT(arr && arrEnd && arr != arrEnd);

	do
	{
		// Calculate the normal
		Vec3 v(randRange(-1.0f, 1.0f), randRange(-1.0f, 1.0f), 0.0f);
		v.normalize();
		*arr = Vec4(v, 0.0f);
	} while(++arr != arrEnd);
}

const PixelFormat Ssao::RT_PIXEL_FORMAT(ComponentFormat::R8, TransformFormat::UNORM);

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
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

Error Ssao::initInternal(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_blurringIterationsCount = config.getNumber("ssao.blurringIterationsCount");

	//
	// Init the widths/heights
	//
	m_width = m_r->getWidth() / SSAO_FRACTION;
	m_height = m_r->getHeight() / SSAO_FRACTION;

	ANKI_LOGI("Initializing SSAO. Size %ux%u", m_width, m_height);

	//
	// create FBOs
	//
	ANKI_CHECK(createFb(m_hblurFb, m_hblurRt));
	ANKI_CHECK(createFb(m_vblurFb, m_vblurRt));

	//
	// noise texture
	//
	TextureInitInfo tinit;

	tinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::UPLOAD;
	tinit.m_width = tinit.m_height = NOISE_TEX_SIZE;
	tinit.m_depth = 1;
	tinit.m_layerCount = 1;
	tinit.m_type = TextureType::_2D;
	tinit.m_format = PixelFormat(ComponentFormat::R32G32B32A32, TransformFormat::FLOAT);
	tinit.m_mipmapsCount = 1;
	tinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	tinit.m_sampling.m_repeat = true;

	m_noiseTex = gr.newInstance<Texture>(tinit);

	Array<Vec4, NOISE_TEX_SIZE * NOISE_TEX_SIZE> noise;
	genNoise(&noise[0], &noise[0] + noise.getSize());

	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags = CommandBufferFlag::SMALL_BATCH;

	CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>(cmdbInit);

	TextureSurfaceInfo surf(0, 0, 0, 0);

	cmdb->setTextureSurfaceBarrier(m_noiseTex, TextureUsageBit::NONE, TextureUsageBit::UPLOAD, surf);

	cmdb->uploadTextureSurfaceCopyData(m_noiseTex, surf, &noise[0], sizeof(noise));

	cmdb->setTextureSurfaceBarrier(m_noiseTex, TextureUsageBit::UPLOAD, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->flush();

	//
	// Kernel
	//
	StringAuto kernelStr(getAllocator());
	Array<Vec3, KERNEL_SIZE> kernel;

	genKernel(kernel.begin(), kernel.end());
	kernelStr.create("vec3[](");
	for(U i = 0; i < kernel.size(); i++)
	{
		StringAuto tmp(getAllocator());

		tmp.sprintf(
			"vec3(%f, %f, %f) %s", kernel[i].x(), kernel[i].y(), kernel[i].z(), (i != kernel.size() - 1) ? ", " : ")");

		kernelStr.append(tmp);
	}

	//
	// Shaders
	//
	PipelineInitInfo ppinit;
	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = RT_PIXEL_FORMAT;
	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	StringAuto pps(getAllocator());

	// vert shader
	ANKI_CHECK(getResourceManager().loadResource("shaders/Quad.vert.glsl", m_quadVert));

	ppinit.m_shaders[ShaderType::VERTEX] = m_quadVert->getGrShader();

	// main pass prog
	pps.destroy();
	pps.sprintf("#define NOISE_MAP_SIZE %u\n"
				"#define WIDTH %u\n"
				"#define HEIGHT %u\n"
				"#define KERNEL_SIZE %u\n"
				"#define KERNEL_ARRAY %s\n",
		NOISE_TEX_SIZE,
		m_width,
		m_height,
		KERNEL_SIZE,
		&kernelStr[0]);

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_ssaoFrag, "shaders/Ssao.frag.glsl", pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::FRAGMENT] = m_ssaoFrag->getGrShader();

	m_ssaoPpline = getGrManager().newInstance<Pipeline>(ppinit);

	// h blur
	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.frag.glsl";

	pps.destroy();
	pps.sprintf("#define HPASS\n"
				"#define COL_R\n"
				"#define TEXTURE_SIZE vec2(%f, %f)\n"
				"#define KERNEL_SIZE 19\n",
		F32(m_width),
		F32(m_height));

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_hblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::FRAGMENT] = m_hblurFrag->getGrShader();

	m_hblurPpline = getGrManager().newInstance<Pipeline>(ppinit);

	// v blur
	pps.destroy();
	pps.sprintf("#define VPASS\n"
				"#define COL_R\n"
				"#define TEXTURE_SIZE vec2(%f, %f)\n"
				"#define KERNEL_SIZE 15\n",
		F32(m_width),
		F32(m_height));

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_vblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::FRAGMENT] = m_vblurFrag->getGrShader();

	m_vblurPpline = getGrManager().newInstance<Pipeline>(ppinit);

	//
	// Resource groups
	//
	ResourceGroupInitInfo rcinit;
	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	sinit.m_repeat = false;

	rcinit.m_textures[0].m_texture = m_r->getHalfDepth().m_depthRt;
	rcinit.m_textures[0].m_sampler = gr.newInstance<Sampler>(sinit);
	rcinit.m_textures[0].m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ;

	rcinit.m_textures[1].m_texture = m_r->getMs().getRt2();
	rcinit.m_textures[1].m_sampler = rcinit.m_textures[0].m_sampler;

	rcinit.m_textures[2].m_texture = m_noiseTex;

	rcinit.m_uniformBuffers[0].m_uploadedMemory = true;
	rcinit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;
	m_rcFirst = gr.newInstance<ResourceGroup>(rcinit);

	rcinit = ResourceGroupInitInfo();
	rcinit.m_textures[0].m_texture = m_vblurRt;
	m_hblurRc = gr.newInstance<ResourceGroup>(rcinit);

	rcinit = ResourceGroupInitInfo();
	rcinit.m_textures[0].m_texture = m_hblurRt;
	m_vblurRc = gr.newInstance<ResourceGroup>(rcinit);

	gr.finish();
	return ErrorCode::NONE;
}

Error Ssao::init(const ConfigSet& config)
{
	Error err = initInternal(config);

	if(err)
	{
		ANKI_LOGE("Failed to init PPS SSAO");
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
	cmdb->bindPipeline(m_ssaoPpline);

	TransientMemoryInfo inf;
	inf.m_uniformBuffers[0] = m_r->getCommonUniformsTransientMemoryToken();
	cmdb->bindResourceGroup(m_rcFirst, 0, &inf);

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
		cmdb->bindPipeline(m_hblurPpline);
		cmdb->bindResourceGroup(m_hblurRc, 0, nullptr);
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
		cmdb->bindPipeline(m_vblurPpline);
		cmdb->bindResourceGroup(m_vblurRc, 0, nullptr);
		m_r->drawQuad(cmdb);
		cmdb->endRenderPass();
	}
}

} // end namespace anki
