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
	ShaderProgramPrePreprocessor pars(filename);

	std::array<const char*, 128> trfVarsArr = {{nullptr}};
	if(pars.getTranformFeedbackVaryings().size() > 0)
	{
		uint i;
		for(i = 0; i < pars.getTranformFeedbackVaryings().size(); i++)
		{
			trfVarsArr[i] = pars.getTranformFeedbackVaryings()[i].c_str();
		}
		trfVarsArr[i] = nullptr;
	}

	create(pars.getShaderSource(ST_VERTEX).c_str(), 
		nullptr, 
		nullptr, 
		nullptr, 
		pars.getShaderSource(ST_FRAGMENT).c_str(),
		&trfVarsArr[0]);
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
	std::size_t h = stringHash(preAppendedSrcCode);
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
