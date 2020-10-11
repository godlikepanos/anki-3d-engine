// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/SamplerImpl.h>
#include <anki/gr/Texture.h>

namespace anki
{

void SamplerImpl::init(const SamplerInitInfo& sinit)
{
	glGenSamplers(1, &m_glName);
	ANKI_ASSERT(m_glName);

	if(sinit.m_addressing == SamplingAddressing::REPEAT)
	{
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}
	else if(sinit.m_addressing == SamplingAddressing::CLAMP)
	{
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(m_glName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	// Set filtering type
	GLenum minFilter = GL_NONE;
	GLenum magFilter = GL_NONE;
	convertFilter(sinit.m_minMagFilter, sinit.m_mipmapFilter, minFilter, magFilter);
	glSamplerParameteri(m_glName, GL_TEXTURE_MIN_FILTER, minFilter);
	glSamplerParameteri(m_glName, GL_TEXTURE_MAG_FILTER, magFilter);

#if ANKI_GL == ANKI_GL_DESKTOP
	if(sinit.m_anisotropyLevel > 1)
	{
		glSamplerParameteri(m_glName, GL_TEXTURE_MAX_ANISOTROPY_EXT, GLint(sinit.m_anisotropyLevel));
	}
#endif

	if(sinit.m_compareOperation != CompareOperation::ALWAYS)
	{
		glSamplerParameteri(m_glName, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(m_glName, GL_TEXTURE_COMPARE_FUNC, convertCompareOperation(sinit.m_compareOperation));
	}

	glSamplerParameteri(m_glName, GL_TEXTURE_MIN_LOD, sinit.m_minLod);
	glSamplerParameteri(m_glName, GL_TEXTURE_MAX_LOD, sinit.m_maxLod);
}

} // end namespace anki
