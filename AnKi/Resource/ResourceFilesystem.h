// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Ptr.h>

namespace anki {

// Forward
class ConfigSet;

/// @addtogroup resource
/// @{

/// Resource filesystem file. An interface that abstracts the resource file.
class ResourceFile
{
public:
	ResourceFile(HeapMemoryPool* pool)
		: m_pool(pool)
	{
		ANKI_ASSERT(pool);
	}

	ResourceFile(const ResourceFile&) = delete; // Non-copyable

	virtual ~ResourceFile()
	{
	}

	ResourceFile& operator=(const ResourceFile&) = delete; // Non-copyable

	/// Read data from the file
	virtual Error read(void* buff, PtrSize size) = 0;

	/// Read all the contents of a text file. If the file is not rewined it will probably fail
	virtual Error readAllText(StringRaii& out) = 0;

	/// Read 32bit unsigned integer. Set the endianness if the file's endianness is different from the machine's
	virtual Error readU32(U32& u) = 0;

	/// Read 32bit float. Set the endianness if the file's endianness is different from the machine's
	virtual Error readF32(F32& f) = 0;

	/// Set the position indicator to a new position
	/// @param offset Number of bytes to offset from origin
	/// @param origin Position used as reference for the offset
	virtual Error seek(PtrSize offset, FileSeekOrigin origin) = 0;

	/// Get the size of the file.
	virtual PtrSize getSize() const = 0;

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	HeapMemoryPool& getMemoryPool() const
	{
		return *m_pool;
	}

private:
	mutable HeapMemoryPool* m_pool = nullptr;
	mutable Atomic<I32> m_refcount = {0};
};

/// Resource file smart pointer.
using ResourceFilePtr = IntrusivePtr<ResourceFile>;

/// Resource filesystem.
class ResourceFilesystem
{
public:
	ResourceFilesystem() = default;

	ResourceFilesystem(const ResourceFilesystem&) = delete; // Non-copyable

	~ResourceFilesystem();

	ResourceFilesystem& operator=(const ResourceFilesystem&) = delete; // Non-copyable

	Error init(const ConfigSet& config, AllocAlignedCallback allocCallback, void* allocCallbackUserData);

	/// Search the path list to find the file. Then open the file for reading. It's thread-safe.
	Error openFile(const ResourceFilename& filename, ResourceFilePtr& file);

	/// Iterate all the filenames from all paths provided.
	template<typename TFunc>
	Error iterateAllFilenames(TFunc func) const
	{
		for(const Path& path : m_paths)
		{
			for(const String& fname : path.m_files)
			{
				ANKI_CHECK(func(fname.toCString()));
			}
		}
		return Error::kNone;
	}

#if !ANKI_TESTS
private:
#endif
	class Path
	{
	public:
		StringList m_files; ///< Files inside the directory.
		String m_path; ///< A directory or an archive.
		Bool m_isArchive = false;

		Path() = default;

		Path(const Path&) = delete; // Non-copyable

		Path(Path&& b)
		{
			*this = std::move(b);
		}

		Path& operator=(const Path&) = delete; // Non-copyable

		Path& operator=(Path&& b)
		{
			m_files = std::move(b.m_files);
			m_path = std::move(b.m_path);
			m_isArchive = b.m_isArchive;
			return *this;
		}
	};

	HeapMemoryPool m_pool;
	List<Path> m_paths;
	String m_cacheDir;

	/// Add a filesystem path or an archive. The path is read-only.
	Error addNewPath(const CString& path, const StringListRaii& excludedStrings);

	Error openFileInternal(const ResourceFilename& filename, ResourceFile*& rfile);
};
/// @}

} // end namespace anki
