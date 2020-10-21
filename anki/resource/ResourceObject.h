// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/util/Atomic.h>
#include <anki/util/String.h>

namespace anki
{

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

	ResourceAllocator<U8> getAllocator() const;
	TempResourceAllocator<U8> getTempAllocator() const;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	const Atomic<I32>& getRefcount() const
	{
		return m_refcount;
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
		m_fname.create(getAllocator(), fname);
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

	ANKI_INTERNAL ANKI_USE_RESULT Error openFile(const ResourceFilename& filename, ResourceFilePtr& file);

	ANKI_INTERNAL ANKI_USE_RESULT Error openFileReadAllText(const ResourceFilename& filename, StringAuto& file);

	ANKI_INTERNAL ANKI_USE_RESULT Error openFileParseXml(const ResourceFilename& filename, XmlDocument& xml);

private:
	ResourceManager* m_manager;
	Atomic<I32> m_refcount;
	String m_fname; ///< Unique resource name.
	U64 m_uuid = 0;
};
/// @}

} // end namespace anki
