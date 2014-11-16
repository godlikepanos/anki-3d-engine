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
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Error ProgramResource::load(const CString& filename, ResourceInitializer& init)
{
	return load(filename, " ", init.m_resources);
}

//==============================================================================
Error ProgramResource::load(const CString& filename, const CString& extraSrc,
	ResourceManager& manager)
{
	Error err = ErrorCode::NONE;
	auto alloc = manager._getTempAllocator();

	ProgramPrePreprocessor pars(&manager);
	ANKI_CHECK(pars.parseFile(filename));
	
	// Allocate new source
	TempResourceString source;
	TempResourceString::ScopeDestroyer sourced(&source, alloc);
	if(extraSrc.getLength() > 0)
	{
		ANKI_CHECK(source.create(alloc, extraSrc));
	}

	ANKI_CHECK(source.append(alloc, pars.getShaderSource()));

	// Create
	GlDevice& gl = manager._getGlDevice();
	GlCommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gl));

	GlClientBufferHandle glsource;
	ANKI_CHECK(glsource.create(cmdb, source.getLength() + 1, nullptr));

	std::strcpy(reinterpret_cast<char*>(glsource.getBaseAddress()), &source[0]);

	ANKI_CHECK(m_shader.create(cmdb, 
		computeGlShaderType(pars.getShaderType()), glsource));

	cmdb.flush();

	return err;
}

//==============================================================================
Error ProgramResource::createToCache(
	const CString& filename, const CString& preAppendedSrcCode, 
	const CString& filenamePrefix, ResourceManager& manager,
	TempResourceString& out)
{
	Error err = ErrorCode::NONE;
	auto alloc = manager._getTempAllocator();

	if(preAppendedSrcCode.getLength() < 1)
	{
		return out.create(alloc, filename);
	}

	// Create suffix
	TempResourceString unique;
	TempResourceString::ScopeDestroyer uniqued(&unique, alloc);

	ANKI_CHECK(unique.create(alloc, filename));
	ANKI_CHECK(unique.append(alloc, preAppendedSrcCode));

	U64 h = computeHash(&unique[0], unique.getLength());

	TempResourceString suffix;
	TempResourceString::ScopeDestroyer suffixd(&suffix, alloc);
	ANKI_CHECK(suffix.toString(alloc, h));

	// Compose cached filename
	TempResourceString newFilename;
	TempResourceString::ScopeDestroyer newFilenamed(&newFilename, alloc);

	ANKI_CHECK(newFilename.sprintf(
		alloc,
		"%s/%s%s.glsl", 
		&manager._getCacheDirectory()[0],
		&filenamePrefix[0],
		&suffix[0]));

	if(fileExists(newFilename.toCString()))
	{
		out = std::move(newFilename);
		return err;
	}

	// Read file and append code
	TempResourceString src;
	TempResourceString::ScopeDestroyer srcd(&src, alloc);

	TempResourceString fixedFname;
	TempResourceString::ScopeDestroyer fixedFnamed(&fixedFname, alloc);
	ANKI_CHECK(manager.fixResourceFilename(filename, fixedFname));

	File file;
	ANKI_CHECK(file.open(fixedFname.toCString(), File::OpenFlag::READ));
	ANKI_CHECK(file.readAllText(TempResourceAllocator<char>(alloc), src));

	TempResourceString srcfull;
	TempResourceString::ScopeDestroyer srcfulld(&srcfull, alloc);
	ANKI_CHECK(srcfull.sprintf(alloc, "%s%s", &preAppendedSrcCode[0], &src[0]));

	// Write cached file
	File f;
	ANKI_CHECK(f.open(newFilename.toCString(), File::OpenFlag::WRITE));
	ANKI_CHECK(f.writeText("%s\n", &srcfull[0]));

	out = std::move(newFilename);
	return err;
}

} // end namespace anki
