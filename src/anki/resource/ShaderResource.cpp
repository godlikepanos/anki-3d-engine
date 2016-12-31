// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderResource.h>
#include <anki/resource/ShaderLoader.h>
#include <anki/resource/ResourceManager.h>
#include <anki/core/App.h> // To get cache dir
#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/util/Hash.h>
#include <anki/util/Assert.h>

namespace anki
{

ShaderResource::ShaderResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

ShaderResource::~ShaderResource()
{
}

Error ShaderResource::load(const ResourceFilename& filename)
{
	return load(filename, " ");
}

Error ShaderResource::load(const ResourceFilename& filename, const CString& extraSrc)
{
	auto alloc = getTempAllocator();

	ShaderLoader pars(&getManager());
	ANKI_CHECK(pars.parseFile(filename));

	// Allocate new source
	StringAuto source(alloc);

	source.append(getManager()._getShadersPrependedSource());

	if(extraSrc.getLength() > 0)
	{
		source.append(extraSrc);
	}

	source.append(pars.getShaderSource());

	// Create
	m_shader = getManager().getGrManager().newInstance<Shader>(pars.getShaderType(), source.toCString());

	m_type = pars.getShaderType();

	return ErrorCode::NONE;
}

Error ShaderResource::createToCache(const ResourceFilename& filename,
	const CString& preAppendedSrcCode,
	const CString& filenamePrefix,
	ResourceManager& manager,
	StringAuto& out)
{
	auto alloc = manager.getTempAllocator();

	if(preAppendedSrcCode.getLength() < 1)
	{
		// Early exit, nothing to mutate
		out.create(filename);
		return ErrorCode::NONE;
	}

	// Create suffix
	StringAuto unique(alloc);

	unique.create(filename);
	unique.append(preAppendedSrcCode);

	U64 h = computeHash(&unique[0], unique.getLength());

	StringAuto suffix(alloc);
	suffix.toString(h);

	// Create out
	out = StringAuto(alloc);
	ShaderType inShaderType;
	ANKI_CHECK(fileExtensionToShaderType(filename, inShaderType));
	out.sprintf("%s%s%s", &filenamePrefix[0], &suffix[0], &shaderTypeToFileExtension(inShaderType)[0]);

	// Compose cached filename
	StringAuto newFilename(alloc);

	newFilename.sprintf("%s/%s", &manager._getCacheDirectory()[0], &out[0]);

	if(fileExists(newFilename.toCString()))
	{
		return ErrorCode::NONE;
	}

	// Read file and append code
	StringAuto src(alloc);

	ResourceFilePtr file;
	ANKI_CHECK(manager.getFilesystem().openFile(filename, file));
	ANKI_CHECK(file->readAllText(alloc, src));

	StringAuto srcfull(alloc);
	srcfull.sprintf("%s%s", &preAppendedSrcCode[0], &src[0]);

	// Write cached file
	File f;
	ANKI_CHECK(f.open(newFilename.toCString(), FileOpenFlag::WRITE));
	ANKI_CHECK(f.writeText("%s\n", &srcfull[0]));

	return ErrorCode::NONE;
}

} // end namespace anki
