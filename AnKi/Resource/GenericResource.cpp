// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/GenericResource.h>

namespace anki {

GenericResource::GenericResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

GenericResource::~GenericResource()
{
	m_data.destroy(getMemoryPool());
}

Error GenericResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	const U32 size = U32(file->getSize());
	m_data.create(getMemoryPool(), size);
	ANKI_CHECK(file->read(&m_data[0], size));

	return Error::kNone;
}

} // end namespace anki
