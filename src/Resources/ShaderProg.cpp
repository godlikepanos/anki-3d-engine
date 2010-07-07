#include <boost/filesystem.hpp>
#include <fstream>
#include "ShaderProg.h"
#include "Renderer.h"
#include "ShaderPrePreprocessor.h"
#include "Texture.h"
#include "App.h"


#define SHADER_ERROR(x) ERROR("Shader (" << getRsrcName() << "): " << x)
#define SHADER_WARNING(x) WARNING("Shader (" << getRsrcName() << "): " << x)


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================

string ShaderProg::stdSourceCode(
	"#version 150 compatibility\n \
	precision lowp float;\n \
	#pragma optimize(on)\n \
	#pragma debug(off)\n"
);


//======================================================================================================================
// set uniforms                                                                                                        =
//======================================================================================================================
/**
 * Standard set uniform checks
 * - Check if initialized
 * - if the current shader program is the var's shader program
 * - if the GL driver gives the same location as the one the var has
 */
#define STD_SET_UNI_CHECK() DEBUG_ERR(getLoc() == -1); \
                            DEBUG_ERR(ShaderProg::getCurrentProgramGlId() != fatherSProg->getGlId()); \
                            DEBUG_ERR(glGetUniformLocation(fatherSProg->getGlId(), getName().c_str()) != getLoc());


void ShaderProg::UniVar::setFloat(float f) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_FLOAT);
	glUniform1f(getLoc(), f);
}

void ShaderProg::UniVar::setFloatVec(float f[], uint size) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_FLOAT);
	glUniform1fv(getLoc(), size, f);
}

void ShaderProg::UniVar::setVec2(const Vec2 v2[], uint size) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_FLOAT_VEC2);
	glUniform2fv(getLoc(), size, &(const_cast<Vec2&>(v2[0]))[0]);
}

void ShaderProg::UniVar::setVec3(const Vec3 v3[], uint size) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_FLOAT_VEC3);
	glUniform3fv(getLoc(), size, &(const_cast<Vec3&>(v3[0]))[0]);
}

void ShaderProg::UniVar::setVec4(const Vec4 v4[], uint size) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_FLOAT_VEC4);
	glUniform4fv(getLoc(), size, &(const_cast<Vec4&>(v4[0]))[0]);
}

void ShaderProg::UniVar::setMat3(const Mat3 m3[], uint size) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_FLOAT_MAT3);
	glUniformMatrix3fv(getLoc(), size, true, &(m3[0])[0]);
}

void ShaderProg::UniVar::setMat4(const Mat4 m4[], uint size) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_FLOAT_MAT4);
	glUniformMatrix4fv(getLoc(), size, true, &(m4[0])[0]);
}

void ShaderProg::UniVar::setTexture(const Texture& tex, uint texUnit) const
{
	STD_SET_UNI_CHECK();
	DEBUG_ERR(getGlDataType() != GL_SAMPLER_2D && getGlDataType() != GL_SAMPLER_2D_SHADOW);
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
		char* infoLog = NULL;

		glGetShaderiv(glId, GL_INFO_LOG_LENGTH, &infoLen);
		infoLog = (char*)malloc((infoLen+1)*sizeof(char));
		glGetShaderInfoLog(glId, infoLen, &charsWritten, infoLog);
		
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
				DEBUG_ERR(1); // Not supported
		}
		SHADER_ERROR(shaderType << " compiler log follows:\n" << infoLog);
		
		free(infoLog);
		return 0;
	}

	return glId;
}


