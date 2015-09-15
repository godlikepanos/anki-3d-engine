// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/ShaderImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/util/StringList.h"
#include "anki/util/Logger.h"

#define ANKI_DUMP_SHADERS ANKI_DEBUG

#if ANKI_DUMP_SHADERS
#	include "anki/util/File.h"
#endif

namespace anki {

//==============================================================================
/// Fake glDeletePrograms because some jenius created conflicting interface
static void deleteProgram(GLsizei n, const GLuint* names)
{
	ANKI_ASSERT(n == 1);
	ANKI_ASSERT(names);
	ANKI_ASSERT(*names > 0);
	glDeleteProgram(*names);
}

//==============================================================================
ShaderImpl::~ShaderImpl()
{
	destroyDeferred(deleteProgram);
}

//==============================================================================
Error ShaderImpl::create(ShaderType type, const CString& source)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(!isCreated());

	static const Array<GLenum, 6> gltype = {{GL_VERTEX_SHADER,
		GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}};

	static const Array<const char*, 6> shaderName = {{"VERTEX_SHADER",
		"TESSELATION_CONTROL_SHADER", "TESSELATION_EVALUATION_SHADER",
		"GEOMETRY_SHADER", "FRAGMENT_SHADER", "COMPUTE_SHADER"}};

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

	fullSrc.sprintf("#version %d %s\n#define %s\n%s\n", version, versionType,
		shaderName[U(type)], &source[0]);

	// 2) Gen name, create, compile and link
	//
	const char* sourceStrs[1] = {nullptr};
	sourceStrs[0] = &fullSrc[0];
	m_glName = glCreateShaderProgramv(m_glType, 1, sourceStrs);
	if(m_glName == 0)
	{
		return ErrorCode::FUNCTION_FAILED;
	}

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
		fname.sprintf("%s/%05u.%s", &cacheDir[0],
			static_cast<U32>(m_glName), ext);

		File file;
		ANKI_CHECK(file.open(fname.toCString(), File::OpenFlag::WRITE));
		ANKI_CHECK(file.writeText("%s", &fname[0]));
	}
#endif

	GLint status = GL_FALSE;
	glGetProgramiv(m_glName, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		handleError(fullSrc);
		// Compilation failed, set error anyway
		return ErrorCode::USER_DATA;
	}

	return ErrorCode::NONE;
}

//==============================================================================
void ShaderImpl::handleError(String& src)
{
	auto alloc = getAllocator();
	GLint compilerLogLen = 0;
	GLint charsWritten = 0;
	String compilerLog;
	String prettySrc;
	StringList lines;

	static const char* padding =
		"======================================="
		"=======================================";

	glGetProgramiv(m_glName, GL_INFO_LOG_LENGTH, &compilerLogLen);

	compilerLog.create(alloc, ' ', compilerLogLen + 1);

	glGetProgramInfoLog(
		m_glName, compilerLogLen, &charsWritten, &compilerLog[0]);

	lines.splitString(alloc, src.toCString(), '\n');

	I lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd(); ++it)
	{
		String tmp;

		tmp.sprintf(alloc, "%4d: %s\n", ++lineno, &(*it)[0]);
		prettySrc.append(alloc, tmp);
		tmp.destroy(alloc);
	}

	ANKI_LOGE("Shader compilation failed (type %x):\n%s\n%s\n%s\n%s",
		m_glType, padding, &compilerLog[0], padding, &prettySrc[0]);

	lines.destroy(alloc);
	prettySrc.destroy(alloc);
	compilerLog.destroy(alloc);
}

} // end namespace anki

