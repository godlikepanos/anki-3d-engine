#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include "ShaderProg.h"
#include "ShaderPrePreprocessor.h"
#include "Texture.h" // For certain setters
#include "App.h" // To get cache dir


#define SPROG_EXCEPTION(x) EXCEPTION("Shader prog \"" + getRsrcName() + "\": " + x)


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================

std::string ShaderProg::stdSourceCode(
	"#version 150 compatibility\n \
	precision lowp float;\n \
	#pragma optimize(on)\n \
	#pragma debug(off)\n"
);


//======================================================================================================================
// set uniforms                                                                                                        =
//======================================================================================================================
/// Standard set uniform checks
/// - Check if initialized
/// - if the current shader program is the var's shader program
/// - if the GL driver gives the same location as the one the var has
#define STD_SET_UNI_CHECK() \
	RASSERT_THROW_EXCEPTION(getLoc() == -1); \
	RASSERT_THROW_EXCEPTION(ShaderProg::getCurrentProgramGlId() != fatherSProg->getGlId()); \
	RASSERT_THROW_EXCEPTION(glGetUniformLocation(fatherSProg->getGlId(), getName().c_str()) != getLoc());


void ShaderProg::UniVar::setFloat(float f) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_FLOAT);
	glUniform1f(getLoc(), f);
}

void ShaderProg::UniVar::setFloatVec(float f[], uint size) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_FLOAT);
	glUniform1fv(getLoc(), size, f);
}

void ShaderProg::UniVar::setVec2(const Vec2 v2[], uint size) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_FLOAT_VEC2);
	glUniform2fv(getLoc(), size, &(const_cast<Vec2&>(v2[0]))[0]);
}

void ShaderProg::UniVar::setVec3(const Vec3 v3[], uint size) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_FLOAT_VEC3);
	glUniform3fv(getLoc(), size, &(const_cast<Vec3&>(v3[0]))[0]);
}

void ShaderProg::UniVar::setVec4(const Vec4 v4[], uint size) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_FLOAT_VEC4);
	glUniform4fv(getLoc(), size, &(const_cast<Vec4&>(v4[0]))[0]);
}

void ShaderProg::UniVar::setMat3(const Mat3 m3[], uint size) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_FLOAT_MAT3);
	glUniformMatrix3fv(getLoc(), size, true, &(m3[0])[0]);
}

void ShaderProg::UniVar::setMat4(const Mat4 m4[], uint size) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_FLOAT_MAT4);
	glUniformMatrix4fv(getLoc(), size, true, &(m4[0])[0]);
}

void ShaderProg::UniVar::setTexture(const Texture& tex, uint texUnit) const
{
	STD_SET_UNI_CHECK();
	RASSERT_THROW_EXCEPTION(getGlDataType() != GL_SAMPLER_2D && getGlDataType() != GL_SAMPLER_2D_SHADOW);
	tex.bind(texUnit);
	glUniform1i(getLoc(), texUnit);
}


//======================================================================================================================
// createAndCompileShader                                                                                              =
//======================================================================================================================
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
		
		const char* shaderType;
		switch(type)
		{
			case GL_VERTEX_SHADER:
				shaderType = "Vertex shader";
				break;
			case GL_FRAGMENT_SHADER:
				shaderType = "Fragment shader";
				break;
			default:
				RASSERT_THROW_EXCEPTION(1); // Not supported
		}
		throw SPROG_EXCEPTION(shaderType + " compiler error log follows:\n" + infoLog);
	}

	return glId;
}


//======================================================================================================================
// link                                                                                                                =
//======================================================================================================================
void ShaderProg::link()
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
		string infoLogTxt;

		glGetProgramiv(glId, GL_INFO_LOG_LENGTH, &info_len);

		infoLogTxt.reserve(info_len + 1);
		glGetProgramInfoLog(glId, info_len, &charsWritten, &infoLogTxt[0]);
		throw SPROG_EXCEPTION("Link error log follows:\n" + infoLogTxt);
	}
}


