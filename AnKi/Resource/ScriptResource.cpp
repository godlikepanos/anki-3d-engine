// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ScriptResource.h>
#include <AnKi/Util/File.h>

namespace anki {

ScriptResource::~ScriptResource()
{
	m_source.destroy(getMemoryPool());
}

Error ScriptResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	StringRaii src(&getMemoryPool());
	ANKI_CHECK(file->readAllText(src));
	m_source = std::move(src);

	return Error::kNone;
}

} // end namespace anki
