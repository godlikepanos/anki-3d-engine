// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Xml.h>

namespace anki {

void ResourceObject::release()
{
	const U32 uuid = m_uuid;
	const U32 idx = m_versionResourceIdx;
	const ResourceType type = m_type;
	ANKI_ASSERT(idx != kMaxU32);

	// Use release because we want the vars before to have been stored before the atomic operation. There are racy cases where a lot can happen
	// between the refcounding and the actual call to freeResource
	const I32 count = m_refcount.fetchSub(1, AtomicMemoryOrder::kRelease);
	if(count == 1)
	{
		switch(type)
		{
#define ANKI_INSTANTIATE_RESOURCE(type_) \
	case ResourceType::k##type_: \
		ResourceManager::getSingleton().freeResource<type_>(uuid, idx); \
		break;
#include <AnKi/Resource/Resources.def.h>

		default:
			ANKI_ASSERT(0);
		}
	}
}

Error ResourceObject::openFile(const CString& filename, ResourceFilePtr& file)
{
	return ResourceFilesystem::getSingleton().openFile(filename, file);
}

Error ResourceObject::openFileReadAllText(const CString& filename, ResourceString& text)
{
	// Load file
	ResourceFilePtr file;
	ANKI_CHECK(ResourceFilesystem::getSingleton().openFile(filename, file));

	// Read string
	ANKI_CHECK(file->readAllText(text));

	return Error::kNone;
}

Error ResourceObject::openFileParseXml(const CString& filename, ResourceXmlDocument& xml)
{
	ResourceString txt;
	ANKI_CHECK(openFileReadAllText(filename, txt));

	ANKI_CHECK(xml.parse(txt.toCString()));

	return Error::kNone;
}

} // end namespace anki
