// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/String.h>

namespace anki {

// The base of all resource objects.
class ResourceObject
{
	friend class ResourceManager;
	template<typename>
	friend class ResourcePtrDeleter;

public:
	ResourceObject(CString fname, U32 uuid)
		: m_uuid(uuid)
		, m_fname(fname)
	{
		ANKI_ASSERT(uuid > 0);
		ANKI_ASSERT(!fname.isEmpty());
	}

	virtual ~ResourceObject() = default;

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	CString getFilename() const
	{
		ANKI_ASSERT(!m_fname.isEmpty());
		return m_fname.toCString();
	}

	// To check if 2 resource pointers are actually the same resource.
	U32 getUuid() const
	{
		ANKI_ASSERT(m_uuid > 0);
		return m_uuid;
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	// If true the resource has changed in the filesystem and this one is an obsolete version
	Bool isObsolete() const
	{
		return m_isObsolete.load() != 0;
	}

protected:
	Error openFile(const ResourceFilename& filename, ResourceFilePtr& file);

	Error openFileReadAllText(const ResourceFilename& filename, ResourceString& file);

	Error openFileParseXml(const ResourceFilename& filename, ResourceXmlDocument& xml);

private:
	mutable Atomic<I32> m_refcount = {0};
	mutable Atomic<U32> m_isObsolete = {0}; // If the file of the resource changed in the filesystem then this flag is 1
	U32 m_uuid = 0;
	ResourceString m_fname; // Unique resource name
};

} // end namespace anki
