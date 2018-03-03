// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ScriptResource.h>
#include <anki/util/File.h>

namespace anki
{

ScriptResource::~ScriptResource()
{
	m_source.destroy(getAllocator());
}

Error ScriptResource::load(const ResourceFilename& filename, Bool async)
{
	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	ANKI_CHECK(file->readAllText(getAllocator(), m_source));

	return Error::NONE;
}

} // end namespace anki
