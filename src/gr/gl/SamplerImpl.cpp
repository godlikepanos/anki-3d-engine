// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/SamplerImpl.h"
#include "anki/gr/TextureSamplerCommon.h"

namespace anki {

//==============================================================================
Error SamplerImpl::create(const SamplerInitializer& sinit)
{
	glGenSamplers(1, &m_glName);
	ANKI_ASSERT(m_glName);
	
	if(sinit.m_repeat)
	{
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// Set filtering type
	switch(sinit.m_filterType)
	{
	case SamplingFilter::NEAREST:
		glSamplerParameteri(m_glName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(m_glName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case SamplingFilter::LINEAR:
		glSamplerParameteri(m_glName, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(m_glName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case SamplingFilter::TRILINEAR:
		glSamplerParameteri(m_glName, 
			GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(m_glName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

#if ANKI_GL == ANKI_GL_DESKTOP
	if(sinit.m_anisotropyLevel > 1)
	{
		glSamplerParameteri(m_glName, GL_TEXTURE_MAX_ANISOTROPY_EXT, 
			GLint(sinit.m_anisotropyLevel));
	}
#endif
	
	return ErrorCode::NONE;
}

} // end namespace anki
