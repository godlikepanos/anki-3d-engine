// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlState.h"
#include "anki/gl/GlBuffer.h"
#include "anki/core/Logger.h"
#include <algorithm>
#include <cstring>

namespace anki {

//==============================================================================
#if ANKI_GL == ANKI_GL_DESKTOP
struct GlDbg
{
	GLenum token;
	const char* str;
};

static const GlDbg gldbgsource[] = {
	{GL_DEBUG_SOURCE_API, "GL_DEBUG_SOURCE_API"},
	{GL_DEBUG_SOURCE_WINDOW_SYSTEM, "GL_DEBUG_SOURCE_WINDOW_SYSTEM"},
	{GL_DEBUG_SOURCE_SHADER_COMPILER, "GL_DEBUG_SOURCE_SHADER_COMPILER"}, 
	{GL_DEBUG_SOURCE_THIRD_PARTY, "GL_DEBUG_SOURCE_THIRD_PARTY"},
	{GL_DEBUG_SOURCE_APPLICATION, "GL_DEBUG_SOURCE_APPLICATION"},
	{GL_DEBUG_SOURCE_OTHER, "GL_DEBUG_SOURCE_OTHER"}};

static const GlDbg gldbgtype[] = {
	{GL_DEBUG_TYPE_ERROR, "GL_DEBUG_TYPE_ERROR"},
	{GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"},
	{GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"},
	{GL_DEBUG_TYPE_PORTABILITY, "GL_DEBUG_TYPE_PORTABILITY"},
	{GL_DEBUG_TYPE_PERFORMANCE, "GL_DEBUG_TYPE_PERFORMANCE"},
	{GL_DEBUG_TYPE_OTHER, "GL_DEBUG_TYPE_OTHER"}};

static const GlDbg gldbgseverity[] = {
	{GL_DEBUG_SEVERITY_LOW, "GL_DEBUG_SEVERITY_LOW"},
	{GL_DEBUG_SEVERITY_MEDIUM, "GL_DEBUG_SEVERITY_MEDIUM"},
	{GL_DEBUG_SEVERITY_HIGH, "GL_DEBUG_SEVERITY_HIGH"}};

//==============================================================================
void oglMessagesCallback(GLenum source,
	GLenum type, GLuint id, GLenum severity, GLsizei length,
	const char* message, GLvoid* userParam)
{
	using namespace anki;

	const GlDbg* sourced = &gldbgsource[0];
	while(source != sourced->token)
	{
		++sourced;
	}

	const GlDbg* typed = &gldbgtype[0];
	while(type != typed->token)
	{
		++typed;
	}

	switch(severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		ANKI_LOGI("GL: %s, %s: %s", sourced->str, typed->str, message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		ANKI_LOGW("GL: %s, %s: %s", sourced->str, typed->str, message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		ANKI_LOGE("GL: %s, %s: %s", sourced->str, typed->str, message);
		break;
	}
}
#endif

//==============================================================================
void GlState::init()
{
	// GL version
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	m_version = major * 100 + minor * 10;

	// Vendor
	String glstr = (const char*)glGetString(GL_VENDOR);
	glstr += (const char*)glGetString(GL_RENDERER);
	std::transform(glstr.begin(), glstr.end(), glstr.begin(), ::tolower);

	if(glstr.find("arm") != String::npos)
	{
		m_gpu = GlGpu::ARM;
	}
	else if(glstr.find("nvidia") != String::npos)
	{
		m_gpu = GlGpu::NVIDIA;
	}

	// Max tex units
	{
		GLint tmp;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp);
		m_texUnitsCount = (U32)tmp;
		ANKI_ASSERT(m_texUnitsCount <= m_texUnits.size());

		memset(&m_texUnits[0], 0, sizeof(m_texUnits));
	}

	// Enable debug messages
#if ANKI_GL == ANKI_GL_DESKTOP
	if(m_registerMessages)
	{
		glDebugMessageCallback(oglMessagesCallback, nullptr);

		for(U s = 0; s < sizeof(gldbgsource) / sizeof(GlDbg); s++)
		{
			for(U t = 0; t < sizeof(gldbgtype) / sizeof(GlDbg); t++)
			{
				for(U sv = 0; sv < sizeof(gldbgseverity) / sizeof(GlDbg); sv++)
				{
					glDebugMessageControl(gldbgsource[s].token, 
						gldbgtype[t].token, gldbgseverity[sv].token, 0, 
						nullptr, GL_TRUE);
				}
			}
		}
	}
#endif

	// Buffer offset alignment
	GLint64 alignment;
	glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
	m_uniBuffOffsetAlignment = alignment;

	glGetInteger64v(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment);
	m_ssBuffOffsetAlignment = alignment;
}

} // end namespace anki


