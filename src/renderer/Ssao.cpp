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
Error Ssao::createFb(GlFramebufferHandle& fb, GlTextureHandle& rt)
{
	Error err = ErrorCode::NONE;

	err = m_r->createRenderTarget(m_width, m_height, GL_R8, GL_RED, 
		GL_UNSIGNED_BYTE, 1, rt);
	if(err) return err;

	// Set to bilinear because the blurring techniques take advantage of that
	GlCommandBufferHandle cmdBuff;
	err = cmdBuff.create(&getGlDevice());
	if(err) return err;
	rt.setFilter(cmdBuff, GlTextureHandle::Filter::LINEAR);

	// create FB
	err = fb.create(cmdBuff, {{rt, GL_COLOR_ATTACHMENT0}});
	if(err) return err;

	cmdBuff.flush();

	return err;
}

//==============================================================================
Error Ssao::initInternal(const ConfigSet& config)
{
	Error err = ErrorCode::NONE;

	m_enabled = config.get("pps.ssao.enabled");

	if(!m_enabled)
	{
		return err;
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
	err = createFb(m_hblurFb, m_hblurRt);
	if(err) return err;
	err = createFb(m_vblurFb, m_vblurRt);
	if(err) return err;

	//
	// noise texture
	//
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&getGlDevice());
	if(err) return err;

	GlClientBufferHandle noise;
	err = noise.create(
		cmdb, sizeof(Vec3) * NOISE_TEX_SIZE * NOISE_TEX_SIZE, nullptr);
	if(err) return err;

	genNoise((Vec3*)noise.getBaseAddress(), 
		(Vec3*)((U8*)noise.getBaseAddress() + noise.getSize()));

	GlTextureHandle::Initializer tinit;

	tinit.m_width = tinit.m_height = NOISE_TEX_SIZE;
	tinit.m_target = GL_TEXTURE_2D;
	tinit.m_internalFormat = GL_RGB32F;
	tinit.m_filterType = GlTextureHandle::Filter::NEAREST;
	tinit.m_repeat = true;
	tinit.m_mipmapsCount = 1;
	tinit.m_data[0][0] = noise;

	m_noiseTex.create(cmdb, tinit);

	//
	// Kernel
	//
	String kernelStr;
	String::ScopeDestroyer kernelStrd(&kernelStr, getAllocator());
	Array<Vec3, KERNEL_SIZE> kernel;

	genKernel(kernel.begin(), kernel.end());
	err = kernelStr.create(getAllocator(), "vec3[](");
	if(err) return err;
	for(U i = 0; i < kernel.size(); i++)
	{
		String tmp;
		String::ScopeDestroyer tmpd(&tmp, getAllocator());

		err = tmp.sprintf(getAllocator(),
			"vec3(%f, %f, %f) %s",
			kernel[i].x(), kernel[i].y(), kernel[i].z(),
			(i != kernel.size() - 1) ? ", " : ")");
		if(err) return err;

		err = kernelStr.append(getAllocator(), tmp);
		if(err) return err;
	}

	//
	// Shaders
	//
	err = m_uniformsBuff.create(cmdb, GL_SHADER_STORAGE_BUFFER, 
		sizeof(ShaderCommonUniforms), GL_DYNAMIC_STORAGE_BIT);
	if(err) return err;

	String pps;
	String::ScopeDestroyer ppsd(&pps, getAllocator());

	// main pass prog
	err = pps.sprintf(getAllocator(),
		"#define NOISE_MAP_SIZE %u\n"
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n"
		"#define KERNEL_SIZE %u\n"
		"#define KERNEL_ARRAY %s\n",
		NOISE_TEX_SIZE, m_width, m_height, KERNEL_SIZE, &kernelStr[0]);
	if(err) return err;

	err = m_ssaoFrag.loadToCache(&getResourceManager(),
		"shaders/PpsSsao.frag.glsl", pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadPipeline(
		m_ssaoFrag->getGlProgram(), m_ssaoPpline);
	if(err) return err;

	// blurring progs
	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	pps.destroy(getAllocator());
	err = pps.sprintf(getAllocator(),
		"#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 7\n", 
		m_height);
	if(err) return err;

	err = m_hblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadPipeline(
		m_hblurFrag->getGlProgram(), m_hblurPpline);
	if(err) return err;

	pps.destroy(getAllocator());
	err = pps.sprintf(getAllocator(),
		"#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 7\n", 
		m_width);
	if(err) return err;

	err = m_vblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadPipeline(
		m_vblurFrag->getGlProgram(), m_vblurPpline);
	if(err) return err;

	cmdb.flush();

	return err;
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
Error Ssao::run(GlCommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

	const Camera& cam = m_r->getSceneGraph().getActiveCamera();

	cmdb.setViewport(0, 0, m_width, m_height);

	// 1st pass
	//
	m_vblurFb.bind(cmdb, true);
	m_ssaoPpline.bind(cmdb);

	m_uniformsBuff.bindShaderBuffer(cmdb, 0);

	Array<GlTextureHandle, 3> tarr = {{
		m_r->getDp().getSmallDepthRt(),
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
		err = tmpBuff.create(cmdb, sizeof(ShaderCommonUniforms),
			nullptr);
		if(err) return err;

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

	return err;
}

} // end namespace anki
