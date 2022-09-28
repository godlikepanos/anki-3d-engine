// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/String.h>

namespace anki {

// Forward
class XmlDocument;

/// @addtogroup resource
/// @{

/// The base of all resource objects.
class ResourceObject
{
	friend class ResourceManager;

public:
	ResourceObject(ResourceManager* manager);

	virtual ~ResourceObject();

	/// @privatesection
	ResourceManager& getManager() const
	{
		return *m_manager;
	}

	HeapMemoryPool& getMemoryPool() const;
	StackMemoryPool& getTempMemoryPool() const;

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

	CString getFilename() const
	{
		ANKI_ASSERT(!m_fname.isEmpty());
		return m_fname.toCString();
	}

	// Internals:

	ANKI_INTERNAL const ConfigSet& getConfig() const;

	ANKI_INTERNAL void setFilename(const CString& fname)
	{
		ANKI_ASSERT(m_fname.isEmpty());
		m_fname.create(getMemoryPool(), fname);
	}

	ANKI_INTERNAL void setUuid(U64 uuid)
	{
		ANKI_ASSERT(uuid > 0);
		m_uuid = uuid;
	}

	/// To check if 2 resource pointers are actually the same resource.
	ANKI_INTERNAL U64 getUuid() const
	{
		ANKI_ASSERT(m_uuid > 0);
		return m_uuid;
	}

	ANKI_INTERNAL Error openFile(const ResourceFilename& filename, ResourceFilePtr& file);

	ANKI_INTERNAL Error openFileReadAllText(const ResourceFilename& filename, StringRaii& file);

	ANKI_INTERNAL Error openFileParseXml(const ResourceFilename& filename, XmlDocument& xml);

private:
	ResourceManager* m_manager;
	mutable Atomic<I32> m_refcount;
	String m_fname; ///< Unique resource name.
	U64 m_uuid = 0;
};
/// @}

} // end namespace anki
