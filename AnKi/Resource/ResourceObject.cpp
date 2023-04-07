// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Xml.h>

namespace anki {

Error ResourceObject::openFile(const CString& filename, ResourceFilePtr& file)
{
	return ResourceManager::getSingleton().getFilesystem().openFile(filename, file);
}

Error ResourceObject::openFileReadAllText(const CString& filename, ResourceString& text)
{
	// Load file
	ResourceFilePtr file;
	ANKI_CHECK(ResourceManager::getSingleton().getFilesystem().openFile(filename, file));

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
