// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/ShaderImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/ShaderCompiler.h>
#include <anki/util/StringList.h>
#include <anki/util/Logger.h>

#define ANKI_DUMP_SHADERS ANKI_EXTRA_CHECKS

#if ANKI_DUMP_SHADERS
#	include <anki/util/File.h>
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

ShaderImpl::~ShaderImpl()
{
	destroyDeferred(getManager(), deleteShaders);
}

Error ShaderImpl::init(CString source, ConstWeakArray<ShaderSpecializationConstValue> constValues)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(!isCreated());

	static const Array<GLenum, 6> gltype = {{GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
											 GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}};

	m_glType = gltype[U(m_shaderType)];

	// Create a new shader with spec consts if needed
	StringAuto newSrc(getAllocator());
	if(constValues.getSize())
	{
		// Create const str
		StringListAuto constStrLines(getAllocator());
		U count = 0;
		for(const ShaderSpecializationConstValue& constVal : constValues)
		{
			if(constVal.m_dataType == ShaderVariableDataType::INT)
			{
				constStrLines.pushBackSprintf("#define _anki_spec_const_%u %i", count, constVal.m_int);
			}
			else
			{
				ANKI_ASSERT(constVal.m_dataType == ShaderVariableDataType::FLOAT);
				constStrLines.pushBackSprintf("#define _anki_spec_const_%u %f", count, constVal.m_float);
			}

			++count;
		}
		StringAuto constStr(getAllocator());
		constStrLines.join("\n", constStr);

		// Break the old source
		StringListAuto lines(getAllocator());
		lines.splitString(source, '\n');
		ANKI_ASSERT(lines.getFront().find("#version") == 0);
		lines.popFront();

		// Append the const values
		lines.pushFront(constStr.toCString());
		lines.pushFront("#version 450 core");

		// Create the new string
		lines.join("\n", newSrc);
		source = newSrc.toCString();
	}

	// Gen name, create and compile
	const char* sourceStrs[1] = {nullptr};
	sourceStrs[0] = &source[0];
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

		StringAuto fname(getAllocator());
		CString cacheDir = getManager().getCacheDirectory();
		fname.sprintf("%s/%05u.%s", &cacheDir[0], static_cast<U32>(m_glName), ext);

		File file;
		ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));
		ANKI_CHECK(file.writeText("%s", source.cstr()));
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

		ShaderCompiler::logShaderErrorCode(compilerLog.toCString(), source, alloc);

		// Compilation failed, set error anyway
		return Error::USER_DATA;
	}

	return Error::NONE;
}

} // end namespace anki
