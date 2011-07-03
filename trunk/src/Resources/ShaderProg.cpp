#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <fstream>
#include <sstream>
#include "Resources/ShaderProg.h"
#include "ShaderPrePreprocessor.h"
#include "Core/App.h" // To get cache dir
#include "GfxApi/GlException.h"
#include "Core/Logger.h"
#include "Util/Util.h"
#include "Core/Globals.h"


#define SPROG_EXCEPTION(x) EXCEPTION("Shader prog \"" + rsrcFilename + "\": " + x)


//==============================================================================
// Statics                                                                     =
//==============================================================================

std::string ShaderProg::stdSourceCode(
	"#version 330 core\n"
	"precision lowp float;\n"
	"#pragma optimize(on)\n"
	"#pragma debug(off)\n"
);


//==============================================================================
// createAndCompileShader                                                      =
//==============================================================================
uint ShaderProg::createAndCompileShader(const char* sourceCode, const char* preproc, int type) const
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
		std::string infoLog;

		glGetShaderiv(glId, GL_INFO_LOG_LENGTH, &infoLen);
		infoLog.resize(infoLen + 1);
		glGetShaderInfoLog(glId, infoLen, &charsWritten, &infoLog[0]);
		
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
				ASSERT(0); // Not supported
		}
		throw SPROG_EXCEPTION(shaderType + " compiler error log follows:\n" + infoLog);
	}

	return glId;
}


//==============================================================================
// link                                                                        =
//==============================================================================
void ShaderProg::link() const
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
		throw SPROG_EXCEPTION("Link error log follows:\n" + infoLogTxt);
	}
}


//==============================================================================
// getUniAndAttribVars                                                         =
//==============================================================================
void ShaderProg::getUniAndAttribVars()
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
		glGetActiveAttrib(glId, i, sizeof(name_) / sizeof(char), &length, &size, &type, &name_[0]);
		name_[length] = '\0';

		// check if its FFP location
		int loc = glGetAttribLocation(glId, &name_[0]);
		if(loc == -1) // if -1 it means that its an FFP var
		{
			WARNING("Shader prog: \"" << rsrcFilename << "\": You are using FFP vertex attributes (\"" <<
			        &name_[0] << "\")");
			continue;
		}

		SProgAttribVar* var = new SProgAttribVar(loc, &name_[0], type, *this);
		attribVars.push_back(var);
		attribNameToVar[attribVars.back().getName().c_str()] = var;
	}


	// uni locations
	glGetProgramiv(glId, GL_ACTIVE_UNIFORMS, &num);
	uniVars.reserve(num);
	for(int i=0; i<num; i++) // loop all uniforms
	{
		glGetActiveUniform(glId, i, sizeof(name_) / sizeof(char), &length, &size, &type, &name_[0]);
		name_[length] = '\0';

		// check if its FFP location
		int loc = glGetUniformLocation(glId, &name_[0]);
		if(loc == -1) // if -1 it means that its an FFP var
		{
			WARNING("Shader prog: \"" << rsrcFilename << "\": You are using FFP vertex uniforms (\"" <<
			        &name_[0] << "\")");
			continue;
		}

		SProgUniVar* var = new SProgUniVar(loc, &name_[0], type, *this);
		uniVars.push_back(var);
		uniNameToVar[uniVars.back().getName().c_str()] = var;
	}
}


//==============================================================================
// bindCustomAttribLocs                                                        =
//==============================================================================
void ShaderProg::bindCustomAttribLocs(const ShaderPrePreprocessor& pars) const
{
	for(uint i=0; i<pars.getOutput().getAttribLocs().size(); ++i)
	{
		const std::string& name = pars.getOutput().getAttribLocs()[i].name;
		int loc = pars.getOutput().getAttribLocs()[i].customLoc;
		glBindAttribLocation(glId, loc, name.c_str());

		// check for error
		try
		{
			ON_GL_FAIL_THROW_EXCEPTION();
		}
		catch(std::exception& e)
		{
			throw SPROG_EXCEPTION("Something went wrong for attrib \"" + name + "\" and location " +
			                      boost::lexical_cast<std::string>(loc) + ": " + e.what());
		}
	}
}


