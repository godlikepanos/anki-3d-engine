// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourceObject.h"
#include "anki/resource/ResourceManager.h"

namespace anki {

//==============================================================================
ResourceObject::~ResourceObject()
{
	m_uuid.destroy(getAllocator());
}

//==============================================================================
ResourceAllocator<U8> ResourceObject::getAllocator() const
{
	return m_manager->getAllocator();
}

//==============================================================================
TempResourceAllocator<U8> ResourceObject::getTempAllocator() const
{
	return m_manager->getTempAllocator();
}

//==============================================================================
Error ResourceObject::openFile(const CString& filename, ResourceFilePtr& file)
{
	return m_manager->getFilesystem().openFile(filename, file);
}

//==============================================================================
Error ResourceObject::openFileReadAllText(
	const CString& filename, StringAuto& text)
{
	// Load file
	ResourceFilePtr file;
	ANKI_CHECK(m_manager->getFilesystem().openFile(filename, file));

	// Read string
	text = std::move(StringAuto(getTempAllocator()));
	ANKI_CHECK(file->readAllText(getTempAllocator(), text));

	return ErrorCode::NONE;
}

} // end namespace anki
