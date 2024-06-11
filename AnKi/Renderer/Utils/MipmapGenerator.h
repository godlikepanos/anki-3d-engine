// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// @memberof MipmapGenerator
class MipmapGeneratorTargetArguments
{
public:
	RenderTargetHandle m_handle;
	UVec2 m_targetSize = UVec2(0u); ///< Size of the 1st mip
	U32 m_layerCount = 1;
	U8 m_mipmapCount = 0;
	Bool m_isCubeTexture = false;
};

/// Generates a mipmap chain.
class MipmapGenerator : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(const MipmapGeneratorTargetArguments& target, RenderGraphDescription& rgraph, CString passesName = {});

private:
	ShaderProgramResourcePtr m_genMipsProg;
	ShaderProgramPtr m_genMipsGrProg;
};
/// @}

} // end namespace anki
