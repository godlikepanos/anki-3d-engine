// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_SAMPLER_IMPL_H
#define ANKI_GR_GL_SAMPLER_IMPL_H

#include "anki/gr/gl/GlObject.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Sampler GL object.
class SamplerImpl: public GlObject
{
public:
	using Base = GlObject;

	SamplerImpl(GrManager* manager)
	:	Base(manager)
	{}

	~SamplerImpl()
	{
		destroy();
	}

	ANKI_USE_RESULT Error create(const SamplerInitializer& sinit);

	/// Bind the texture to a specified unit
	void bind(U32 unit) const
	{
		ANKI_ASSERT(isCreated());
		glBindSampler(unit, m_glName);
	}

	/// Unbind sampler from unit
	static void unbind(U32 unit)
	{
		glBindSampler(unit, 0);
	}

private:
	void destroy()
	{
		if(m_glName)
		{
			glDeleteSamplers(1, &m_glName);
			m_glName = 0;
		}
	}
};
/// @}

} // end namespace anki

#endif
