// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/MainRenderer.h"
#include "anki/util/Logger.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/File.h"
#include "anki/util/Filesystem.h"
#include "anki/core/Counters.h"
#include "anki/core/App.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
MainRenderer::MainRenderer()	
{}

//==============================================================================
MainRenderer::~MainRenderer()
{}

//==============================================================================
Error MainRenderer::create(
	Threadpool* threadpool, 
	ResourceManager* resources,
	GrManager* gl,
	HeapAllocator<U8>& alloc,
	const ConfigSet& config,
	const Timestamp* globalTimestamp)
{
	ANKI_LOGI("Initializing main renderer...");

	ANKI_CHECK(Renderer::init(threadpool, resources, gl, alloc, config, 
		globalTimestamp));

	ANKI_CHECK(initGl());

	ANKI_CHECK(
		m_blitFrag.load("shaders/Final.frag.glsl", &_getResourceManager()));

	ANKI_CHECK(createDrawQuadPipeline(
		m_blitFrag->getGrShader(), m_blitPpline));

	ANKI_LOGI("Main renderer initialized. Rendering size %dx%d", 
		getWidth(), getHeight());

	return ErrorCode::NONE;
}

//==============================================================================
Error MainRenderer::render(SceneGraph& scene)
{
	ANKI_COUNTER_START_TIMER(MAIN_RENDERER_TIME);

	GrManager& gl = _getGrManager();
	Array<CommandBufferHandle, JOB_CHAINS_COUNT> jobs;
	CommandBufferHandle& lastJobs = jobs[JOB_CHAINS_COUNT - 1];

	for(U i = 0; i < JOB_CHAINS_COUNT; i++)
	{
		ANKI_CHECK(jobs[i].create(&gl, m_jobsInitHints[i]));
	}

	ANKI_CHECK(Renderer::render(scene, jobs));

#if 1
	Bool alreadyDrawnToDefault = 
		!getDbg().getEnabled()
		&& getRenderingQuality() == 1.0;
#else
	const Bool alreadyDrawnToDefault = false;
	ANKI_LOGW("Have a debug code enabled");
#endif

	if(!alreadyDrawnToDefault)
	{
		getDefaultFramebuffer().bind(lastJobs);
		lastJobs.setViewport(0, 0, 
			getDefaultFramebufferWidth(), getDefaultFramebufferHeight());

		m_blitPpline.bind(lastJobs);

		TextureHandle* rt;

		if(getPps().getEnabled())
		{
			rt = &getPps()._getRt();
		}
		else
		{
			rt = &getIs()._getRt();
		}

		//rt = &getIs()._getRt();
		//rt = &getPps().getHdr()._getRt();

		rt->bind(lastJobs, 0);

		drawQuad(lastJobs);
	}

	// Set the hints
	for(U i = 0; i < JOB_CHAINS_COUNT; i++)
	{
		m_jobsInitHints[i] = jobs[i].computeInitHints();	
	}

	// Flush the last job chain
	//ANKI_ASSERT(lastJobs.get().getReferenceCount() == 1);
	lastJobs.flush();

	ANKI_COUNTER_STOP_TIMER_INC(MAIN_RENDERER_TIME);

	return ErrorCode::NONE;
}

//==============================================================================
Error MainRenderer::initGl()
{
	// get max texture units
	CommandBufferHandle cmdb;
	Error err = cmdb.create(&_getGrManager());

	if(!err)
	{
		cmdb.enableCulling(true);
		cmdb.setCullFace(GL_BACK);
		cmdb.enablePointSize(true);

		cmdb.flush();
	}

	return err;
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
#if 0
	String ext = getFileExtension(filename, _getAllocator());

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
#endif
}

} // end namespace anki
