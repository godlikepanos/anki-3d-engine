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
void GlState::init0(const ConfigSet& config)
{
	m_dynamicBuffers[BufferUsage::UNIFORM].m_size =
		config.getNumber("gr.uniformPerFrameMemorySize");

	m_dynamicBuffers[BufferUsage::STORAGE].m_size =
		config.getNumber("gr.storagePerFrameMemorySize");

	m_dynamicBuffers[BufferUsage::VERTEX].m_size = 1024;

	m_dynamicBuffers[BufferUsage::TRANSFER].m_size =
		config.getNumber("gr.transferPersistentMemorySize");
}

//==============================================================================
void GlState::init1()
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

	// Init ring buffers
	GLint64 blockAlignment;
	glGetInteger64v(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
	initDynamicBuffer(GL_UNIFORM_BUFFER,
		blockAlignment,
		MAX_UNIFORM_BLOCK_SIZE,
		BufferUsage::UNIFORM);

	glGetInteger64v(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &blockAlignment);
	initDynamicBuffer(GL_SHADER_STORAGE_BUFFER,
		blockAlignment,
		MAX_STORAGE_BLOCK_SIZE,
		BufferUsage::STORAGE);

	initDynamicBuffer(GL_ARRAY_BUFFER, 16, MAX_U32, BufferUsage::VERTEX);

	{
		m_transferBuffer.create(m_manager->getAllocator(),
			m_dynamicBuffers[BufferUsage::TRANSFER].m_size
				/ sizeof(Aligned16Type));

		auto& buff = m_dynamicBuffers[BufferUsage::TRANSFER];
		buff.m_address = reinterpret_cast<U8*>(&m_transferBuffer[0]);
		ANKI_ASSERT(isAligned(ANKI_SAFE_ALIGNMENT, buff.m_address));
		buff.m_alignment = ANKI_SAFE_ALIGNMENT;
		buff.m_maxAllocationSize = MAX_U32;
	}
}

//==============================================================================
void GlState::initDynamicBuffer(
	GLenum target, U32 alignment, U32 maxAllocationSize, BufferUsage usage)
{
	DynamicBuffer& buff = m_dynamicBuffers[usage];
	ANKI_ASSERT(buff.m_size > 0);

	const U FLAGS = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

	alignRoundUp(alignment, buff.m_size);

	glGenBuffers(1, &buff.m_name);

	glBindBuffer(target, buff.m_name);
	glBufferStorage(target, buff.m_size, nullptr, FLAGS);

	buff.m_address =
		static_cast<U8*>(glMapBufferRange(target, 0, buff.m_size, FLAGS));
	buff.m_alignment = alignment;
	buff.m_maxAllocationSize = maxAllocationSize;
}

//==============================================================================
void GlState::checkDynamicMemoryConsumption()
{
	for(BufferUsage usage = BufferUsage::FIRST; usage < BufferUsage::COUNT;
		++usage)
	{
		DynamicBuffer& buff = m_dynamicBuffers[usage];

		if(buff.m_address)
		{
			auto bytesUsed = buff.m_bytesUsed.exchange(0);
			if(bytesUsed >= buff.m_size / MAX_FRAMES_IN_FLIGHT)
			{
				ANKI_LOGW("Using too much dynamic memory (mem_type: %u, "
						  "mem_used: %u). Increase the limit",
					U(usage),
					U(bytesUsed));
			}

			// Increase the counters
			switch(usage)
			{
			case BufferUsage::UNIFORM:
				ANKI_TRACE_INC_COUNTER(GR_DYNAMIC_UNIFORMS_SIZE, bytesUsed);
				break;
			case BufferUsage::STORAGE:
				ANKI_TRACE_INC_COUNTER(GR_DYNAMIC_STORAGE_SIZE, bytesUsed);
				break;
			default:
				break;
			}
		}
	}
}

//==============================================================================
void GlState::destroy()
{
	for(auto& x : m_dynamicBuffers)
	{
		if(x.m_name)
		{
			glDeleteBuffers(1, &x.m_name);
		}
	}

	glDeleteVertexArrays(1, &m_defaultVao);
	m_transferBuffer.destroy(m_manager->getAllocator());
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
