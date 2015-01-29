// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlShader.h"
#include "anki/util/StringList.h"
#include "anki/util/Logger.h"

#define ANKI_DUMP_SHADERS ANKI_DEBUG

#if ANKI_DUMP_SHADERS
#	include "anki/util/File.h"
#endif

namespace anki {

//==============================================================================
Error GlShader::create(GLenum type, const CString& source, 
	GlAllocator<U8>& alloc, const CString& cacheDir)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(!isCreated());

	Error err = ErrorCode::NONE;

	m_type = type;

	// 1) Append some things in the source string
	//
	U32 version;
	{
		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		version = major * 100 + minor * 10;
	}

	String fullSrc;
	String::ScopeDestroyer fullSrcd(&fullSrc, alloc); 
#if ANKI_GL == ANKI_GL_DESKTOP
	err = fullSrc.sprintf(alloc, "#version %d core\n%s\n", version, &source[0]); 
#else
	err = fullSrc.sprintf(alloc, "#version %d es\n%s\n", version, &source[0]);
#endif

	// 2) Gen name, create, compile and link
	//
	if(!err)
	{
		const char* sourceStrs[1] = {nullptr};
		sourceStrs[0] = &fullSrc[0];
		m_glName = glCreateShaderProgramv(m_type, 1, sourceStrs);
		if(m_glName == 0)
		{
			err = ErrorCode::FUNCTION_FAILED;
		}
	}

#if ANKI_DUMP_SHADERS
	if(!err)
	{
		const char* ext;

		switch(m_type)
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

		String fname;
		err = fname.sprintf(alloc,
			"%s/%05u.%s", &cacheDir[0], static_cast<U32>(m_glName), ext);

		if(!err)
		{
			File file;
			err = file.open(fname.toCString(), File::OpenFlag::WRITE);
		}

		fname.destroy(alloc);
	}
#endif
	
	if(!err)
	{
		GLint status = GL_FALSE;
		glGetProgramiv(m_glName, GL_LINK_STATUS, &status);

		if(status == GL_FALSE)
		{
			err = handleError(alloc, fullSrc);

			if(!err)
			{
				// Compilation failed, set error anyway
				err = ErrorCode::USER_DATA;
			}
		}
	}

	return err;
}

//==============================================================================
Error GlShader::handleError(GlAllocator<U8>& alloc, String& src)
{
	Error err = ErrorCode::NONE;

	GLint compilerLogLen = 0;
	GLint charsWritten = 0;
	String compilerLog;
	String prettySrc;
	StringList lines;

	static const char* padding = 
		"======================================="
		"=======================================";

	glGetProgramiv(m_glName, GL_INFO_LOG_LENGTH, &compilerLogLen);

	err = compilerLog.create(alloc, ' ', compilerLogLen + 1);

	if(!err)
	{
		glGetProgramInfoLog(
			m_glName, compilerLogLen, &charsWritten, &compilerLog[0]);
		
		err = lines.splitString(alloc, src.toCString(), '\n');
	}

	I lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd() && !err; ++it)
	{
		String tmp;

		err = tmp.sprintf(alloc, "%4d: %s\n", ++lineno, &(*it)[0]);
		
		if(!err)
		{
			err = prettySrc.append(alloc, tmp);
		}

		tmp.destroy(alloc);
	}

	if(!err)
	{
		ANKI_LOGE("Shader compilation failed (type %x):\n%s\n%s\n%s\n%s",
			m_type, padding, &compilerLog[0], padding, &prettySrc[0]);
	}

	lines.destroy(alloc);
	prettySrc.destroy(alloc);
	compilerLog.destroy(alloc);
	return err;
}

//==============================================================================
void GlShader::destroy()
{
	if(m_glName != 0)
	{
		glDeleteProgram(m_glName);
		m_glName = 0;
	}
}

} // end namespace anki

