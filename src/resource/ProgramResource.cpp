// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ProgramResource.h"
#include "anki/resource/ProgramPrePreprocessor.h"
#include "anki/resource/ResourceManager.h"
#include "anki/core/App.h" // To get cache dir
#include "anki/util/File.h"
#include "anki/util/Exception.h"
#include <cstring>

namespace anki {

//==============================================================================
void ProgramResource::load(const char* filename)
{
	load(filename, "");
}

//==============================================================================
void ProgramResource::load(const char* filename, const char* extraSrc)
{
	ProgramPrePreprocessor pars(filename);
	std::string source = extraSrc + pars.getShaderSource();

	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);
	GlClientBufferHandle glsource(jobs, source.length() + 1, nullptr);

	strcpy((char*)glsource.getBaseAddress(), &source[0]);

	m_prog = GlProgramHandle(jobs, 
		computeGlShaderType((U)pars.getShaderType()), glsource);

	jobs.flush();
}

//==============================================================================
std::string ProgramResource::createSrcCodeToCache(
	const char* filename, const char* preAppendedSrcCode, 
	const char* filenamePrefix)
{
	ANKI_ASSERT(filename && preAppendedSrcCode && filenamePrefix);

	if(strlen(preAppendedSrcCode) < 1)
	{
		return filename;
	}

	// Create suffix
	std::hash<std::string> stringHasher;
	PtrSize h = stringHasher(std::string(filename) + preAppendedSrcCode);
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
		File::OpenFlag::READ).readAllText(src);
	src = preAppendedSrcCode + src;

	// Write cached file
	File f(newFilename.c_str(), File::OpenFlag::WRITE);
	f.writeText("%s\n", src.c_str());

	return newFilename;
}

} // end namespace anki
