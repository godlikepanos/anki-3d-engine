#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/core/App.h" // To get cache dir
#include "anki/util/Filesystem.h"
#include "anki/util/Exception.h"
#include <fstream>
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

	std::string vertSrc = extraSrc + pars.getShaderSource(ST_VERTEX);
	std::string fragSrc = extraSrc + pars.getShaderSource(ST_FRAGMENT);

	create(vertSrc.c_str(),
		nullptr,
		nullptr,
		nullptr,
		fragSrc.c_str(),
		nullptr,
		&trfVarsArr[0],
		xfbBufferMode);
}

//==============================================================================
std::string ShaderProgramResource::createSrcCodeToCache(
	const char* sProgFPathName, const char* preAppendedSrcCode)
{
	if(strlen(preAppendedSrcCode) < 1)
	{
		return sProgFPathName;
	}

	// Create suffix
	std::hash<std::string> stringHash;
	std::size_t h = stringHash(std::string(sProgFPathName) 
		+ preAppendedSrcCode);
	std::string suffix = std::to_string(h);

	//
	std::string newfPathName = AppSingleton::get().getCachePath()
		+ "/" + suffix + ".glsl";

	if(fileExists(newfPathName.c_str()))
	{
		return newfPathName;
	}

	std::string src_ = readFile(sProgFPathName);
	std::string src = preAppendedSrcCode + src_;

	std::ofstream f(newfPathName.c_str());
	if(!f.is_open())
	{
		throw ANKI_EXCEPTION("Cannot open file for writing: "
			+ newfPathName);
	}

	f.write(src.c_str(), src.length());

	return newfPathName;
}

} // end namespace
