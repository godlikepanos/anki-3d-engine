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
#include <anki/scene/FrustumComponent.h>

namespace anki
{

const U NOISE_TEX_SIZE = 4;
const U KERNEL_SIZE = 16;

template<typename TVec>
static void genHemisphere(TVec* ANKI_RESTRICT arr, TVec* ANKI_RESTRICT arrEnd)
{
	ANKI_ASSERT(arr && arrEnd && arr != arrEnd);

	do
	{
		// Calculate the normal
		arr->x() = randRange(-1.0, 1.0);
		arr->y() = randRange(-1.0, 1.0);
		arr->z() = randRange(0.0, 1.0);
		arr->normalize();

		// Adjust the length
		(*arr) *= randRange(0.0, 1.0);
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

void Ssao::createHemisphereLut()
{
	constexpr F64 PI = getPi<F64>();
	constexpr F64 MIN_ANGLE = PI / 8.0;

	// Compute the hemisphere
	Array<DVec3, KERNEL_SIZE> kernel;
	genHemisphere(&kernel[0], &kernel[0] + KERNEL_SIZE);

	constexpr U LUT_TEX_SIZE_X = 2.0 * PI / MIN_ANGLE;
	constexpr U LUT_TEX_SIZE_Y = PI / MIN_ANGLE;
	constexpr U LUT_TEX_LAYERS = KERNEL_SIZE;

	Array<Array2d<Vec4, LUT_TEX_SIZE_Y, LUT_TEX_SIZE_X>, LUT_TEX_LAYERS> lutTexData;

	UVec3 counts(0u);
	U totalCount = 0;
	(void)totalCount;
	for(F64 theta = 0.0; theta < 2.0 * PI; theta += MIN_ANGLE)
	{
		counts.y() = 0;
		for(F64 phi = 0.0; phi < PI; phi += MIN_ANGLE)
		{
			// Compute the normal from the spherical coordinates
			DVec3 normal;
			normal.x() = cos(theta) * sin(phi);
			normal.y() = sin(theta) * sin(phi);
			normal.z() = cos(phi);
			normal.normalize();

			// Compute a tangent & bitangent
			DVec3 bitangent(0.01, 1.0, 0.01);
			bitangent.normalize();

			DVec3 tangent = bitangent.cross(normal);
			tangent.normalize();

			bitangent = normal.cross(tangent);

			// Set the TBN matrix
			DMat3 rot;
			rot.setColumns(tangent, bitangent, normal);

			counts.z() = 0;
			for(U k = 0; k < KERNEL_SIZE; ++k)
			{
				DVec3 rotVec = rot * kernel[k];

				lutTexData[counts.z()][counts.y()][counts.x()] = Vec4(rotVec.x(), rotVec.y(), rotVec.z(), 0.0);

				++counts.z();
				++totalCount;
			}

			++counts.y();
		}

		++counts.x();
	}

	ANKI_ASSERT(totalCount == (LUT_TEX_SIZE_Y * LUT_TEX_SIZE_X * LUT_TEX_LAYERS));

	// Create the texture
	TextureInitInfo tinit;
	tinit.m_usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::UPLOAD;
	tinit.m_width = LUT_TEX_SIZE_X;
	tinit.m_height = LUT_TEX_SIZE_Y;
	tinit.m_depth = 1;
	tinit.m_layerCount = LUT_TEX_LAYERS;
	tinit.m_type = TextureType::_2D_ARRAY;
	tinit.m_format = PixelFormat(ComponentFormat::R32G32B32A32, TransformFormat::FLOAT);
	tinit.m_mipmapsCount = 1;
	tinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	tinit.m_sampling.m_repeat = false;

	m_hemisphereLut = getGrManager().newInstance<Texture>(tinit);

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(cmdbinit);

	for(U i = 0; i < LUT_TEX_LAYERS; ++i)
	{
		cmdb->setTextureSurfaceBarrier(
			m_hemisphereLut, TextureUsageBit::NONE, TextureUsageBit::UPLOAD, TextureSurfaceInfo(0, 0, 0, i));
	}

	for(U i = 0; i < LUT_TEX_LAYERS; ++i)
	{
		cmdb->uploadTextureSurfaceCopyData(
			m_hemisphereLut, TextureSurfaceInfo(0, 0, 0, i), &lutTexData[i][0][0], sizeof(lutTexData[i]));
	}

	for(U i = 0; i < LUT_TEX_LAYERS; ++i)
	{
		cmdb->setTextureSurfaceBarrier(m_hemisphereLut,
			TextureUsageBit::UPLOAD,
			TextureUsageBit::SAMPLED_FRAGMENT,
			TextureSurfaceInfo(0, 0, 0, i));
	}

	cmdb->flush();
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

	genHemisphere(kernel.begin(), kernel.end());
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
	// Lookup texture
	//
	createHemisphereLut();

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

	rcinit.m_textures[3].m_texture = m_hemisphereLut;

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
	Vec4* unis = static_cast<Vec4*>(getGrManager().allocateFrameTransientMemory(
		sizeof(Vec4) * 2, BufferUsageBit::UNIFORM_ALL, inf.m_uniformBuffers[0]));

	const FrustumComponent& frc = *ctx.m_frustumComponent;
	const Mat4& pmat = frc.getProjectionMatrix();
	*unis = frc.getProjectionParameters();
	++unis;
	*unis = Vec4(pmat(0, 0), pmat(1, 1), pmat(2, 2), pmat(2, 3));

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
