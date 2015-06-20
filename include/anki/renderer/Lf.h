// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"
#include "anki/resource/ShaderResource.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Lens flare rendering pass. Part of forward shading.
class Lf: public RenderingPass
{
public:
	/// @privatesection
	/// @{
	Lf(Renderer* r)
		: RenderingPass(r)
	{}

	~Lf();

	ANKI_USE_RESULT Error init(const ConfigSet& config);

	void runOcclusionTests(CommandBufferPtr& cmdb);

	void run(CommandBufferPtr& cmdb);
	/// @}

private:
	// Occlusion query
	Array<BufferPtr, 3> m_positionsVertBuff;
	BufferPtr m_mvpBuff;
	ShaderResourcePtr m_occlusionVert;
	ShaderResourcePtr m_occlusionFrag;
	PipelinePtr m_occlusionPpline;

	// Sprite billboards
	ShaderResourcePtr m_realVert;
	ShaderResourcePtr m_realFrag;
	PipelinePtr m_realPpline;
	Array<BufferPtr, 3> m_flareDataBuff;
	U32 m_flareSize;
	U8 m_maxSpritesPerFlare;
	U8 m_maxFlares;


	ANKI_USE_RESULT Error initSprite(const ConfigSet& config);
	ANKI_USE_RESULT Error initOcclusion(const ConfigSet& config);

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki

