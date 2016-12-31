// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/util/String.h>
#include <anki/util/StringList.h>
#include <anki/util/File.h>
#include <anki/util/Ptr.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup resource
/// @{

/// Resource filesystem file. An interface that abstracts the resource file.
class ResourceFile : public NonCopyable
{
public:
	using SeekOrigin = File::SeekOrigin;

	ResourceFile(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	virtual ~ResourceFile()
	{
	}

	/// Read data from the file
	virtual ANKI_USE_RESULT Error read(void* buff, PtrSize size) = 0;

	/// Read all the contents of a text file. If the file is not rewined it will probably fail
	virtual ANKI_USE_RESULT Error readAllText(GenericMemoryPoolAllocator<U8> alloc, String& out) = 0;

	/// Read 32bit unsigned integer. Set the endianness if the file's endianness is different from the machine's
	virtual ANKI_USE_RESULT Error readU32(U32& u) = 0;

	/// Read 32bit float. Set the endianness if the file's endianness is different from the machine's
	virtual ANKI_USE_RESULT Error readF32(F32& f) = 0;

	/// Set the position indicator to a new position
	/// @param offset Number of bytes to offset from origin
	/// @param origin Position used as reference for the offset
	virtual ANKI_USE_RESULT Error seek(PtrSize offset, SeekOrigin origin) = 0;

	/// Get the size of the file.
	virtual PtrSize getSize() const = 0;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	GenericMemoryPoolAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	Atomic<I32> m_refcount = {0};
};

/// Resource file smart pointer.
using ResourceFilePtr = IntrusivePtr<ResourceFile>;

/// Resource filesystem.
class ResourceFilesystem : public NonCopyable
{
public:
	ResourceFilesystem(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~ResourceFilesystem();

	ANKI_USE_RESULT Error init(const ConfigSet& config, const CString& cacheDir);

	/// Search the path list to find the file. Then open the file for reading.
	ANKI_USE_RESULT Error openFile(const ResourceFilename& filename, ResourceFilePtr& file);

private:
	class Path : public NonCopyable
	{
	public:
		StringList m_files; ///< Files inside the directory.
		String m_path; ///< A directory or an archive.
		Bool8 m_isArchive = false;
		Bool8 m_isCache = false;

		Path() = default;

		Path(Path&& b)
			: m_files(std::move(b.m_files))
			, m_path(std::move(b.m_path))
			, m_isArchive(std::move(b.m_isArchive))
			, m_isCache(std::move(b.m_isCache))
		{
		}

		Path& operator=(Path&& b)
		{
			m_files = std::move(b.m_files);
			m_path = std::move(b.m_path);
			m_isArchive = std::move(b.m_isArchive);
			m_isCache = std::move(b.m_isCache);
			return *this;
		}
	};

	GenericMemoryPoolAllocator<U8> m_alloc;
	List<Path> m_paths;
	String m_cacheDir;

	/// Add a filesystem path or an archive. The path is read-only.
	ANKI_USE_RESULT Error addNewPath(const CString& path);

	void addCachePath(const CString& path);
};
/// @}

} // end namespace anki
