// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ShaderResource.h"
#include "anki/resource/ProgramPrePreprocessor.h"
#include "anki/resource/ResourceManager.h"
#include "anki/core/App.h" // To get cache dir
#include "anki/util/File.h"
#include "anki/util/Filesystem.h"
#include "anki/util/Hash.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Error ShaderResource::load(const CString& filename, ResourceInitializer& init)
{
	return load(filename, " ", init.m_resources);
}

//==============================================================================
Error ShaderResource::load(const CString& filename, const CString& extraSrc,
	ResourceManager& manager)
{
	Error err = ErrorCode::NONE;
	auto alloc = manager._getTempAllocator();

	ProgramPrePreprocessor pars(&manager);
	ANKI_CHECK(pars.parseFile(filename));
	
	// Allocate new source
	StringAuto source(alloc);
	if(extraSrc.getLength() > 0)
	{
		ANKI_CHECK(source.create(extraSrc));
	}

	ANKI_CHECK(source.append(pars.getShaderSource()));

	// Create
	GrManager& gr = manager.getGrManager();
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gr));

	ANKI_CHECK(
		m_shader.create(
		cmdb, 
		pars.getShaderType(), &source[0], 
		source.getLength() + 1));

	cmdb.flush();

	m_type = pars.getShaderType();

	return err;
}

//==============================================================================
Error ShaderResource::createToCache(
	const CString& filename, const CString& preAppendedSrcCode, 
	const CString& filenamePrefix, ResourceManager& manager,
	StringAuto& out)
{
	Error err = ErrorCode::NONE;
	auto alloc = manager._getTempAllocator();

	if(preAppendedSrcCode.getLength() < 1)
	{
		return out.create(filename);
	}

	// Create suffix
	StringAuto unique(alloc);

	ANKI_CHECK(unique.create(filename));
	ANKI_CHECK(unique.append(preAppendedSrcCode));

	U64 h = computeHash(&unique[0], unique.getLength());

	StringAuto suffix(alloc);
	ANKI_CHECK(suffix.toString(h));

	// Compose cached filename
	StringAuto newFilename(alloc);

	ANKI_CHECK(
		newFilename.sprintf(
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
	StringAuto src(alloc);

	StringAuto fixedFname(alloc);
	ANKI_CHECK(manager.fixResourceFilename(filename, fixedFname));

	File file;
	ANKI_CHECK(file.open(fixedFname.toCString(), File::OpenFlag::READ));
	ANKI_CHECK(file.readAllText(TempResourceAllocator<char>(alloc), src));

	StringAuto srcfull(alloc);
	ANKI_CHECK(srcfull.sprintf("%s%s", &preAppendedSrcCode[0], &src[0]));

	// Write cached file
	File f;
	ANKI_CHECK(f.open(newFilename.toCString(), File::OpenFlag::WRITE));
	ANKI_CHECK(f.writeText("%s\n", &srcfull[0]));

	out = std::move(newFilename);
	return err;
}

} // end namespace anki
