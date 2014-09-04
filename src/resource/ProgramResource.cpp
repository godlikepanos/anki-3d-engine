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
void ProgramResource::load(const CString& filename, ResourceInitializer& init)
{
	load(filename, "", init.m_resourceManager);
}

//==============================================================================
void ProgramResource::load(const CString& filename, const CString& extraSrc,
	ResourceManager& manager)
{
	ProgramPrePreprocessor pars(filename, &manager);
	TempResourceString source = extraSrc + pars.getShaderSource();

	GlDevice& gl = GlDeviceSingleton::get();
	GlCommandBufferHandle jobs(&gl);
	GlClientBufferHandle glsource(jobs, source.length() + 1, nullptr);

	strcpy((char*)glsource.getBaseAddress(), &source[0]);

	m_prog = GlProgramHandle(jobs, 
		computeGlShaderType((U)pars.getShaderType()), glsource);

	jobs.flush();
}

//==============================================================================
String ProgramResource::createSourceToCache(
	const char* filename, const char* preAppendedSrcCode, 
	const char* filenamePrefix, App& app)
{
	ANKI_ASSERT(filename && preAppendedSrcCode && filenamePrefix);

	auto& alloc = app.getAllocator();

	if(std::strlen(preAppendedSrcCode) < 1)
	{
		return String(filename, alloc);
	}

	// Create suffix
	std::hash<String> stringHasher;
	PtrSize h = stringHasher(String(filename, alloc) + preAppendedSrcCode);

	String suffix(alloc);
	toString(h, suffix);

	// Compose cached filename
	String newFilename(app.getCachePath()
		+ "/" + filenamePrefix + suffix + ".glsl";

	if(File::fileExists(newFilename.c_str()))
	{
		return newFilename;
	}

	// Read file and append code
	String src(alloc);
	File(ResourceManagerSingleton::get().fixResourcePath(filename).c_str(), 
		File::OpenFlag::READ).readAllText(src);
	src = preAppendedSrcCode + src;

	// Write cached file
	File f(newFilename.c_str(), File::OpenFlag::WRITE);
	f.writeText("%s\n", src.c_str());

	return newFilename;
}

} // end namespace anki