//==============================================================================
// load                                                                        =
//==============================================================================
void ShaderProg::load(const char* filename)
{
	rsrcFilename = filename;
	ASSERT(glId == std::numeric_limits<uint>::max());

	ShaderPrePreprocessor pars(filename);

	// 1) create and compile the shaders
	std::string preprocSource = stdSourceCode;
	vertShaderGlId = createAndCompileShader(pars.getOutput().getVertShaderSource().c_str(), preprocSource.c_str(),
	                                        GL_VERTEX_SHADER);

	fragShaderGlId = createAndCompileShader(pars.getOutput().getFragShaderSource().c_str(), preprocSource.c_str(),
	                                        GL_FRAGMENT_SHADER);

	// 2) create program and attach shaders
	glId = glCreateProgram();
	if(glId == 0)
	{
		throw SPROG_EXCEPTION("glCreateProgram failed");
	}
	glAttachShader(glId, vertShaderGlId);
	glAttachShader(glId, fragShaderGlId);

	// 3) bind the custom attrib locs
	bindCustomAttribLocs(pars);

	// 5) set the TRFFB varyings
	if(pars.getOutput().getTrffbVaryings().size() > 0)
	{
		boost::array<const char*, 128> varsArr;
		for(uint i=0; i<pars.getOutput().getTrffbVaryings().size(); i++)
		{
			varsArr[i] = pars.getOutput().getTrffbVaryings()[i].name.c_str();
		}
		glTransformFeedbackVaryings(glId, pars.getOutput().getTrffbVaryings().size(), &varsArr[0], GL_SEPARATE_ATTRIBS);
	}

	// 6) link
	link();
	
	/*if(pars.getOutput().getTrffbVaryings().size() > 0)
	{
		char name[128] = {'\0'};
		GLsizei size;
		GLsizei len;
		GLenum type;
		glGetTransformFeedbackVarying(glId, 0, 128, &len, &size, &type, name);
		INFO(name);
	}*/

	// init the rest
	getUniAndAttribVars();
}


//==============================================================================
// findUniVar                                                                  =
//==============================================================================
const SProgUniVar* ShaderProg::findUniVar(const char* name) const
{
	NameToSProgUniVarIterator it = uniNameToVar.find(name);
	if(it == uniNameToVar.end())
	{
		throw SPROG_EXCEPTION("Cannot get uniform loc \"" + name + '\"');
	}
	return it->second;
}


//==============================================================================
// findAttribVar                                                               =
//==============================================================================
const SProgAttribVar* ShaderProg::findAttribVar(const char* name) const
{
	NameToSProgAttribVarIterator it = attribNameToVar.find(name);
	if(it == attribNameToVar.end())
	{
		throw SPROG_EXCEPTION("Cannot get attribute loc \"" + name + '\"');
	}
	return it->second;
}


//==============================================================================
// uniVarExists                                                                =
//==============================================================================
bool ShaderProg::uniVarExists(const char* name) const
{
	NameToSProgUniVarIterator it = uniNameToVar.find(name);
	return it != uniNameToVar.end();
}


//==============================================================================
// attribVarExists                                                             =
//==============================================================================
bool ShaderProg::attribVarExists(const char* name) const
{
	NameToSProgAttribVarIterator it = attribNameToVar.find(name);
	return it != attribNameToVar.end();
}


//==============================================================================
// createSrcCodeToCache                                                        =
//==============================================================================
std::string ShaderProg::createSrcCodeToCache(const char* sProgFPathName, const char* preAppendedSrcCode)
{
	if(strlen(preAppendedSrcCode) < 1)
	{
		return sProgFPathName;
	}

	// Create suffix
	boost::hash<std::string> stringHash;
	std::size_t h = stringHash(preAppendedSrcCode);
	std::string suffix = boost::lexical_cast<std::string>(h);

	//
	boost::filesystem::path newfPathName = AppSingleton::getInstance().getCachePath() /
		(boost::filesystem::path(sProgFPathName).filename() + "." + suffix);

	if(boost::filesystem::exists(newfPathName))
	{
		return newfPathName.string();
	}

	std::string src_ = Util::readFile(sProgFPathName);
	std::string src = preAppendedSrcCode + src_;

	std::ofstream f(newfPathName.string().c_str());
	if(!f.is_open())
	{
		throw EXCEPTION("Cannot open file for writing \"" + newfPathName.string() + "\"");
	}

	f.write(src.c_str(), src.length());

	return newfPathName.string();
}


//==============================================================================
// getShaderInfoString                                                         =
//==============================================================================
std::string ShaderProg::getShaderInfoString() const
{
	std::stringstream ss;

	ss << "Uniform vars:\n";
	BOOST_FOREACH(const SProgUniVar& var, uniVars)
	{
		ss << var.getName() << " " << var.getLoc() << "\n";
	}

	ss << "\nAttribute vars:\n";
	BOOST_FOREACH(const SProgAttribVar& var, attribVars)
	{
		ss << var.getName() << " " << var.getLoc() << "\n";
	}

	return ss.str();
}

