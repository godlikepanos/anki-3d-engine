// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderingPass.h>

namespace anki {

// Forward
struct ShaderReflectionProbe;

/// @addtogroup renderer
/// @{

/// Image based reflections.
class Ir: public RenderingPass
{
anki_internal:
	Ir(Renderer* r)
		: RenderingPass(r)
	{}

	~Ir();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(CommandBufferPtr cmdb);

	TexturePtr getCubemapArray() const
	{
		return m_cubemapArr;
	}

private:
	Renderer m_nestedR;
	TexturePtr m_cubemapArr;
	U16 m_cubemapArrSize = 0;
	U16 m_fbSize = 0;
	DynamicBufferToken m_probesToken;

	ANKI_USE_RESULT Error renderReflection(SceneNode& node,
		ShaderReflectionProbe& shaderProb);
};
/// @}

} // end namespace anki

