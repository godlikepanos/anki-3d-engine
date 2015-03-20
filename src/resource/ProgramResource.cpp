// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
	GrManager& gr = manager.getGrManager();
	CommandBufferHandle cmdb;
	err = cmdb.create(&gr);
	if(err)
	{
		return err;
	}

	err = m_shader.create(cmdb, 
		computeGlShaderType(pars.getShaderType()), &source[0], 
		source.getLength() + 1);
	if(err)
	{
		return err;
	}

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

	err = unique.create(alloc, filename);
	if(err)
	{
		return err;
	}

	err = unique.append(alloc, preAppendedSrcCode);
	if(err)
	{
		return err;
	}

	U64 h = computeHash(&unique[0], unique.getLength());

	TempResourceString suffix;
	TempResourceString::ScopeDestroyer suffixd(&suffix, alloc);
	ANKI_CHECK(suffix.toString(alloc, h));

	// Compose cached filename
	TempResourceString newFilename;
	TempResourceString::ScopeDestroyer newFilenamed(&newFilename, alloc);

	err = newFilename.sprintf(
		alloc,
		"%s/%s%s.glsl", 
		&manager._getCacheDirectory()[0],
		&filenamePrefix[0],
		&suffix[0]);
	if(err)
	{
		return err;
	}

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
	err = manager.fixResourceFilename(filename, fixedFname);
	if(err)
	{
		return err;
	}

	File file;
	err = file.open(fixedFname.toCString(), File::OpenFlag::READ);
	if(err)
	{
		return err;
	}
	err = file.readAllText(TempResourceAllocator<char>(alloc), src);
	if(err)
	{
		return err;
	}

	TempResourceString srcfull;
	TempResourceString::ScopeDestroyer srcfulld(&srcfull, alloc);
	err = srcfull.sprintf(alloc, "%s%s", &preAppendedSrcCode[0], &src[0]);
	if(err)
	{
		return err;
	}

	// Write cached file
	File f;
	err = f.open(newFilename.toCString(), File::OpenFlag::WRITE);
	if(err)
	{
		return err;
	}

	err = f.writeText("%s\n", &srcfull[0]);
	if(err)
	{
		return err;
	}

	out = std::move(newFilename);
	return err;
}

} // end namespace anki
