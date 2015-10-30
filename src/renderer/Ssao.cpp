// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssao.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/scene/Camera.h>
#include <anki/scene/SceneGraph.h>
#include <anki/util/Functions.h>
#include <anki/misc/ConfigSet.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

const U NOISE_TEX_SIZE = 8;
const U KERNEL_SIZE = 16;

//==============================================================================
static void genKernel(Vec3* ANKI_RESTRICT arr,
	Vec3* ANKI_RESTRICT arrEnd)
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

//==============================================================================
static void genNoise(Vec3* ANKI_RESTRICT arr,
	Vec3* ANKI_RESTRICT arrEnd)
{
	ANKI_ASSERT(arr && arrEnd && arr != arrEnd);

	do
	{
		// Calculate the normal
		arr->x() = randRange(-1.0f, 1.0f);
		arr->y() = randRange(-1.0f, 1.0f);
		arr->z() = 0.0;
		arr->normalize();
	} while(++arr != arrEnd);
}

//==============================================================================
class ShaderCommonUniforms
{
public:
	Vec4 m_projectionParams;
	Mat4 m_projectionMatrix;
};

//==============================================================================
// Ssao                                                                        =
//==============================================================================

//==============================================================================
const PixelFormat Ssao::RT_PIXEL_FORMAT(
	ComponentFormat::R8, TransformFormat::UNORM);

