// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Gr.h>

namespace anki {

ANKI_TEST(Gr, TextureBuffer)
{
	ConfigSet cfg(allocAligned, nullptr);
	cfg.setGrValidation(true);

	NativeWindow* win = createWindow(cfg);
	GrManager* gr = createGrManager(&cfg, win);

	{
		const CString shaderSrc = R"(
layout(binding = 0) uniform textureBuffer u_tbuff;
layout(binding = 1) uniform image2D u_img;

void main()
{
	imageStore(u_img, IVec2(0), texelFetch(u_tbuff, I32(gl_GlobalInvocationID.x)));
}
	)";

		ShaderPtr shader = createShader(shaderSrc, ShaderType::COMPUTE, *gr);
	}

	GrManager::deleteInstance(gr);
	NativeWindow::deleteInstance(win);
}

} // end namespace anki