//======================================================================================================================
// getUniAndAttribVars                                                                                                 =
//======================================================================================================================
void ShaderProg::getUniAndAttribVars()
{
	int num;
	char name_[256];
	GLsizei length;
	GLint size;
	GLenum type;


	// attrib locations
	glGetProgramiv(glId, GL_ACTIVE_ATTRIBUTES, &num);
	attribVars.reserve(num);
	for(int i=0; i<num; i++) // loop all attributes
	{
		glGetActiveAttrib(glId, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_);
		name_[length] = '\0';

		// check if its FFP location
		int loc = glGetAttribLocation(glId, name_);
		if(loc == -1) // if -1 it means that its an FFP var
		{
			std::cout << "You are using FFP vertex attributes (\"" << name_ << "\")" << endl; /// @todo fix this
			continue;
		}

		attribVars.push_back(AttribVar(loc, name_, type, this));
		attribNameToVar[attribVars.back().getName().c_str()] = &attribVars.back();
	}


	// uni locations
	glGetProgramiv(glId, GL_ACTIVE_UNIFORMS, &num);
	uniVars.reserve(num);
	for(int i=0; i<num; i++) // loop all uniforms
	{
		glGetActiveUniform(glId, i, sizeof(name_)/sizeof(char), &length, &size, &type, name_);
		name_[length] = '\0';

		// check if its FFP location
		int loc = glGetUniformLocation(glId, name_);
		if(loc == -1) // if -1 it means that its an FFP var
		{
			std::cout << "You are using FFP uniforms (\"" << name_ << "\")" << std::endl;
			continue;
		}

		uniVars.push_back(UniVar(loc, name_, type, this));
		uniNameToVar[uniVars.back().getName().c_str()] = &uniVars.back();
	}
}


//======================================================================================================================
// bindCustomAttribLocs                                                                                                =
//======================================================================================================================
void ShaderProg::bindCustomAttribLocs(const ShaderPrePreprocessor& pars) const
{
	for(uint i=0; i<pars.getOutput().getAttribLocs().size(); ++i)
	{
		const std::string& name = pars.getOutput().getAttribLocs()[i].name;
		int loc = pars.getOutput().getAttribLocs()[i].customLoc;
		glBindAttribLocation(glId, loc, name.c_str());

		// check for error
		if(!GL_OK())
		{
			throw SPROG_EXCEPTION("Something went wrong for attrib \"" + name + "\" and location " +
			                      boost::lexical_cast<std::string>(loc));
		}
	}
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void ShaderProg::load(const char* filename)
{
	RASSERT_THROW_EXCEPTION(glId != std::numeric_limits<uint>::max());

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
	if(pars.getOutput().getTrffbVaryings().size() > 1)
	{
		const char* varsArr[128];
		for(uint i=0; i<pars.getOutput().getTrffbVaryings().size(); i++)
		{
			varsArr[i] = pars.getOutput().getTrffbVaryings()[i].name.c_str();
		}
		glTransformFeedbackVaryings(glId, pars.getOutput().getTrffbVaryings().size(), varsArr, GL_SEPARATE_ATTRIBS);
	}

	// 6) link
	link();
	
	// init the rest
	getUniAndAttribVars();
}


//======================================================================================================================
// findUniVar                                                                                                          =
//======================================================================================================================
const ShaderProg::UniVar* ShaderProg::findUniVar(const char* name) const
{
	NameToUniVarIterator it = uniNameToVar.find(name);
	if(it == uniNameToVar.end())
	{
		throw SPROG_EXCEPTION("Cannot get uniform loc \"" + name + '\"');
	}
	return it->second;
}


//======================================================================================================================
// findAttribVar                                                                                                       =
//======================================================================================================================
const ShaderProg::AttribVar* ShaderProg::findAttribVar(const char* name) const
{
	NameToAttribVarIterator it = attribNameToVar.find(name);
	if(it == attribNameToVar.end())
	{
		throw SPROG_EXCEPTION("Cannot get attribute loc \"" + name + '\"');
	}
	return it->second;
}


//======================================================================================================================
// uniVarExists                                                                                                        =
//======================================================================================================================
bool ShaderProg::uniVarExists(const char* name) const
{
	NameToUniVarIterator it = uniNameToVar.find(name);
	return it != uniNameToVar.end();
}


//======================================================================================================================
// attribVarExists                                                                                                     =
//======================================================================================================================
bool ShaderProg::attribVarExists(const char* name) const
{
	NameToAttribVarIterator it = attribNameToVar.find(name);
	return it != attribNameToVar.end();
}


//======================================================================================================================
// createSrcCodeToCache                                                                                                =
//======================================================================================================================
std::string ShaderProg::createSrcCodeToCache(const char* sProgFPathName, const char* preAppendedSrcCode,
                                             const char* newFNamePrefix)
{
	filesystem::path newfPathName = app->getCachePath() /
	                                (std::string(newFNamePrefix) + "_" + filesystem::path(sProgFPathName).filename());

	if(filesystem::exists(newfPathName))
	{
		return newfPathName.string();
	}

	std::string src_ = Util::readFile(sProgFPathName);
	std::string src = preAppendedSrcCode + src_;

	ofstream f(newfPathName.string().c_str());
	if(!f.good())
	{
		throw EXCEPTION("Cannot open file for writing \"" + newfPathName.string() + "\"");
	}

	f.write(src.c_str(), src.length());

	return newfPathName.string();
}
