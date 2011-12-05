#include "anki/resource/ShaderProgram.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/core/App.h" // To get cache dir
#include "anki/gl/GlException.h"
#include "anki/core/Logger.h"
#include "anki/util/Util.h"
#include "anki/core/Globals.h"
#include "anki/util/Exception.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <fstream>
#include <sstream>


namespace anki {


//==============================================================================

#define SHADER_PROGRAM_EXCEPTION(x) ANKI_EXCEPTION( \
	"Shader program \"" + rsrcFilename + \
	"\": " + x)

const char* ShaderProgram::stdSourceCode =
	"#version 330 core\n"
	//"precision lowp float;\n"
#if defined(NDEBUG)
	"#pragma optimize(on)\n"
	"#pragma debug(off)\n";
#else
	"#pragma optimize(of)\n"
	"#pragma debug(on)\n";
#endif


//==============================================================================
ShaderProgram::~ShaderProgram()
{
	/// @todo add code
}


//==============================================================================
uint ShaderProgram::createAndCompileShader(const char* sourceCode,
	const char* preproc, int type) const
{
	uint glId = 0;
	const char* sourceStrs[2] = {NULL, NULL};

	// create the shader
	glId = glCreateShader(type);

	// attach the source
	sourceStrs[1] = sourceCode;
	sourceStrs[0] = preproc;

	// compile
	glShaderSource(glId, 2, sourceStrs, NULL);
	glCompileShader(glId);

	int success;
	glGetShaderiv(glId, GL_COMPILE_STATUS, &success);

	if(!success)
	{
		// print info log
		int infoLen = 0;
		int charsWritten = 0;
		std::vector<char> infoLog;

		glGetShaderiv(glId, GL_INFO_LOG_LENGTH, &infoLen);
		infoLog.resize(infoLen + 1);
		glGetShaderInfoLog(glId, infoLen, &charsWritten, &infoLog[0]);
		infoLog[charsWritten] = '\0';
		
		const char* shaderType = "*dummy*";
		switch(type)
		{
			case GL_VERTEX_SHADER:
				shaderType = "Vertex shader";
				break;
			case GL_FRAGMENT_SHADER:
				shaderType = "Fragment shader";
				break;
			default:
				ANKI_ASSERT(0); // Not supported
		}
		throw SHADER_PROGRAM_EXCEPTION(shaderType + 
			" compiler error log follows:\n"
			"===================================\n" +
			&infoLog[0] +
			"\n===================================\n" + sourceCode);
	}

	return glId;
}


//==============================================================================
void ShaderProgram::link() const
{
	// link
	glLinkProgram(glId);

	// check if linked correctly
	int success;
	glGetProgramiv(glId, GL_LINK_STATUS, &success);

	if(!success)
	{
		int info_len = 0;
		int charsWritten = 0;
		std::string infoLogTxt;

		glGetProgramiv(glId, GL_INFO_LOG_LENGTH, &info_len);

		infoLogTxt.resize(info_len + 1);
		glGetProgramInfoLog(glId, info_len, &charsWritten, &infoLogTxt[0]);
		throw SHADER_PROGRAM_EXCEPTION("Link error log follows:\n" +
			infoLogTxt);
	}
}


//==============================================================================
void ShaderProgram::getUniAndAttribVars()
{
	int num;
	boost::array<char, 256> name_;
	GLsizei length;
	GLint size;
	GLenum type;

	// attrib locations
	glGetProgramiv(glId, GL_ACTIVE_ATTRIBUTES, &num);
	for(int i = 0; i < num; i++) // loop all attributes
	{
		glGetActiveAttrib(glId, i, sizeof(name_) / sizeof(char), &length,
			&size, &type, &name_[0]);
		name_[length] = '\0';

		// check if its FFP location
		int loc = glGetAttribLocation(glId, &name_[0]);
		if(loc == -1) // if -1 it means that its an FFP var
		{
			ANKI_WARNING("Shader prog: \"" << rsrcFilename <<
				"\": You are using FFP vertex attributes (\"" <<
				&name_[0] << "\")");
			continue;
		}

		ShaderProgramAttributeVariable* var =
			new ShaderProgramAttributeVariable(loc, &name_[0], type, *this);

		vars.push_back(var);
		attribs.push_back(var);
		nameToVar[var->getName().c_str()] = var;
		nameToAttribVar[var->getName().c_str()] = var;
	}

	// uni locations
	glGetProgramiv(glId, GL_ACTIVE_UNIFORMS, &num);
	for(int i = 0; i < num; i++) // loop all uniforms
	{
		glGetActiveUniform(glId, i, sizeof(name_) / sizeof(char), &length,
			&size, &type, &name_[0]);
		name_[length] = '\0';

		// check if its FFP location
		int loc = glGetUniformLocation(glId, &name_[0]);
		if(loc == -1) // if -1 it means that its an FFP var
		{
			ANKI_WARNING("Shader prog: \"" << rsrcFilename <<
				"\": You are using FFP vertex uniforms (\"" <<
				&name_[0] << "\")");
			continue;
		}

		ShaderProgramUniformVariable* var =
			new ShaderProgramUniformVariable(loc, &name_[0], type, *this);

		vars.push_back(var);
		unis.push_back(var);
		nameToVar[var->getName().c_str()] = var;
		nameToUniVar[var->getName().c_str()] = var;
	}
}


