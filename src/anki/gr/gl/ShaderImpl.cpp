// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/ShaderImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/common/Misc.h>
#include <anki/util/StringList.h>
#include <anki/util/Logger.h>

#define ANKI_DUMP_SHADERS ANKI_EXTRA_CHECKS

#if ANKI_DUMP_SHADERS
#include <anki/util/File.h>
#endif

namespace anki
{

/// Fake glDeleteShaders because some jenius created a conflicting interface
static void deleteShaders(GLsizei n, const GLuint* names)
{
	ANKI_ASSERT(n == 1);
	ANKI_ASSERT(names);
	glDeleteShader(*names);
}

static const char* SHADER_HEADER = R"(#version %u %s
#define ANKI_GL 1
#define ANKI_VENDOR_%s
#define %s
#define ANKI_UBO_BINDING(set_, binding_) binding = set_ * %u + binding_
#define ANKI_SS_BINDING(set_, binding_) binding = set_ * %u + binding_
#define ANKI_TEX_BINDING(set_, binding_) binding = set_ * %u + binding_
#define ANKI_IMAGE_BINDING(set_, binding_) binding = set_ * %u + binding_
#define ANKI_TBO_BINDING(set_, binding_) binding = %u + set_ * %u + binding_

#if defined(FRAGMENT_SHADER)
#define ANKI_USING_FRAG_COORD(height_) vec4 anki_fragCoord = gl_FragCoord;
#endif

#if defined(VERTEX_SHADER)
#define ANKI_WRITE_POSITION(x_) gl_Position = x_
#endif

%s)";

ShaderImpl::~ShaderImpl()
{
	destroyDeferred(deleteShaders);
}

Error ShaderImpl::init(ShaderType type, const CString& source)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(!isCreated());

	static const Array<GLenum, 6> gltype = {{GL_VERTEX_SHADER,
		GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER,
		GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER,
		GL_COMPUTE_SHADER}};

	static const Array<const char*, 6> shaderName = {{"VERTEX_SHADER",
		"TESSELATION_CONTROL_SHADER",
		"TESSELATION_EVALUATION_SHADER",
		"GEOMETRY_SHADER",
		"FRAGMENT_SHADER",
		"COMPUTE_SHADER"}};

	m_type = type;
	m_glType = gltype[U(type)];

	// 1) Append some things in the source string
	//
	U32 version;
	{
		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		version = major * 100 + minor * 10;
	}

	auto alloc = getAllocator();
	StringAuto fullSrc(alloc);
#if ANKI_GL == ANKI_GL_DESKTOP
	static const char* versionType = "core";
#else
	static const char* versionType = "es";
#endif

	fullSrc.sprintf(SHADER_HEADER,
		version,
		versionType,
		&GPU_VENDOR_STR[getManager().getImplementation().getState().m_gpu][0],
		shaderName[U(type)],
		MAX_UNIFORM_BUFFER_BINDINGS,
		MAX_STORAGE_BUFFER_BINDINGS,
		MAX_TEXTURE_BINDINGS,
		MAX_IMAGE_BINDINGS,
		MAX_TEXTURE_BINDINGS * MAX_DESCRIPTOR_SETS,
		MAX_TEXTURE_BUFFER_BINDINGS,
		&source[0]);

	// 2) Gen name, create, compile and link
	//
	const char* sourceStrs[1] = {nullptr};
	sourceStrs[0] = &fullSrc[0];
	m_glName = glCreateShader(m_glType);
	glShaderSource(m_glName, 1, sourceStrs, NULL);
	glCompileShader(m_glName);

#if ANKI_DUMP_SHADERS
	{
		const char* ext;

		switch(m_glType)
		{
		case GL_VERTEX_SHADER:
			ext = "vert";
			break;
		case GL_TESS_CONTROL_SHADER:
			ext = "tesc";
			break;
		case GL_TESS_EVALUATION_SHADER:
			ext = "tese";
			break;
		case GL_GEOMETRY_SHADER:
			ext = "geom";
			break;
		case GL_FRAGMENT_SHADER:
			ext = "frag";
			break;
		case GL_COMPUTE_SHADER:
			ext = "comp";
			break;
		default:
			ext = nullptr;
			ANKI_ASSERT(0);
		}

		StringAuto fname(alloc);
		CString cacheDir = m_manager->getCacheDirectory();
		fname.sprintf("%s/%05u.%s", &cacheDir[0], static_cast<U32>(m_glName), ext);

		File file;
		ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));
		ANKI_CHECK(file.writeText("%s", &fullSrc[0]));
	}
#endif

	GLint status = GL_FALSE;
	glGetShaderiv(m_glName, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		auto alloc = getAllocator();
		StringAuto compilerLog(alloc);
		GLint compilerLogLen = 0;
		GLint charsWritten = 0;

		glGetShaderiv(m_glName, GL_INFO_LOG_LENGTH, &compilerLogLen);
		compilerLog.create(' ', compilerLogLen + 1);

		glGetShaderInfoLog(m_glName, compilerLogLen, &charsWritten, &compilerLog[0]);

		logShaderErrorCode(compilerLog.toCString(), fullSrc.toCString(), alloc);

		// Compilation failed, set error anyway
		return ErrorCode::USER_DATA;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
