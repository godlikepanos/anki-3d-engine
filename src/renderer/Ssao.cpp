// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
void Ssao::createFb(GlFramebufferHandle& fb, GlTextureHandle& rt)
{
	m_r->createRenderTarget(m_width, m_height, GL_RED, GL_RED, 
		GL_UNSIGNED_BYTE, 1, rt);

	// Set to bilinear because the blurring techniques take advantage of that
	GlCommandBufferHandle cmdBuff;
	cmdBuff.create(&getGlDevice());
	rt.setFilter(cmdBuff, GlTextureHandle::Filter::LINEAR);

	// create FB
	fb.create(cmdBuff, {{rt, GL_COLOR_ATTACHMENT0}});

	cmdBuff.flush();
}

//==============================================================================
void Ssao::initInternal(const ConfigSet& config)
{
	m_enabled = config.get("pps.ssao.enabled");

	if(!m_enabled)
	{
		return;
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
	createFb(m_hblurFb, m_hblurRt);
	createFb(m_vblurFb, m_vblurRt);

	//
	// noise texture
	//
	GlCommandBufferHandle cmdb;
	cmdb.create(&getGlDevice());

	GlClientBufferHandle noise;
	noise.create(
		cmdb, sizeof(Vec3) * NOISE_TEX_SIZE * NOISE_TEX_SIZE, nullptr);

	genNoise((Vec3*)noise.getBaseAddress(), 
		(Vec3*)((U8*)noise.getBaseAddress() + noise.getSize()));

	GlTextureHandle::Initializer tinit;

	tinit.m_width = tinit.m_height = NOISE_TEX_SIZE;
	tinit.m_target = GL_TEXTURE_2D;
	tinit.m_internalFormat = GL_RGB32F;
	tinit.m_format = GL_RGB;
	tinit.m_type = GL_FLOAT;
	tinit.m_filterType = GlTextureHandle::Filter::NEAREST;
	tinit.m_repeat = true;
	tinit.m_mipmapsCount = 1;
	tinit.m_data[0][0] = noise;

	m_noiseTex.create(cmdb, tinit);

	//
	// Kernel
	//
	String kernelStr(getAllocator());
	Array<Vec3, KERNEL_SIZE> kernel;

	genKernel(kernel.begin(), kernel.end());
	kernelStr = "vec3[](";
	for(U i = 0; i < kernel.size(); i++)
	{
		String tmp(getAllocator());

		tmp.sprintf("vec3(%f, %f, %f) %s",
			kernel[i].x(), kernel[i].y(), kernel[i].z(),
			(i != kernel.size() - 1) ? ", " : ")");

		kernelStr += tmp;
	}

	//
	// Shaders
	//
	m_uniformsBuff.create(cmdb, GL_SHADER_STORAGE_BUFFER, 
		sizeof(ShaderCommonUniforms), GL_DYNAMIC_STORAGE_BIT);

	String pps(getAllocator());

	// main pass prog
	pps.sprintf(
		"#define NOISE_MAP_SIZE %u\n"
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n"
		"#define KERNEL_SIZE %u\n"
		"#define KERNEL_ARRAY %s\n",
		NOISE_TEX_SIZE, m_width, m_height, KERNEL_SIZE, &kernelStr[0]);

	m_ssaoFrag.loadToCache(&getResourceManager(),
		"shaders/PpsSsao.frag.glsl", pps.toCString(), "r_");


	m_ssaoPpline = m_r->createDrawQuadProgramPipeline(
		m_ssaoFrag->getGlProgram());

	// blurring progs
	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	pps.sprintf(
		"#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 7\n", 
		m_height);

	m_hblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_");

	m_hblurPpline = m_r->createDrawQuadProgramPipeline(
		m_hblurFrag->getGlProgram());

	pps.sprintf(
		"#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 7\n", 
		m_width);

	m_vblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_");

	m_vblurPpline = m_r->createDrawQuadProgramPipeline(
		m_vblurFrag->getGlProgram());

	cmdb.flush();
}

//==============================================================================
void Ssao::init(const ConfigSet& config)
{
	try
	{
		initInternal(config);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init PPS SSAO") << e;
	}
}

//==============================================================================
void Ssao::run(GlCommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);

	const Camera& cam = m_r->getSceneGraph().getActiveCamera();

	cmdb.setViewport(0, 0, m_width, m_height);

	// 1st pass
	//
	m_vblurFb.bind(cmdb, true);
	m_ssaoPpline.bind(cmdb);

	m_uniformsBuff.bindShaderBuffer(cmdb, 0);

	Array<GlTextureHandle, 3> tarr = {{
		m_r->getMs()._getSmallDepthRt(),
		m_r->getMs()._getRt1(),
		m_noiseTex}};
	cmdb.bindTextures(0, tarr.begin(), tarr.getSize());

	// Write common block
	if(m_commonUboUpdateTimestamp 
			< m_r->getProjectionParametersUpdateTimestamp()
		|| m_commonUboUpdateTimestamp < cam.FrustumComponent::getTimestamp()
		|| m_commonUboUpdateTimestamp == 1)
	{
		GlClientBufferHandle tmpBuff;
		tmpBuff.create(cmdb, sizeof(ShaderCommonUniforms),
			nullptr);

		ShaderCommonUniforms& blk = 
			*((ShaderCommonUniforms*)tmpBuff.getBaseAddress());

		blk.m_projectionParams = m_r->getProjectionParameters();

		blk.m_projectionMatrix = cam.getProjectionMatrix().getTransposed();

		m_uniformsBuff.write(cmdb, tmpBuff, 0, 0, tmpBuff.getSize());
		m_commonUboUpdateTimestamp = getGlobTimestamp();
	}

	// Draw
	m_r->drawQuad(cmdb);

	// Blurring passes
	//
	for(U i = 0; i < m_blurringIterationsCount; i++)
	{
		if(i == 0)
		{
			Array<GlTextureHandle, 2> tarr = {{m_hblurRt, m_vblurRt}};
			cmdb.bindTextures(0, tarr.begin(), tarr.getSize());
		}

		// hpass
		m_hblurFb.bind(cmdb, true);
		m_hblurPpline.bind(cmdb);
		m_r->drawQuad(cmdb);

		// vpass
		m_vblurFb.bind(cmdb, true);
		m_vblurPpline.bind(cmdb);
		m_r->drawQuad(cmdb);
	}
}

} // end namespace anki
