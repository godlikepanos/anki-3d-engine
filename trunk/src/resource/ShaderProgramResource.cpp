#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/resource/ResourceManager.h"
#include "anki/core/App.h" // To get cache dir
#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include <sstream>
#include <unordered_map>

namespace anki {

//==============================================================================
void ShaderProgramResource::load(const char* filename)
{
	load(filename, "");
}

//==============================================================================
void ShaderProgramResource::load(const char* filename, const char* extraSrc)
{
	ShaderProgramPrePreprocessor pars(filename);

	Array<const char*, 128> trfVarsArr = {{nullptr}};
	GLenum xfbBufferMode = GL_NONE;
	if(pars.getTranformFeedbackVaryings().size() > 0)
	{
		U32 i;
		for(i = 0; i < pars.getTranformFeedbackVaryings().size(); i++)
		{
			trfVarsArr[i] = pars.getTranformFeedbackVaryings()[i].c_str();
		}
		trfVarsArr[i] = nullptr;

		switch(pars.getXfbBufferMode())
		{
		case ShaderProgramPrePreprocessor::XFBBM_INTERLEAVED:
			xfbBufferMode = GL_INTERLEAVED_ATTRIBS;
			break;
		case ShaderProgramPrePreprocessor::XFBBM_SEPARATE:
			xfbBufferMode = GL_SEPARATE_ATTRIBS;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	// Create the program
	if(pars.getShaderSource(ST_VERTEX).size() != 0)
	{
		std::string vertSrc = extraSrc + pars.getShaderSource(ST_VERTEX);
		std::string fragSrc = extraSrc + pars.getShaderSource(ST_FRAGMENT);

		std::string tcSrc;
		if(pars.getShaderSource(ST_TC).size() > 0)
		{
			tcSrc = extraSrc + pars.getShaderSource(ST_TC);
		}

		std::string teSrc;
		if(pars.getShaderSource(ST_TE).size() > 0)
		{
			teSrc = extraSrc + pars.getShaderSource(ST_TE);
		}

		std::string geomSrc;
		if(pars.getShaderSource(ST_GEOMETRY).size() > 0)
		{
			geomSrc = extraSrc + pars.getShaderSource(ST_GEOMETRY);
		}

		create(
			vertSrc.c_str(),
			tcSrc.size() ? tcSrc.c_str() : nullptr,
			teSrc.size() ? teSrc.c_str() : nullptr,
			geomSrc.size() ? geomSrc.c_str() : nullptr,
			fragSrc.c_str(),
			nullptr,
			&trfVarsArr[0],
			xfbBufferMode);
	}
	else
	{
		std::string computeSrc = extraSrc + pars.getShaderSource(ST_COMPUTE);

		create(
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			computeSrc.c_str(),
			nullptr,
			xfbBufferMode);
	}
}

//==============================================================================
std::string ShaderProgramResource::createSrcCodeToCache(
	const char* filename, const char* preAppendedSrcCode, 
	const char* filenamePrefix)
{
	ANKI_ASSERT(filename && preAppendedSrcCode && filenamePrefix);

	if(strlen(preAppendedSrcCode) < 1)
	{
		return filename;
	}

	// Create suffix
	std::hash<std::string> stringHash;
	std::size_t h = stringHash(std::string(filename) + preAppendedSrcCode);
	std::string suffix = std::to_string(h);

	// Compose cached filename
	std::string newFilename = AppSingleton::get().getCachePath()
		+ "/" + filenamePrefix + suffix + ".glsl";

	if(File::fileExists(newFilename.c_str()))
	{
		return newFilename;
	}

	// Read file and append code
	std::string src;
	File(ResourceManagerSingleton::get().fixResourcePath(filename).c_str(), 
		File::OF_READ).readAllText(src);
	src = preAppendedSrcCode + src;

	// Write cached file
	File f(newFilename.c_str(), File::OF_WRITE);
	f.writeText("%s\n", src.c_str());

	return newFilename;
}

} // end namespace anki
