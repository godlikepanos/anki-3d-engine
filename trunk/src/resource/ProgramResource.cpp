// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ProgramResource.h"
#include "anki/resource/ProgramPrePreprocessor.h"
#include "anki/resource/ResourceManager.h"
#include "anki/core/App.h" // To get cache dir
#include "anki/util/File.h"
#include "anki/util/Filesystem.h"
#include "anki/util/Hash.h"
#include "anki/util/Exception.h"

namespace anki {

//==============================================================================
void ProgramResource::load(const CString& filename, ResourceInitializer& init)
{
	load(filename, " ", init.m_resourceManager);
}

//==============================================================================
void ProgramResource::load(const CString& filename, const CString& extraSrc,
	ResourceManager& manager)
{
	ProgramPrePreprocessor pars(filename, &manager);
	TempResourceString source(extraSrc + pars.getShaderSource());

	GlDevice& gl = manager._getGlDevice();
	GlCommandBufferHandle jobs(&gl);
	GlClientBufferHandle glsource(jobs, source.getLength() + 1, nullptr);

	std::strcpy(reinterpret_cast<char*>(glsource.getBaseAddress()), &source[0]);

	m_prog = GlProgramHandle(jobs, 
		computeGlShaderType((U)pars.getShaderType()), glsource);

	jobs.flush();
}

//==============================================================================
String ProgramResource::createSourceToCache(
	const CString& filename, const CString& preAppendedSrcCode, 
	const CString& filenamePrefix, ResourceManager& manager)
{
	auto& alloc = manager._getAllocator();

	if(preAppendedSrcCode == "")
	{
		return String(filename, alloc);
	}

	// Create suffix
	String unique(String(filename, alloc) + preAppendedSrcCode);
	U64 h = computeHash(&unique[0], unique.getLength());

	String suffix(String::toString(h, alloc));

	// Compose cached filename
	String newFilename(manager._getApp().getCachePath()
		+ CString("/") + filenamePrefix + suffix + CString(".glsl"));

	if(fileExists(newFilename.toCString()))
	{
		return newFilename;
	}

	// Read file and append code
	String src(alloc);
	File(manager.fixResourceFilename(filename).toCString(), 
		File::OpenFlag::READ).readAllText(src);
	src = preAppendedSrcCode + src;

	// Write cached file
	File f(newFilename.toCString(), File::OpenFlag::WRITE);
	f.writeText("%s\n", &src[0]);

	return newFilename;
}

} // end namespace anki
