// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Ssao.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/util/Functions.h"
#include "anki/misc/ConfigSet.h"

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
Error Ssao::createFb(FramebufferHandle& fb, TextureHandle& rt)
{
	ANKI_CHECK(m_r->createRenderTarget(m_width, m_height, 
		PixelFormat(ComponentFormat::R8, TransformFormat::UNORM), 
		1, true, rt));

	// Set to bilinear because the blurring techniques take advantage of that
	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(&getGrManager()));

	// create FB
	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = rt;
	fbInit.m_colorAttachments[0].m_loadOperation = 
		AttachmentLoadOperation::DONT_CARE;
	ANKI_CHECK(fb.create(cmdBuff, fbInit));

	cmdBuff.flush();

	return ErrorCode::NONE;
}

//==============================================================================
Error Ssao::initInternal(const ConfigSet& config)
{
	m_enabled = config.get("pps.ssao.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	m_blurringIterationsCount = 
		config.get("pps.ssao.blurringIterationsCount");

	//
	// Init the widths/heights
	//
	const F32 quality = config.get("pps.ssao.renderingQuality");

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
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&getGrManager()));

	Array<Vec3, NOISE_TEX_SIZE * NOISE_TEX_SIZE> noise;

	genNoise(&noise[0], &noise[0] + noise.getSize());

	TextureHandle::Initializer tinit;

	tinit.m_width = tinit.m_height = NOISE_TEX_SIZE;
	tinit.m_type = TextureType::_2D;
	tinit.m_format = 
		PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
	tinit.m_mipmapsCount = 1;
	tinit.m_sampling.m_filterType = SamplingFilter::NEAREST;
	tinit.m_sampling.m_repeat = true;
	tinit.m_data[0][0].m_ptr = static_cast<void*>(&noise[0]);
	tinit.m_data[0][0].m_size = sizeof(noise);

	m_noiseTex.create(cmdb, tinit);

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
	ANKI_CHECK(m_uniformsBuff.create(cmdb, GL_SHADER_STORAGE_BUFFER, 
		nullptr, sizeof(ShaderCommonUniforms), GL_DYNAMIC_STORAGE_BIT));

	StringAuto pps(getAllocator());

	// main pass prog
	pps.sprintf(
		"#define NOISE_MAP_SIZE %u\n"
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n"
		"#define KERNEL_SIZE %u\n"
		"#define KERNEL_ARRAY %s\n",
		NOISE_TEX_SIZE, m_width, m_height, KERNEL_SIZE, &kernelStr[0]);

	ANKI_CHECK(m_ssaoFrag.loadToCache(&getResourceManager(),
		"shaders/PpsSsao.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_ssaoFrag->getGrShader(), m_ssaoPpline));

	// blurring progs
	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	pps.destroy(getAllocator());
	pps.sprintf(
		"#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 7\n", 
		m_height);

	ANKI_CHECK(m_hblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_hblurFrag->getGrShader(), m_hblurPpline));

	pps.destroy(getAllocator());
	pps.sprintf(
		"#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 7\n", 
		m_width);

	ANKI_CHECK(m_vblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_vblurFrag->getGrShader(), m_vblurPpline));

	cmdb.flush();

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
Error Ssao::run(CommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);

	const Camera& cam = m_r->getSceneGraph().getActiveCamera();

	cmdb.setViewport(0, 0, m_width, m_height);

	// 1st pass
	//
	m_vblurFb.bind(cmdb);
	m_ssaoPpline.bind(cmdb);

	m_uniformsBuff.bindShaderBuffer(cmdb, 0);

	Array<TextureHandle, 3> tarr = {{
		m_r->getDp().getSmallDepthRt(),
		m_r->getMs()._getRt1(),
		m_noiseTex}};
	cmdb.bindTextures(0, tarr.begin(), tarr.getSize());

	// Write common block
	const FrustumComponent& camFr = cam.getComponent<FrustumComponent>();

	if(m_commonUboUpdateTimestamp 
			< m_r->getProjectionParametersUpdateTimestamp()
		|| m_commonUboUpdateTimestamp < camFr.getTimestamp()
		|| m_commonUboUpdateTimestamp == 1)
	{
		ShaderCommonUniforms blk;
		blk.m_projectionParams = m_r->getProjectionParameters();
		blk.m_projectionMatrix = camFr.getProjectionMatrix().getTransposed();

		m_uniformsBuff.write(cmdb, &blk, sizeof(blk), 0, 0, sizeof(blk));
		m_commonUboUpdateTimestamp = getGlobalTimestamp();
	}

	// Draw
	m_r->drawQuad(cmdb);

	// Blurring passes
	//
	for(U i = 0; i < m_blurringIterationsCount; i++)
	{
		if(i == 0)
		{
			Array<TextureHandle, 2> tarr = {{m_hblurRt, m_vblurRt}};
			cmdb.bindTextures(0, tarr.begin(), tarr.getSize());
		}

		// hpass
		m_hblurFb.bind(cmdb);
		m_hblurPpline.bind(cmdb);
		m_r->drawQuad(cmdb);

		// vpass
		m_vblurFb.bind(cmdb);
		m_vblurPpline.bind(cmdb);
		m_r->drawQuad(cmdb);
	}

	return ErrorCode::NONE;
}

} // end namespace anki
