// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/GenericResource.h>

namespace anki
{

GenericResource::GenericResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

GenericResource::~GenericResource()
{
	m_data.destroy(getAllocator());
}

Error GenericResource::load(const ResourceFilename& filename, Bool async)
{
	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));

	const U32 size = U32(file->getSize());
	m_data.create(getAllocator(), size);
	ANKI_CHECK(file->read(&m_data[0], size));

	return Error::NONE;
}

} // end namespace anki
