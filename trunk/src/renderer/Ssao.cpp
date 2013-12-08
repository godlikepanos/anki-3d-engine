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
struct ShaderCommonUniforms
{
	Vec4 nearPlanes;
	Vec4 limitsOfNearPlane;
	Mat4 projectionMatrix;
};

//==============================================================================
// Ssao                                                                        =
//==============================================================================

//==============================================================================
void Ssao::createFbo(Fbo& fbo, Texture& fai, U width, U height)
{
	fai.create2dFai(width, height, GL_RED, GL_RED, GL_UNSIGNED_BYTE);

	// Set to bilinear because the blurring techniques take advantage of that
	fai.setFiltering(Texture::TFT_LINEAR);
	fbo.create({{&fai, GL_COLOR_ATTACHMENT0}});
}

//==============================================================================
void Ssao::initInternal(const RendererInitializer& initializer)
{
	enabled = initializer.get("pps.ssao.enabled");

	if(!enabled)
	{
		return;
	}

	blurringIterationsCount = initializer.get("pps.ssao.blurringIterationsNum");

	//
	// Init the widths/heights
	//
	const F32 quality = initializer.get("pps.ssao.renderingQuality");

	width = quality * (F32)r->getWidth();
	height = quality * (F32)r->getHeight();

	//
	// create FBOs
	//
	createFbo(hblurFbo, hblurFai, width, height);
	createFbo(vblurFbo, vblurFai, width, height);

	//
	// noise texture
	//
	Array<Vec3, NOISE_TEX_SIZE * NOISE_TEX_SIZE> noise;
	Texture::Initializer tinit;

	genNoise(noise.begin(), noise.end());

	tinit.width = tinit.height = NOISE_TEX_SIZE;
	tinit.target = GL_TEXTURE_2D;
	tinit.internalFormat = GL_RGB32F;
	tinit.format = GL_RGB;
	tinit.type = GL_FLOAT;
	tinit.filteringType = Texture::TFT_NEAREST;
	tinit.repeat = true;
	tinit.mipmapsCount = 1;
	tinit.data[0][0] = {&noise[0], sizeof(noise)};

	noiseTex.create(tinit);

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
	commonUbo.create(GL_UNIFORM_BUFFER, sizeof(ShaderCommonUniforms), nullptr,
		GL_DYNAMIC_DRAW);

	std::stringstream pps;

	// main pass prog
	pps << "#define NOISE_MAP_SIZE " << NOISE_TEX_SIZE
		<< "\n#define WIDTH " << width
		<< "\n#define HEIGHT " << height
		<< "\n#define USE_MRT " << r->getUseMrt()
		<< "\n#define KERNEL_SIZE " << KERNEL_SIZE << "U"
		<< "\n#define KERNEL_ARRAY " << kernelStr.str() 
		<< "\n";
	ssaoSProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsSsao.glsl", pps.str().c_str(), "r_").c_str());

	ssaoSProg->findUniformBlock("commonBlock").setBinding(0);

	// blurring progs
	const char* SHADER_FILENAME = "shaders/VariableSamplingBlurGeneric.glsl";

	pps.str("");
	pps << "#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << height << "\n"
		"#define SAMPLES 7\n";
	hblurSProg.load(ShaderProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str(), "r_").c_str());

	pps.str("");
	pps << "#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << width << "\n"
		"#define SAMPLES 7\n";
	vblurSProg.load(ShaderProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str(), "r_").c_str());
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
void Ssao::run()
{
	ANKI_ASSERT(enabled);

	const Camera& cam = r->getSceneGraph().getActiveCamera();

	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_DEPTH_TEST);

	// 1st pass
	//
	vblurFbo.bind(true);
	GlStateSingleton::get().setViewport(0, 0, width, height);
	ssaoSProg->bind();
	commonUbo.setBinding(0);

	// Write common block
	if(commonUboUpdateTimestamp < r->getPlanesUpdateTimestamp()
		|| commonUboUpdateTimestamp < cam.FrustumComponent::getTimestamp()
		|| commonUboUpdateTimestamp == 1)
	{
		ShaderCommonUniforms blk;

		blk.nearPlanes = Vec4(cam.getNear(), cam.getFar(), r->getPlanes().x(),
			r->getPlanes().y());

		blk.limitsOfNearPlane = Vec4(r->getLimitsOfNearPlane(),
			r->getLimitsOfNearPlane2());

		blk.projectionMatrix = cam.getProjectionMatrix().getTransposed();

		commonUbo.write(&blk);
		commonUboUpdateTimestamp = getGlobTimestamp();
	}

	// msDepthFai
	ssaoSProg->findUniformVariable("msDepthFai").set(
		r->getMs().getDepthFai());

	// noiseMap
	ssaoSProg->findUniformVariable("noiseMap").set(noiseTex);

	// msGFai
	if(r->getUseMrt())
	{
		ssaoSProg->findUniformVariable("msGFai").set(r->getMs().getFai1());
	}
	else
	{
		ssaoSProg->findUniformVariable("msGFai").set(r->getMs().getFai0());
	}

	// Draw
	r->drawQuad();

	// Blurring passes
	//
	for(U32 i = 0; i < blurringIterationsCount; i++)
	{
		// hpass
		hblurFbo.bind(true);
		hblurSProg->bind();
		hblurSProg->findUniformVariable("img").set(vblurFai);
		r->drawQuad();

		// vpass
		vblurFbo.bind(true);
		vblurSProg->bind();
		vblurSProg->findUniformVariable("img").set(hblurFai);
		r->drawQuad();
	}
}

} // end namespace anki
