// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Xml.h>

namespace anki {

ResourceObject::ResourceObject(ResourceManager* manager)
	: m_manager(manager)
	, m_refcount(0)
{
}

ResourceObject::~ResourceObject()
{
	m_fname.destroy(getMemoryPool());
}

HeapMemoryPool& ResourceObject::getMemoryPool() const
{
	return m_manager->getMemoryPool();
}

StackMemoryPool& ResourceObject::getTempMemoryPool() const
{
	return m_manager->getTempMemoryPool();
}

Error ResourceObject::openFile(const CString& filename, ResourceFilePtr& file)
{
	return m_manager->getFilesystem().openFile(filename, file);
}

Error ResourceObject::openFileReadAllText(const CString& filename, StringRaii& text)
{
	// Load file
	ResourceFilePtr file;
	ANKI_CHECK(m_manager->getFilesystem().openFile(filename, file));

	// Read string
	ANKI_CHECK(file->readAllText(text));

	return Error::kNone;
}

Error ResourceObject::openFileParseXml(const CString& filename, XmlDocument& xml)
{
	StringRaii txt(&xml.getMemoryPool());
	ANKI_CHECK(openFileReadAllText(filename, txt));

	ANKI_CHECK(xml.parse(txt.toCString()));

	return Error::kNone;
}

const ConfigSet& ResourceObject::getConfig() const
{
	return m_manager->getConfig();
}

} // end namespace anki
