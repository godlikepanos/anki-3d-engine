#include "anki/gl/ShaderProgram.h"
#include "anki/gl/GlException.h"
#include "anki/math/Math.h"
#include "anki/util/Exception.h"
#include "anki/gl/Texture.h"

namespace anki {

//==============================================================================
// ShaderProgramVariable                                                       =
//==============================================================================

//==============================================================================
void ShaderProgramUniformVariable::doCommonSetCode()
{
	ANKI_ASSERT(getLocation() != -1);
	ANKI_ASSERT(ShaderProgram::getCurrentProgramGlId() == 
		getFatherShaderProgram().getGlId());
	ANKI_ASSERT(glGetUniformLocation(getFatherShaderProgram().getGlId(),
		getName().c_str()) == getLocation());

	enableFlag(SPUVF_DIRTY);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const float x) 
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT);
	ANKI_ASSERT(getSize() == 1);

	glUniform1f(getLocation(), x);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec2& x)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ANKI_ASSERT(getSize() == 2);

	glUniform2f(getLocation(), x.x(), x.y());
}

//==============================================================================
void ShaderProgramUniformVariable::set(const float x[], uint size)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT);
	ANKI_ASSERT(getSize() == size);
	
	glUniform1fv(getLocation(), size, x);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec2 x[], uint size)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ANKI_ASSERT(getSize() == size * 2);

	glUniform2fv(getLocation(), size, &(const_cast<Vec2&>(x[0]))[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec3 x[], uint size)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	ANKI_ASSERT(getSize() == size * 3);

	glUniform3fv(getLocation(), size, &(const_cast<Vec3&>(x[0]))[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec4 x[], uint size)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	ANKI_ASSERT(getSize() == size * 4);
	
	glUniform4fv(getLocation(), size, &(const_cast<Vec4&>(x[0]))[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Mat3 x[], uint size)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_MAT3);
	ANKI_ASSERT(getSize() == size);

	glUniformMatrix3fv(getLocation(), size, true, &(x[0])[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Mat4 x[], uint size)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_MAT4);
	ANKI_ASSERT(getSize() == size);

	glUniformMatrix4fv(getLocation(), size, true, &(x[0])[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Texture& tex)
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_SAMPLER_2D 
		|| getGlDataType() == GL_SAMPLER_2D_SHADOW);
	
	tex.bind();
	glUniform1i(getLocation(), tex.getUnit());
}

//==============================================================================
// ShaderProgram                                                               =
//==============================================================================

//==============================================================================

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

const thread_local ShaderProgram* ShaderProgram::currentProgram = nullptr;

//==============================================================================
void ShaderProgram::create(const char* vertSource, const char* tcSource, 
	const char* teSource, const char* geomSource, const char* fragSource,
	const char* transformFeedbackVaryings[])
{
	ANKI_ASSERT(!isCreated());

	// 1) create and compile the shaders
	//
	std::string preprocSource = stdSourceCode;

	ANKI_ASSERT(vertSource != nullptr);
	vertShaderGlId = createAndCompileShader(vertSource, preprocSource.c_str(),
		GL_VERTEX_SHADER);

	if(tcSource != nullptr)
	{
		tcShaderGlId = createAndCompileShader(tcSource, preprocSource.c_str(), 
			GL_TESS_CONTROL_SHADER);
	}

	if(teSource != nullptr)
	{
		teShaderGlId = createAndCompileShader(teSource, preprocSource.c_str(), 
			GL_TESS_EVALUATION_SHADER);
	}

	if(geomSource != nullptr)
	{
		geomShaderGlId = createAndCompileShader(geomSource, 
			preprocSource.c_str(), GL_GEOMETRY_SHADER);
	}

	ANKI_ASSERT(fragSource != nullptr);
	fragShaderGlId = createAndCompileShader(fragSource, preprocSource.c_str(),
		GL_FRAGMENT_SHADER);

	// 2) create program and attach shaders
	glId = glCreateProgram();
	if(glId == 0)
	{
		throw ANKI_EXCEPTION("glCreateProgram failed");
	}
	glAttachShader(glId, vertShaderGlId);
	glAttachShader(glId, fragShaderGlId);

	if(tcSource != nullptr)
	{
		glAttachShader(glId, tcShaderGlId);
	}

	if(teSource != nullptr)
	{
		glAttachShader(glId, teShaderGlId);
	}

	if(geomSource != nullptr)
	{
		glAttachShader(glId, geomShaderGlId);
	}

	// 3) set the TRFFB varyings
	ANKI_ASSERT(transformFeedbackVaryings != nullptr);
	int count = 0;
	while(*transformFeedbackVaryings != nullptr)
	{
		++count;
		++transformFeedbackVaryings;
	}

	if(count)
	{
		glTransformFeedbackVaryings(
			glId,
			count, 
			transformFeedbackVaryings,
			GL_SEPARATE_ATTRIBS);
	}

	// 4) link
	link();

	// init the rest
	getUniAndAttribVars();
}

//==============================================================================
void ShaderProgram::destroy()
{
	unbind();

	if(vertShaderGlId != 0)
	{
		glDeleteShader(vertShaderGlId);
	}

	if(tcShaderGlId != 0)
	{
		glDeleteShader(tcShaderGlId);
	}

	if(teShaderGlId != 0)
	{
		glDeleteShader(teShaderGlId);
	}

	if(geomShaderGlId != 0)
	{
		glDeleteShader(geomShaderGlId);
	}

	if(fragShaderGlId != 0)
	{
		glDeleteShader(fragShaderGlId);
	}

	if(glId != 0)
	{
		glDeleteProgram(glId);
	}

	init();
}

//==============================================================================
GLuint ShaderProgram::createAndCompileShader(const char* sourceCode,
	const char* preproc, GLenum type)
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
		
		throw ANKI_EXCEPTION("Shader failed. Log:\n" + &infoLog[0]
			+ "\nSource:\n" + sourceCode);
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
		throw ANKI_EXCEPTION("Link error log follows:\n" 
			+ infoLogTxt);
	}
}

