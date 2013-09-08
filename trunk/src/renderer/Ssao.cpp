#include "anki/renderer/Ssao.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/util/Functions.h"

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
void Ssao::createFbo(Fbo& fbo, Texture& fai, F32 width, F32 height)
{
	fai.create2dFai(width, height, GL_RED, GL_RED, GL_UNSIGNED_BYTE);

	// Set to bilinear because the blurring techniques take advantage of that
	fai.setFiltering(Texture::TFT_LINEAR);

	fbo.create();
	fbo.setColorAttachments({&fai});

	if(!fbo.isComplete())
	{
		throw ANKI_EXCEPTION("Fbo not complete");
	}
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
	const F32 bQuality = initializer.get("pps.ssao.blurringRenderingQuality");
	const F32 mpQuality = initializer.get("pps.ssao.mainPassRenderingQuality");

	if(mpQuality > bQuality)
	{
		throw ANKI_EXCEPTION("SSAO blur quality shouldn't be less than "
			"main pass SSAO quality");
	}

	bWidth = bQuality * (F32)r->getWidth();
	bHeight = bQuality * (F32)r->getHeight();
	mpWidth =  mpQuality * (F32)r->getWidth();
	mpHeight =  mpQuality * (F32)r->getHeight();

	//
	// create FBOs
	//
	createFbo(hblurFbo, hblurFai, bWidth, bHeight);
	createFbo(vblurFbo, vblurFai, bWidth, bHeight);

	if(blit())
	{
		createFbo(mpFbo, mpFai, mpWidth, mpHeight);
	}

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
	kernelStr << "const vec3 KERNEL[" << KERNEL_SIZE << "] = vec3[](";
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
			kernelStr << ");";
		}
	}

	//
	// Shaders
	//
	commonUbo.create(sizeof(ShaderCommonUniforms), nullptr);

	std::stringstream pps;

	// main pass prog
	pps << "#define NOISE_MAP_SIZE " << NOISE_TEX_SIZE
		<< "\n#define WIDTH " << mpWidth
		<< "\n#define HEIGHT " << mpHeight
		<< "\n#define USE_MRT " << r->getUseMrt()
		<< "\n#define KERNEL_SIZE " << KERNEL_SIZE
		<< "\n" << kernelStr.str() 
		<< "\n";
	ssaoSProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsSsao.glsl", pps.str().c_str(), "r_").c_str());

	ssaoSProg->findUniformBlock("commonBlock").setBinding(0);

	// blurring progs
	const char* SHADER_FILENAME = "shaders/VariableSamplingBlurGeneric.glsl";

	pps.clear();
	pps << "#define HPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << bHeight << "\n"
		"#define SAMPLES 7\n";
	hblurSProg.load(ShaderProgramResource::createSrcCodeToCache(
		SHADER_FILENAME, pps.str().c_str(), "r_").c_str());

	pps.clear();
	pps << "#define VPASS\n"
		"#define COL_R\n"
		"#define IMG_DIMENSION " << bWidth << "\n"
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
	if(blit())
	{
		mpFbo.bind();
		GlStateSingleton::get().setViewport(0, 0, mpWidth, mpHeight);
	}
	else
	{
		vblurFbo.bind();
		GlStateSingleton::get().setViewport(0, 0, bWidth, bHeight);
	}
	r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
	ssaoSProg->bind();
	commonUbo.setBinding(0);

	// Write common block
	if(commonUboUpdateTimestamp < r->getPlanesUpdateTimestamp()
		|| commonUboUpdateTimestamp < cam.getFrustumableTimestamp()
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

	// Blit from main pass FBO to vertical pass FBO
	if(blit())
	{
		vblurFbo.blit(mpFbo,
			0, 0, mpWidth, mpHeight,
			0, 0, bWidth, bHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);

		// Set the correct viewport
		GlStateSingleton::get().setViewport(0, 0, bWidth, bHeight);
	}

	// Blurring passes
	//
	for(U32 i = 0; i < blurringIterationsCount; i++)
	{
		// hpass
		hblurFbo.bind();
		r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
		hblurSProg->bind();
		if(i == 0)
		{
			hblurSProg->findUniformVariable("img").set(vblurFai);
		}
		r->drawQuad();

		// vpass
		vblurFbo.bind();
		r->clearAfterBindingFbo(GL_COLOR_BUFFER_BIT);
		vblurSProg->bind();
		if(i == 0)
		{
			vblurSProg->findUniformVariable("img").set(hblurFai);
		}
		r->drawQuad();
	}
}

} // end namespace anki
