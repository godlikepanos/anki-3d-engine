// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/GlState.h"
#include "anki/gr/gl/BufferImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/util/Logger.h"
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
#if ANKI_OS == ANKI_OS_WINDOWS
__stdcall
#endif
void oglMessagesCallback(GLenum source,
	GLenum type, GLuint id, GLenum severity, GLsizei length,
	const char* message, const GLvoid* userParam)
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

	// Set some GL state
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Other
	memset(&m_vertexBindingStrides[0], 0, sizeof(m_vertexBindingStrides));
	initGlobalUbo();
}

//==============================================================================
void GlState::initGlobalUbo()
{
	ANKI_ASSERT(m_globalUboSize > 0);
	auto alloc = m_manager->getAllocator();

	m_globalUboSize = getAlignedRoundUp(MAX_UBO_SIZE, m_globalUboSize);
	U ubosCount = m_globalUboSize / MAX_UBO_SIZE;

	m_globalUbos.create(alloc, ubosCount);
	m_globalUboAddresses.create(alloc, ubosCount);

	for(U i = 0; i < ubosCount; ++i)
	{
		const U FLAGS = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

		GLuint name;
		glGenBuffers(1, &name);

		glBindBuffer(GL_UNIFORM_BUFFER, name);
		glBufferStorage(GL_UNIFORM_BUFFER, MAX_UBO_SIZE, nullptr, FLAGS);

		m_globalUbos[i] = name;

		m_globalUboAddresses[i] = static_cast<U8*>(
			glMapBufferRange(GL_UNIFORM_BUFFER, 0, MAX_UBO_SIZE, FLAGS));
	}
}

//==============================================================================
void GlState::destroy()
{
	for(GLuint name : m_globalUbos)
	{
		glDeleteBuffers(1, &name);
	}

	auto alloc = m_manager->getAllocator();
	m_globalUbos.destroy(alloc);
	m_globalUboAddresses.destroy(alloc);
}

} // end namespace anki

