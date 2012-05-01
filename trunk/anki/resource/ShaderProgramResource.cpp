#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/core/App.h" // To get cache dir
#include "anki/util/Util.h"
#include "anki/util/Exception.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>


namespace anki {


//==============================================================================
void ShaderProgramResource::load(const char* filename)
{
	ShaderProgramPrePreprocessor pars(filename);

	boost::array<const char*, 128> trfVarsArr = {{nullptr}};
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
	path newfPathName = AppSingleton::get().getCachePath() 
		/ (path(sProgFPathName).filename().string() + "." + suffix);

	if(exists(newfPathName))
	{
		return newfPathName.string();
	}

	std::string src_ = Util::readFile(sProgFPathName);
	std::string src = preAppendedSrcCode + src_;

	std::ofstream f(newfPathName.string().c_str());
	if(!f.is_open())
	{
		throw ANKI_EXCEPTION("Cannot open file for writing \"" 
			+ newfPathName.string() + "\"");
	}

	f.write(src.c_str(), src.length());

	return newfPathName.string();
}


} // end namespace
