#include "anki/renderer/MainRenderer.h"
#include "anki/core/Logger.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/File.h"
#include "anki/core/Counters.h"
#include <cstdlib>
#include <cstdio>

#define glewGetContext() (&glContext)

namespace anki {

//==============================================================================
MainRenderer::~MainRenderer()
{}

//==============================================================================
void MainRenderer::init(const RendererInitializer& initializer_)
{
	ANKI_LOGI("Initializing main renderer...");

	RendererInitializer initializer = initializer_;
	initializer.set("offscreen", false);
	initializer.set("width", 
		initializer.get("width") * initializer.get("renderingQuality"));
	initializer.set("height", 
		initializer.get("height") * initializer.get("renderingQuality"));

	initGl();

	Renderer::init(initializer);
	m_deformer.reset(new Deformer);

	m_blitFrag.load("shaders/Final.frag.glsl");
	m_blitPpline = createDrawQuadProgramPipeline(
		m_blitFrag->getGlProgram());

	ANKI_LOGI("Main renderer initialized. Rendering size %dx%d", 
		getWidth(), getHeight());
}

//==============================================================================
void MainRenderer::render(SceneGraph& scene)
{
	ANKI_COUNTER_START_TIMER(MAIN_RENDERER_TIME);

	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl, m_jobsInitHints);

	Renderer::render(scene, jobs);

	Bool notDrawnToDefault = 
		getRenderingQuality() != 1.0 || getDbg().getEnabled();

	if(notDrawnToDefault)
	{
		getDefaultFramebuffer().bind(jobs, false);
		jobs.setViewport(0, 0, getWindowWidth(), getWindowHeight());

		m_blitPpline.bind(jobs);

		GlTextureHandle* rt;

		if(getPps().getEnabled())
		{
			rt = &getPps().getRt();
		}
		else
		{
			rt = &getIs().getRt();
		}

		//rt = &getPps().getHdr().getRt();

		rt->setFilter(jobs, GlTextureHandle::Filter::LINEAR);
		rt->bind(jobs, 0);

		drawQuad(jobs);
	}

	ANKI_ASSERT(jobs.getReferenceCount() == 1);
	jobs.flush();
	m_jobsInitHints = jobs.computeInitHints();

	ANKI_COUNTER_STOP_TIMER_INC(MAIN_RENDERER_TIME);
}

//==============================================================================
void MainRenderer::initGl()
{
	// get max texture units
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	jobs.enableCulling(true);
	jobs.setCullFace(GL_BACK);

	jobs.flush();
}

//==============================================================================
void MainRenderer::takeScreenshotTga(const char* filename)
{
#if 0
	// open file and check
	File fs;
	try
	{
		fs.open(filename, File::OF_WRITE | File::OF_BINARY);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot write screenshot file") << e;
	}

	// write headers
	static const U8 tgaHeaderUncompressed[12] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	Array<U8, 6> header;

	header[1] = getWidth() / 256;
	header[0] = getWidth() % 256;
	header[3] = getHeight() / 256;
	header[2] = getHeight() % 256;
	header[4] = 24;
	header[5] = 0;

	fs.write((char*)tgaHeaderUncompressed, sizeof(tgaHeaderUncompressed));
	fs.write((char*)&header[0], sizeof(header));

	// Create the buffers
	U outBufferSize = getWidth() * getHeight() * 3;
	Vector<U8> outBuffer;
	outBuffer.resize(outBufferSize);

	U rgbaBufferSize = getWidth() * getHeight() * 4;
	Vector<U8> rgbaBuffer;
	rgbaBuffer.resize(rgbaBufferSize);

	// Read pixels
	glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		&rgbaBuffer[0]);

	U j = 0;
	for(U i = 0; i < outBufferSize; i += 3)
	{
		outBuffer[i] = rgbaBuffer[j + 2];
		outBuffer[i + 1] = rgbaBuffer[j + 1];
		outBuffer[i + 2] = rgbaBuffer[j];

		j += 4;
	}

	fs.write((char*)&outBuffer[0], outBufferSize);

	// end
	ANKI_CHECK_GL_ERROR();
#endif
}

//==============================================================================
void MainRenderer::takeScreenshot(const char* filename)
{
	std::string ext = File::getFileExtension(filename);

	// exec from this extension
	if(ext == "tga")
	{
		takeScreenshotTga(filename);
	}
	else
	{
		throw ANKI_EXCEPTION("Unsupported file extension");
	}
	ANKI_LOGI("Screenshot %s saved", filename);
}

} // end namespace anki
