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
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
void ProgramResource::load(const CString& filename, ResourceInitializer& init)
{
	load(filename, " ", init.m_resources);
}

//==============================================================================
void ProgramResource::load(const CString& filename, const CString& extraSrc,
	ResourceManager& manager)
{
	ProgramPrePreprocessor pars(filename, &manager);
	TempResourceString source(extraSrc + pars.getShaderSource());

	GlDevice& gl = manager._getGlDevice();
	GlCommandBufferHandle cmdb;
	cmdb.create(&gl);
	GlClientBufferHandle glsource;
	glsource.create(cmdb, source.getLength() + 1, nullptr);

	std::strcpy(reinterpret_cast<char*>(glsource.getBaseAddress()), &source[0]);

	m_prog.create(cmdb, 
		computeGlShaderType(pars.getShaderType()), glsource);

	cmdb.flush();
}

//==============================================================================
String ProgramResource::createToCache(
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
	String newFilename(manager._getCacheDirectory()
		+ "/" + filenamePrefix + suffix + ".glsl");

	if(fileExists(newFilename.toCString()))
	{
		return newFilename;
	}

	// Read file and append code
	String src(alloc);
	File file;
	file.open(manager.fixResourceFilename(filename).toCString(), 
		File::OpenFlag::READ);
	file.readAllText(src);
	src = preAppendedSrcCode + src;

	// Write cached file
	File f;
	f.open(newFilename.toCString(), File::OpenFlag::WRITE);
	Error err = f.writeText("%s\n", &src[0]);
	ANKI_ASSERT(!err && "handle_error");

	return newFilename;
}

} // end namespace anki