//======================================================================================================================
// link                                                                                                                =
//======================================================================================================================
bool ShaderProg::link()
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
		char* infoLogTxt = NULL;

		glGetProgramiv(glId, GL_INFO_LOG_LENGTH, &info_len);

		infoLogTxt = (char*)malloc((info_len+1)*sizeof(char));
		glGetProgramInfoLog(glId, info_len, &charsWritten, infoLogTxt);
		SHADER_ERROR("Link log follows:\n" << infoLogTxt);
		free(infoLogTxt);
		return false;
	}

	return true;
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
			SHADER_WARNING("You are using FFP vertex attributes (\"" << name_ << "\")");
			continue;
		}

		attribVars.push_back(AttribVar(loc, name_, type, this));
		attribNameToVar[name_] = &attribVars.back();
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
			SHADER_WARNING("You are using FFP uniforms (\"" << name_ << "\")");
			continue;
		}

		uniVars.push_back(UniVar(loc, name_, type, this));
		uniNameToVar[name_] = &uniVars.back();
	}
}


//======================================================================================================================
// bindCustomAttribLocs                                                                                                =
//======================================================================================================================
bool ShaderProg::bindCustomAttribLocs(const ShaderPrePreprocessor& pars) const
{
	for(uint i=0; i<pars.getOutput().getAttribLocs().size(); ++i)
	{
		const string& name = pars.getOutput().getAttribLocs()[i].name;
		int loc = pars.getOutput().getAttribLocs()[i].customLoc;
		glBindAttribLocation(glId, loc, name.c_str());

		// check for error
		if(!GL_OK())
		{
			SHADER_ERROR("Something went wrong for attrib \"" << name << "\" and location " << loc);
			return false;
		}
	}
	return true;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool ShaderProg::load(const char* filename)
{
	DEBUG_ERR(glId != numeric_limits<uint>::max());

	ShaderPrePreprocessor pars;

	if(!pars.parseFile(filename)) return false;

	// 1) create and compile the shaders
	string preprocSource = stdSourceCode;
	uint vertGlId = createAndCompileShader(pars.getOutput().getVertShaderSource().c_str(), preprocSource.c_str(),
	                                       GL_VERTEX_SHADER);
	if(vertGlId == 0) return false;

	uint fragGlId = createAndCompileShader(pars.getOutput().getFragShaderSource().c_str(), preprocSource.c_str(),
	                                       GL_FRAGMENT_SHADER);
	if(fragGlId == 0) return false;

	// 2) create program and attach shaders
	glId = glCreateProgram();
	if(glId == 0)
	{
		ERROR("glCreateProgram failed");
		return false;
	}
	glAttachShader(glId, vertGlId);
	glAttachShader(glId, fragGlId);

	// 3) bind the custom attrib locs
	if(!bindCustomAttribLocs(pars)) return false;

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
	if(!link()) return false;
	
	// init the rest
	getUniAndAttribVars();

	return true;
}


//======================================================================================================================
// findUniVar                                                                                                          =
//======================================================================================================================
const ShaderProg::UniVar* ShaderProg::findUniVar(const char* name) const
{
	NameToUniVarIterator it = uniNameToVar.find(name);
	if(it == uniNameToVar.end())
	{
		SHADER_ERROR("Cannot get uniform loc \"" << name << '\"');
		return NULL;
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
		SHADER_ERROR("Cannot get attribute loc \"" << name << '\"');
		return NULL;
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
string ShaderProg::createSrcCodeToCache(const char* sProgFPathName, const char* preAppendedSrcCode,
                                        const char* newFNamePrefix)
{
	filesystem::path newfPathName = app->getCachePath() /
	                                (string(newFNamePrefix) + "_" + filesystem::path(sProgFPathName).filename());

	if(filesystem::exists(newfPathName))
	{
		return newfPathName.string();
	}

	string src_ = Util::readFile(sProgFPathName);
	DEBUG_ERR(src_ == "");
	string src = preAppendedSrcCode + src_;

	ofstream f(newfPathName.string().c_str());
	if(!f.good())
	{
		ERROR("Cannot open file for writing \"" << newfPathName.string() << "\"");
		return newfPathName.string();
	}

	f.write(src.c_str(), src.length());

	return newfPathName.string();
}
