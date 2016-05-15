// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/GlState.h>
#include <anki/gr/gl/BufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/util/Logger.h>
#include <anki/core/Trace.h>
#include <anki/misc/ConfigSet.h>
#include <algorithm>
#include <cstring>

namespace anki
{

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

static const GlDbg gldbgtype[] = {{GL_DEBUG_TYPE_ERROR, "GL_DEBUG_TYPE_ERROR"},
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
#if ANKI_OS == ANKI_OS_WINDOWS
__stdcall
#endif
	void
	oglMessagesCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const GLvoid* userParam)
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
void GlState::initMainThread(const ConfigSet& config)
{
	m_registerMessages = config.getNumber("debugContext");
}

//==============================================================================
void GlState::initRenderThread()
{
	// GL version
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	m_version = major * 100 + minor * 10;

	// Vendor
	CString glstr = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

	if(glstr.find("ARM") != CString::NPOS)
	{
		m_gpu = GpuVendor::ARM;
	}
	else if(glstr.find("NVIDIA") != CString::NPOS)
	{
		m_gpu = GpuVendor::NVIDIA;
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
						gldbgtype[t].token,
						gldbgseverity[sv].token,
						0,
						nullptr,
						GL_TRUE);
				}
			}
		}
	}
#endif
	// Set some GL state
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Create default VAO
	glGenVertexArrays(1, &m_defaultVao);
	glBindVertexArray(m_defaultVao);

	// Enable all attributes
	for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
	{
		glEnableVertexAttribArray(i);
	}

	// Other
	memset(&m_vertexBindingStrides[0], 0, sizeof(m_vertexBindingStrides));
}

//==============================================================================
void GlState::destroy()
{
	glDeleteVertexArrays(1, &m_defaultVao);
}

//==============================================================================
void GlState::flushVertexState()
{
	if(m_vertBindingsDirty)
	{
		m_vertBindingsDirty = false;

		glBindVertexBuffers(0,
			m_vertBindingCount,
			&m_vertBuffNames[0],
			&m_vertBuffOffsets[0],
			&m_vertexBindingStrides[0]);
	}
}

} // end namespace anki
