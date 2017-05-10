// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Temporal AA resolve.
class Taa : public RenderingPass
{
public:
	Taa(Renderer* r);

	~Taa();

	TexturePtr getRt() const;

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void setPreRunBarriers(RenderingContext& ctx);
	void run(RenderingContext& ctx);
	void setPostRunBarriers(RenderingContext& ctx);

private:
	Array<TexturePtr, 2> m_rts;
	Array<FramebufferPtr, 2> m_fbs;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
};
/// @}

} // end namespace anki
