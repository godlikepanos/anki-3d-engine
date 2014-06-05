// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_MAIN_RENDERER_H
#define ANKI_RENDERER_MAIN_RENDERER_H

#include "anki/renderer/Renderer.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/Singleton.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Main onscreen renderer
class MainRenderer: public Renderer
{
public:
	MainRenderer()
	{}

	~MainRenderer();

	/// The same as Renderer::init but with additional initialization.
	/// @see Renderer::init
	void init(const RendererInitializer& initializer);

	void render(SceneGraph& scene);

	/// Save the color buffer to a tga (lossless & uncompressed & slow)
	/// or jpeg (lossy & compressed fast)
	/// @param filename The file to save
	void takeScreenshot(const char* filename);

private:
	std::unique_ptr<Deformer> m_deformer;
	ProgramResourcePointer m_blitFrag;
	GlProgramPipelineHandle m_blitPpline;

	/// Optimize job chain
	Array<GlJobChainInitHints, JOB_CHAINS_COUNT> m_jobsInitHints; 

	void takeScreenshotTga(const char* filename);
	void initGl();
};

typedef Singleton<MainRenderer> MainRendererSingleton;

/// @}

} // end namespace anki

#endif