//==============================================================================
void ShaderProgram::load(const char* filename)
{
	rsrcFilename = filename;
	ANKI_ASSERT(!isInitialized());

	ShaderProgramPrePreprocessor pars(filename);

	// 1) create and compile the shaders
	std::string preprocSource = stdSourceCode;
	vertShaderGlId = createAndCompileShader(
		pars.getShaderSource(ST_VERTEX).c_str(),
		preprocSource.c_str(),
		GL_VERTEX_SHADER);

	fragShaderGlId = createAndCompileShader(
		pars.getShaderSource(ST_FRAGMENT).c_str(),
		preprocSource.c_str(),
		GL_FRAGMENT_SHADER);

	// 2) create program and attach shaders
	glId = glCreateProgram();
	if(glId == 0)
	{
		throw SHADER_PROGRAM_EXCEPTION("glCreateProgram failed");
	}
	glAttachShader(glId, vertShaderGlId);
	glAttachShader(glId, fragShaderGlId);

	// 3) set the TRFFB varyings
	if(pars.getTranformFeedbackVaryings().size() > 0)
	{
		boost::array<const char*, 128> varsArr;
		for(uint i = 0; i < pars.getTranformFeedbackVaryings().size(); i++)
		{
			varsArr[i] = pars.getTranformFeedbackVaryings()[i].c_str();
		}
		glTransformFeedbackVaryings(glId,
			pars.getTranformFeedbackVaryings().size(), &varsArr[0],
			GL_SEPARATE_ATTRIBS);
	}

	// 4) link
	link();

	// init the rest
	getUniAndAttribVars();
}


//==============================================================================
const ShaderProgramVariable& ShaderProgram::findVariableByName(
	const char* name) const
{
	NameToVarHashMap::const_iterator it = nameToVar.find(name);
	if(it == nameToVar.end())
	{
		throw SHADER_PROGRAM_EXCEPTION("Cannot find variable: " + name);
	}
	return *(it->second);
}


//==============================================================================
const ShaderProgramAttributeVariable&
	ShaderProgram::findAttributeVariableByName(const char* name) const
{
	NameToAttribVarHashMap::const_iterator it = nameToAttribVar.find(name);
	if(it == nameToAttribVar.end())
	{
		throw SHADER_PROGRAM_EXCEPTION("Cannot find attribute loc: " + name);
	}
	return *(it->second);
}


//==============================================================================
const ShaderProgramUniformVariable& ShaderProgram::findUniformVariableByName(
	const char* name) const
{
	NameToUniVarHashMap::const_iterator it = nameToUniVar.find(name);
	if(it == nameToUniVar.end())
	{
		throw SHADER_PROGRAM_EXCEPTION("Cannot find uniform loc: " + name);
	}
	return *(it->second);
}


//==============================================================================
bool ShaderProgram::variableExists(const char* name) const
{
	NameToVarHashMap::const_iterator it = nameToVar.find(name);
	return it != nameToVar.end();
}


//==============================================================================
bool ShaderProgram::uniformVariableExists(const char* name) const
{
	NameToUniVarHashMap::const_iterator it = nameToUniVar.find(name);
	return it != nameToUniVar.end();
}


//==============================================================================
bool ShaderProgram::attributeVariableExists(const char* name) const
{
	NameToAttribVarHashMap::const_iterator it = nameToAttribVar.find(name);
	return it != nameToAttribVar.end();
}


//==============================================================================
std::string ShaderProgram::createSrcCodeToCache(const char* sProgFPathName,
	const char* preAppendedSrcCode)
{
	using namespace boost::filesystem;

	if(strlen(preAppendedSrcCode) < 1)
	{
		return sProgFPathName;
	}

	// Create suffix
	boost::hash<std::string> stringHash;
	std::size_t h = stringHash(preAppendedSrcCode);
	std::string suffix = boost::lexical_cast<std::string>(h);

	//
	path newfPathName = AppSingleton::get().getCachePath() /
		(path(sProgFPathName).filename().string() + "." + suffix);

	if(exists(newfPathName))
	{
		return newfPathName.string();
	}

	std::string src_ = Util::readFile(sProgFPathName);
	std::string src = preAppendedSrcCode + src_;

	std::ofstream f(newfPathName.string().c_str());
	if(!f.is_open())
	{
		throw ANKI_EXCEPTION("Cannot open file for writing \"" +
			newfPathName.string() + "\"");
	}

	f.write(src.c_str(), src.length());

	return newfPathName.string();
}


//==============================================================================
std::ostream& operator<<(std::ostream& s, const ShaderProgram& x)
{
	s << "Variables:\n";
	BOOST_FOREACH(const ShaderProgramVariable& var, x.getVariables())
	{
		s << var.getName() << " " << var.getLocation() << " ";
		if(var.getType() == ShaderProgramVariable::T_ATTRIBUTE)
		{
			s << "attribute";
		}
		else
		{
			s << "uniform";
		}
	}
	return s;
}


} // end namespace
