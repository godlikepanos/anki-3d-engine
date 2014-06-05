// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Ssao.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/util/Functions.h"
#include <sstream>

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
void Ssao::createFb(GlFramebufferHandle & fb, GlTextureHandle& rt)
{
	GlManager& gl = GlManagerSingleton::get();

	m_r->createRenderTarget(m_width, m_height, GL_RED, GL_RED, 
		GL_UNSIGNED_BYTE, 1, rt);

	// Set to bilinear because the blurring techniques take advantage of that
	GlJobChainHandle jobs(&gl);
	rt.setFilter(jobs, GlTextureHandle::Filter::LINEAR);

	// create FB
	fb = GlFramebufferHandle(jobs, {{rt, GL_COLOR_ATTACHMENT0}});

	jobs.flush();
}

//==============================================================================
void Ssao::initInternal(const RendererInitializer& initializer)
{
	m_enabled = initializer.get("pps.ssao.enabled");

	if(!m_enabled)
	{
		return;
	}

	m_blurringIterationsCount = 
		initializer.get("pps.ssao.blurringIterationsCount");

	//
	// Init the widths/heights
	//
	const F32 quality = initializer.get("pps.ssao.renderingQuality");

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
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	GlClientBufferHandle noise(
		jobs, sizeof(Vec3) * NOISE_TEX_SIZE * NOISE_TEX_SIZE, nullptr);

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

	m_noiseTex = GlTextureHandle(jobs, tinit);

	//
	// Kernel
	//
	std::stringstream kernelStr;
	Array<Vec3, KERNEL_SIZE> kernel;

	genKernel(kernel.begin(), kernel.end());
	kernelStr << "vec3[](";
	for(U i = 0; i < kernel.size(); i++)
	{
		kernelStr << "vec3(" << kernel[i].x() << ", " << kernel[i].y()
			<< ", " << kernel[i].z() << ")";

		if(i != kernel.size() - 1)
		{
			kernelStr << ", ";
		}
		else
		{
			kernelStr << ")";
		}
	}

	//
	// Shaders
	//
	m_uniformsBuff = GlBufferHandle(jobs, GL_SHADER_STORAGE_BUFFER, 
		sizeof(ShaderCommonUniforms), GL_DYNAMIC_STORAGE_BIT);

	std::stringstream pps;

	// main pass prog
	pps << "#define NOISE_MAP_SIZE " << NOISE_TEX_SIZE
		<< "\n#define WIDTH " << m_width
		<< "\n#define HEIGHT " << m_height
		<< "\n#define KERNEL_SIZE " << KERNEL_SIZE << "U"
		<< "\n#define KERNEL_ARRAY " << kernelStr.str() 
		<< "\n";

	m_ssaoFrag.load(ProgramResource::createSrcCodeToCache(
		"shaders/PpsSsao.frag.glsl", pps.str().c_str(), "r_").c_str());

	m_ssaoPpline = m_r->createDrawQuadProgramPipeline(
		m_ssaoFrag->getGlProgram());

	// blurring progs
	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	pps.str("");
	pps << "#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << m_height << "\n"
		"#define SAMPLES 7\n";

	m_hblurFrag.load(ProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str(), "r_").c_str());

	m_hblurPpline = m_r->createDrawQuadProgramPipeline(
		m_hblurFrag->getGlProgram());

	pps.str("");
	pps << "#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << m_width << "\n"
		"#define SAMPLES 7\n";

	m_vblurFrag.load(ProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str(), "r_").c_str());

	m_vblurPpline = m_r->createDrawQuadProgramPipeline(
		m_vblurFrag->getGlProgram());

	jobs.flush();
}

//==============================================================================
void Ssao::init(const RendererInitializer& initializer)
{
	try
	{
		initInternal(initializer);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init PPS SSAO") << e;
	}
}

//==============================================================================
void Ssao::run(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(m_enabled);

	const Camera& cam = m_r->getSceneGraph().getActiveCamera();

	jobs.setViewport(0, 0, m_width, m_height);

	// 1st pass
	//
	m_vblurFb.bind(jobs, true);
	m_ssaoPpline.bind(jobs);

	m_uniformsBuff.bindShaderBuffer(jobs, 0);

	jobs.bindTextures(0, {
		m_r->getMs()._getSmallDepthRt(),
		m_r->getMs()._getRt1(),
		m_noiseTex});

	// Write common block
	if(m_commonUboUpdateTimestamp 
			< m_r->getProjectionParametersUpdateTimestamp()
		|| m_commonUboUpdateTimestamp < cam.FrustumComponent::getTimestamp()
		|| m_commonUboUpdateTimestamp == 1)
	{
		GlClientBufferHandle tmpBuff(jobs, sizeof(ShaderCommonUniforms),
			nullptr);

		ShaderCommonUniforms& blk = 
			*((ShaderCommonUniforms*)tmpBuff.getBaseAddress());

		blk.m_projectionParams = m_r->getProjectionParameters();

		blk.m_projectionMatrix = cam.getProjectionMatrix().getTransposed();

		m_uniformsBuff.write(jobs, tmpBuff, 0, 0, tmpBuff.getSize());
		m_commonUboUpdateTimestamp = getGlobTimestamp();
	}

	// Draw
	m_r->drawQuad(jobs);

	// Blurring passes
	//
	for(U i = 0; i < m_blurringIterationsCount; i++)
	{
		if(i == 0)
		{
			jobs.bindTextures(0, {m_hblurRt, m_vblurRt});
		}

		// hpass
		m_hblurFb.bind(jobs, true);
		m_hblurPpline.bind(jobs);
		m_r->drawQuad(jobs);

		// vpass
		m_vblurFb.bind(jobs, true);
		m_vblurPpline.bind(jobs);
		m_r->drawQuad(jobs);
	}
}

} // end namespace anki