//==============================================================================
Error Ssao::createFb(FramebufferPtr& fb, TexturePtr& rt)
{
	// Set to bilinear because the blurring techniques take advantage of that
	m_r->createRenderTarget(m_width, m_height,
		RT_PIXEL_FORMAT, 1, SamplingFilter::LINEAR, 1, rt);

	// Create FB
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ssao::initInternal(const ConfigSet& config)
{
	m_enabled = config.getNumber("pps.ssao.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	m_blurringIterationsCount =
		config.getNumber("pps.ssao.blurringIterationsCount");

	//
	// Init the widths/heights
	//
	const F32 quality = config.getNumber("pps.ssao.renderingQuality");

	m_width = quality * (F32)m_r->getWidth();
	alignRoundUp(16, m_width);
	m_height = quality * (F32)m_r->getHeight();
	alignRoundUp(16, m_height);

	//
	// create FBOs
	//
	ANKI_CHECK(createFb(m_hblurFb, m_hblurRt));
	ANKI_CHECK(createFb(m_vblurFb, m_vblurRt));

	//
	// noise texture
	//
	TextureInitializer tinit;

	tinit.m_width = tinit.m_height = NOISE_TEX_SIZE;
	tinit.m_type = TextureType::_2D;
	tinit.m_format =
		PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
	tinit.m_mipmapsCount = 1;
	tinit.m_sampling.m_minMagFilter = SamplingFilter::NEAREST;
	tinit.m_sampling.m_repeat = true;

	m_noiseTex = getGrManager().newInstance<Texture>(tinit);

	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>();

	PtrSize noiseSize = NOISE_TEX_SIZE * NOISE_TEX_SIZE * sizeof(Vec3);

	DynamicBufferToken token;
	Vec3* noise = static_cast<Vec3*>(
		getGrManager().allocateFrameHostVisibleMemory(noiseSize,
		BufferUsage::TRANSFER, token));

	genNoise(noise, noise + NOISE_TEX_SIZE * NOISE_TEX_SIZE);

	cmdb->textureUpload(m_noiseTex, 0, 0, token);
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

		tmp.sprintf("vec3(%f, %f, %f) %s",
			kernel[i].x(), kernel[i].y(), kernel[i].z(),
			(i != kernel.size() - 1) ? ", " : ")");

		kernelStr.append(tmp);
	}

	//
	// Shaders
	//
	m_uniformsBuff = getGrManager().newInstance<Buffer>(
		sizeof(ShaderCommonUniforms), BufferUsageBit::UNIFORM,
		BufferAccessBit::CLIENT_WRITE);

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = RT_PIXEL_FORMAT;

	StringAuto pps(getAllocator());

	// main pass prog
	pps.sprintf(
		"#define NOISE_MAP_SIZE %u\n"
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n"
		"#define KERNEL_SIZE %u\n"
		"#define KERNEL_ARRAY %s\n",
		NOISE_TEX_SIZE, m_width, m_height, KERNEL_SIZE, &kernelStr[0]);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_ssaoFrag, "shaders/PpsSsao.frag.glsl", pps.toCString(), "r_"));

	m_r->createDrawQuadPipeline(
		m_ssaoFrag->getGrShader(), colorState, m_ssaoPpline);

	// blurring progs
	const char* SHADER_FILENAME =
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	pps.destroy(getAllocator());
	pps.sprintf(
		"#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 11\n",
		m_height);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_hblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	m_r->createDrawQuadPipeline(
		m_hblurFrag->getGrShader(), colorState, m_hblurPpline);

	pps.destroy(getAllocator());
	pps.sprintf(
		"#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 9\n",
		m_width);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_vblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	m_r->createDrawQuadPipeline(
		m_vblurFrag->getGrShader(), colorState, m_vblurPpline);

	//
	// Resource groups
	//
	ResourceGroupInitializer rcinit;
	rcinit.m_textures[0].m_texture = m_r->getMs().getDepthRt();
	rcinit.m_textures[1].m_texture = m_r->getMs().getRt2();
	rcinit.m_textures[2].m_texture = m_noiseTex;
	rcinit.m_uniformBuffers[0].m_buffer = m_uniformsBuff;
	m_rcFirst = getGrManager().newInstance<ResourceGroup>(rcinit);

	rcinit = ResourceGroupInitializer();
	rcinit.m_textures[0].m_texture = m_vblurRt;
	m_hblurRc = getGrManager().newInstance<ResourceGroup>(rcinit);

	rcinit = ResourceGroupInitializer();
	rcinit.m_textures[0].m_texture = m_hblurRt;
	m_vblurRc = getGrManager().newInstance<ResourceGroup>(rcinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ssao::init(const ConfigSet& config)
{
	Error err = initInternal(config);

	if(err)
	{
		ANKI_LOGE("Failed to init PPS SSAO");
	}

	return err;
}

//==============================================================================
void Ssao::run(CommandBufferPtr& cmdb)
{
	ANKI_ASSERT(m_enabled);

	// 1st pass
	//
	cmdb->bindFramebuffer(m_vblurFb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->bindPipeline(m_ssaoPpline);
	cmdb->bindResourceGroup(m_rcFirst, 0, nullptr);

	// Write common block
	const FrustumComponent& camFr = m_r->getActiveFrustumComponent();

	if(m_commonUboUpdateTimestamp
			< m_r->getProjectionParametersUpdateTimestamp()
		|| m_commonUboUpdateTimestamp < camFr.getTimestamp()
		|| m_commonUboUpdateTimestamp == 1)
	{
		DynamicBufferToken token;
		ShaderCommonUniforms* blk = static_cast<ShaderCommonUniforms*>(
			getGrManager().allocateFrameHostVisibleMemory(
			sizeof(ShaderCommonUniforms), BufferUsage::TRANSFER, token));

		blk->m_projectionParams = m_r->getProjectionParameters();
		blk->m_projectionMatrix = camFr.getProjectionMatrix();

		cmdb->writeBuffer(m_uniformsBuff, 0, token);

		m_commonUboUpdateTimestamp = getGlobalTimestamp();
	}

	// Draw
	m_r->drawQuad(cmdb);

	// Blurring passes
	//
	for(U i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		cmdb->bindFramebuffer(m_hblurFb);
		cmdb->bindPipeline(m_hblurPpline);
		cmdb->bindResourceGroup(m_hblurRc, 0, nullptr);
		m_r->drawQuad(cmdb);

		// vpass
		cmdb->bindFramebuffer(m_vblurFb);
		cmdb->bindPipeline(m_vblurPpline);
		cmdb->bindResourceGroup(m_vblurRc, 0, nullptr);
		m_r->drawQuad(cmdb);
	}
}

} // end namespace anki
