// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/String.h>

namespace anki {

/// @addtogroup resource
/// @{

/// The base of all resource objects.
class ResourceObject
{
	friend class ResourceManager;
	template<typename>
	friend class ResourcePtrDeleter;

public:
	virtual ~ResourceObject() = default;

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

	ANKI_INTERNAL void setFilename(const CString& fname)
	{
		ANKI_ASSERT(m_fname.isEmpty());
		m_fname = fname;
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

	ANKI_INTERNAL Error openFileReadAllText(const ResourceFilename& filename, ResourceString& file);

	ANKI_INTERNAL Error openFileParseXml(const ResourceFilename& filename, ResourceXmlDocument& xml);

private:
	mutable Atomic<I32> m_refcount = {0};
	ResourceString m_fname; ///< Unique resource name.
	U64 m_uuid = 0;
};
/// @}

} // end namespace anki
