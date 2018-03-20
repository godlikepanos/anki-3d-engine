// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

#if ANKI_GL == ANKI_GL_DESKTOP
struct GlDbg
{
	GLenum token;
	const char* str;
};

static const GlDbg gldbgsource[] = {{GL_DEBUG_SOURCE_API, "GL_DEBUG_SOURCE_API"},
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

static const GlDbg gldbgseverity[] = {{GL_DEBUG_SEVERITY_LOW, "GL_DEBUG_SEVERITY_LOW"},
	{GL_DEBUG_SEVERITY_MEDIUM, "GL_DEBUG_SEVERITY_MEDIUM"},
	{GL_DEBUG_SEVERITY_HIGH, "GL_DEBUG_SEVERITY_HIGH"}};

#	if ANKI_OS == ANKI_OS_WINDOWS && ANKI_COMPILER != ANKI_COMPILER_MSVC
__stdcall
#	endif
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
		ANKI_GL_LOGI("GL: %s, %s: %s", sourced->str, typed->str, message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		ANKI_GL_LOGW("GL: %s, %s: %s", sourced->str, typed->str, message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		ANKI_GL_LOGE("GL: %s, %s: %s", sourced->str, typed->str, message);
		break;
	}
}
#endif

void GlState::initMainThread(const ConfigSet& config)
{
	m_registerMessages = config.getNumber("window.debugContext");
}

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
	else if(glstr.find("AMD") != CString::NPOS || glstr.find("ATI") != CString::NPOS)
	{
		m_gpu = GpuVendor::AMD;
	}
	else if(glstr.find("Intel") != CString::NPOS)
	{
		m_gpu = GpuVendor::INTEL;
	}

	ANKI_GL_LOGI("GPU vendor is %s", &GPU_VENDOR_STR[m_gpu][0]);

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
					glDebugMessageControl(
						gldbgsource[s].token, gldbgtype[t].token, gldbgseverity[sv].token, 0, nullptr, GL_TRUE);
				}
			}
		}
	}
#endif
	// Set some GL state
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, MAX_I16, MAX_I16);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	// Create default VAO
	glGenVertexArrays(1, &m_defaultVao);
	glBindVertexArray(m_defaultVao);

	// Enable all attributes
	for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
	{
		glEnableVertexAttribArray(i);
	}

	I64 val;
	glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &val);
	m_uboAlignment = val;

	glGetInteger64v(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &val);
	m_ssboAlignment = val;

	glGetInteger64v(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &val);
	m_tboAlignment = val;

	glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &val);
	m_uniBlockMaxSize = val;

	glGetInteger64v(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &val);
	m_storageBlockMaxSize = val;

	m_tboMaxRange = MAX_U32;

	// Texture buffer textures
	glGenTextures(MAX_DESCRIPTOR_SETS * MAX_TEXTURE_BUFFER_BINDINGS, &m_texBuffTextures[0][0]);
	for(U i = 0; i < MAX_DESCRIPTOR_SETS; ++i)
	{
		for(U j = 0; j < MAX_TEXTURE_BUFFER_BINDINGS; ++j)
		{
			U unit = MAX_TEXTURE_BINDINGS * MAX_DESCRIPTOR_SETS + MAX_TEXTURE_BUFFER_BINDINGS * i + j;
			glActiveTexture(GL_TEXTURE0 + unit);

			glBindTexture(GL_TEXTURE_BUFFER, m_texBuffTextures[i][j]);
		}
	}

	// Get extensions
	GLint extCount = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
	while(extCount--)
	{
		const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, extCount));
		if(CString(ext) == "GL_ARB_shader_ballot")
		{
			ANKI_GL_LOGI("Found GL_ARB_shader_ballot");
			m_extensions |= GlExtensions::ARB_SHADER_BALLOT;
		}
	}
}

void GlState::destroy()
{
	glDeleteVertexArrays(1, &m_defaultVao);
	m_crntProg.reset(nullptr);
}

} // end namespace anki