//==============================================================================
void ShaderProgram::getUniAndAttribVars()
{
	int num;
	std::array<char, 256> name_;
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
			throw ANKI_EXCEPTION("You are using FFP vertex attributes");
		}

		ShaderProgramAttributeVariable* var =
			new ShaderProgramAttributeVariable(loc, &name_[0], type, 
			size, this);

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
			throw ANKI_EXCEPTION("You are using FFP vertex uniforms");
		}

		ShaderProgramUniformVariable* var =
			new ShaderProgramUniformVariable(loc, &name_[0], type, 
			size, this);

		vars.push_back(var);
		unis.push_back(var);
		nameToVar[var->getName().c_str()] = var;
		nameToUniVar[var->getName().c_str()] = var;
	}
}

//==============================================================================
const ShaderProgramVariable* ShaderProgram::findVariableByName(
	const char* name) const
{
	NameToVarHashMap::const_iterator it = nameToVar.find(name);
	if(it == nameToVar.end())
	{
		return nullptr;
	}
	return it->second;
}

//==============================================================================
const ShaderProgramAttributeVariable*
	ShaderProgram::findAttributeVariableByName(const char* name) const
{
	NameToAttribVarHashMap::const_iterator it = nameToAttribVar.find(name);
	if(it == nameToAttribVar.end())
	{
		return nullptr;
	}
	return it->second;
}

//==============================================================================
const ShaderProgramUniformVariable* ShaderProgram::findUniformVariableByName(
	const char* name) const
{
	NameToUniVarHashMap::const_iterator it = nameToUniVar.find(name);
	if(it == nameToUniVar.end())
	{
		return nullptr;
	}
	return it->second;
}

//==============================================================================
void ShaderProgram::cleanAllUniformsDirtyFlags()
{
	for(ShaderProgramUniformVariable* uni : unis)
	{
		uni->disableFlag(ShaderProgramUniformVariable::SPUVF_DIRTY);
	}
}

//==============================================================================
std::ostream& operator<<(std::ostream& s, const ShaderProgram& x)
{
	s << "ShaderProgram\n";
	s << "Variables:\n";
	for(const ShaderProgramVariable& var : x.vars)
	{
		s << var.getName() << " " << var.getLocation() << " " 
			<< (var.getType() == ShaderProgramVariable::SPVT_ATTRIBUTE 
			? "[A]" : "[U]") <<  '\n';
	}
	return s;
}

} // end namespace
