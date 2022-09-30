// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Tracer.h>
#include <ZLib/contrib/minizip/unzip.h>
#if ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki {

/// C resource file
class CResourceFile final : public ResourceFile
{
public:
	File m_file;

	CResourceFile(HeapMemoryPool* pool)
		: ResourceFile(pool)
	{
	}

	Error read(void* buff, PtrSize size) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.read(buff, size);
	}

	Error readAllText(StringRaii& out) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.readAllText(out);
	}

	Error readU32(U32& u) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.readU32(u);
	}

	Error readF32(F32& f) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.readF32(f);
	}

	Error seek(PtrSize offset, FileSeekOrigin origin) override
	{
		return m_file.seek(offset, origin);
	}

	PtrSize getSize() const override
	{
		return m_file.getSize();
	}
};

/// ZIP file
class ZipResourceFile final : public ResourceFile
{
public:
	unzFile m_archive = nullptr;
	PtrSize m_size = 0;

	ZipResourceFile(HeapMemoryPool* pool)
		: ResourceFile(pool)
	{
	}

	~ZipResourceFile()
	{
		if(m_archive)
		{
			// It's open
			unzClose(m_archive);
			m_archive = nullptr;
			m_size = 0;
		}
	}

	Error open(const CString& archive, const CString& archivedFname)
	{
		// Open archive
		m_archive = unzOpen(&archive[0]);
		if(m_archive == nullptr)
		{
			ANKI_RESOURCE_LOGE("Failed to open archive");
			return Error::kFileAccess;
		}

		// Locate archived
		const int caseSensitive = 1;
		if(unzLocateFile(m_archive, &archivedFname[0], caseSensitive) != UNZ_OK)
		{
			ANKI_RESOURCE_LOGE("Failed to locate file in archive");
			return Error::kFileAccess;
		}

		// Open file
		if(unzOpenCurrentFile(m_archive) != UNZ_OK)
		{
			ANKI_RESOURCE_LOGE("unzOpenCurrentFile() failed");
			return Error::kFileAccess;
		}

		// Get size just in case
		unz_file_info zinfo;
		zinfo.uncompressed_size = 0;
		unzGetCurrentFileInfo(m_archive, &zinfo, nullptr, 0, nullptr, 0, nullptr, 0);
		m_size = zinfo.uncompressed_size;
		ANKI_ASSERT(m_size != 0);

		return Error::kNone;
	}

	void close()
	{
		if(m_archive)
		{
			unzClose(m_archive);
			m_archive = nullptr;
			m_size = 0;
		}
	}

	Error read(void* buff, PtrSize size) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);

		I64 readSize = unzReadCurrentFile(m_archive, buff, U32(size));

		if(I64(size) != readSize)
		{
			ANKI_RESOURCE_LOGE("File read failed");
			return Error::kFileAccess;
		}

		return Error::kNone;
	}

	Error readAllText(StringRaii& out) override
	{
		ANKI_ASSERT(m_size);
		out.create('?', m_size);
		return read(&out[0], m_size);
	}

	Error readU32(U32& u) override
	{
		// Assume machine and file have same endianness
		ANKI_CHECK(read(&u, sizeof(u)));
		return Error::kNone;
	}

	Error readF32(F32& u) override
	{
		// Assume machine and file have same endianness
		ANKI_CHECK(read(&u, sizeof(u)));
		return Error::kNone;
	}

	Error seek(PtrSize offset, FileSeekOrigin origin) override
	{
		// Rewind if needed
		if(origin == FileSeekOrigin::kBeginning)
		{
			if(unzCloseCurrentFile(m_archive) || unzOpenCurrentFile(m_archive))
			{
				ANKI_RESOURCE_LOGE("Rewind failed");
				return Error::kFunctionFailed;
			}
		}

		// Move forward by reading dummy data
		Array<char, 128> buff;
		while(offset != 0)
		{
			PtrSize toRead = min<PtrSize>(offset, sizeof(buff));
			ANKI_CHECK(read(&buff[0], toRead));
			offset -= toRead;
		}

		return Error::kNone;
	}

	PtrSize getSize() const override
	{
		ANKI_ASSERT(m_size > 0);
		return m_size;
	}
};

ResourceFilesystem::~ResourceFilesystem()
{
	for(Path& p : m_paths)
	{
		p.m_files.destroy(m_pool);
		p.m_path.destroy(m_pool);
	}

	m_paths.destroy(m_pool);
	m_cacheDir.destroy(m_pool);
}

Error ResourceFilesystem::init(const ConfigSet& config, AllocAlignedCallback allocCallback, void* allocCallbackUserData)
{
	m_pool.init(allocCallback, allocCallbackUserData);
	StringListRaii paths(&m_pool);
	paths.splitString(config.getRsrcDataPaths(), ':');

	StringListRaii excludedStrings(&m_pool);
	excludedStrings.splitString(config.getRsrcDataPathExcludedStrings(), ':');

	// Workaround the fact that : is used in drives in Windows
#if ANKI_OS_WINDOWS
	StringListRaii paths2(&m_pool);
	StringListRaii::Iterator it = paths.getBegin();
	while(it != paths.getEnd())
	{
		const String& s = *it;
		StringListRaii::Iterator it2 = it + 1;
		if(s.getLength() == 1 && (s[0] >= 'a' && s[0] <= 'z') || (s[0] >= 'A' && s[0] <= 'Z') && it2 != paths.getEnd())
		{
			paths2.pushBackSprintf("%s:%s", s.cstr(), it2->cstr());
			++it;
		}
		else
		{
			paths2.pushBack(s);
		}

		++it;
	}

	paths.destroy();
	paths = std::move(paths2);
#endif

	if(paths.getSize() < 1)
	{
		ANKI_RESOURCE_LOGE("Config option \"RsrcDataPaths\" is empty");
		return Error::kUserData;
	}

	for(auto& path : paths)
	{
		ANKI_CHECK(addNewPath(path.toCString(), excludedStrings));
	}

#if ANKI_OS_ANDROID
	// Add the external storage
	ANKI_CHECK(addNewPath(g_androidApp->activity->externalDataPath, excludedStrings));
#endif

	return Error::kNone;
}

Error ResourceFilesystem::addNewPath(const CString& filepath, const StringListRaii& excludedStrings)
{
	ANKI_RESOURCE_LOGV("Adding new resource path: %s", filepath.cstr());

	U32 fileCount = 0; // Count files manually because it's slower to get that number from the list
	constexpr CString extension(".ankizip");

	auto rejectPath = [&](CString p) -> Bool {
		for(const String& s : excludedStrings)
		{
			if(p.find(s) != CString::kNpos)
			{
				return true;
			}
		}

		return false;
	};

	PtrSize pos;
	Path path;
	if((pos = filepath.find(extension)) != CString::kNpos && pos == filepath.getLength() - extension.getLength())
	{
		// It's an archive

		// Open
		unzFile zfile = unzOpen(&filepath[0]);
		if(!zfile)
		{
			ANKI_RESOURCE_LOGE("Failed to open archive");
			return Error::kFileAccess;
		}

		// List files
		if(unzGoToFirstFile(zfile) != UNZ_OK)
		{
			unzClose(zfile);
			ANKI_RESOURCE_LOGE("unzGoToFirstFile() failed. Empty archive?");
			return Error::kFileAccess;
		}

		do
		{
			Array<char, 1024> filename;

			unz_file_info info;
			if(unzGetCurrentFileInfo(zfile, &info, &filename[0], filename.getSize(), nullptr, 0, nullptr, 0) != UNZ_OK)
			{
				unzClose(zfile);
				ANKI_RESOURCE_LOGE("unzGetCurrentFileInfo() failed");
				return Error::kFileAccess;
			}

			const Bool itsADir = info.uncompressed_size == 0;
			if(!itsADir && !rejectPath(&filename[0]))
			{
				path.m_files.pushBackSprintf(m_pool, "%s", &filename[0]);
				++fileCount;
			}
		} while(unzGoToNextFile(zfile) == UNZ_OK);

		unzClose(zfile);

		path.m_isArchive = true;
	}
	else
	{
		// It's simple directory

		ANKI_CHECK(walkDirectoryTree(filepath, m_pool, [&, this](const CString& fname, Bool isDir) -> Error {
			if(!isDir && !rejectPath(fname))
			{
				path.m_files.pushBackSprintf(m_pool, "%s", fname.cstr());
				++fileCount;
			}

			return Error::kNone;
		}));
	}

	ANKI_ASSERT(path.m_files.getSize() == fileCount);
	if(fileCount == 0)
	{
		ANKI_RESOURCE_LOGW("Ignoring empty resource path: %s", &filepath[0]);
	}
	else
	{
		path.m_path.sprintf(m_pool, "%s", &filepath[0]);
		m_paths.emplaceFront(m_pool, std::move(path));

		ANKI_RESOURCE_LOGI("Added new data path \"%s\" that contains %u files", &filepath[0], fileCount);
	}

	if(false)
	{
		for(const String& s : m_paths.getFront().m_files)
		{
			printf("%s\n", s.cstr());
		}
	}

	return Error::kNone;
}

Error ResourceFilesystem::openFile(const ResourceFilename& filename, ResourceFilePtr& filePtr)
{
	ResourceFile* rfile;
	Error err = openFileInternal(filename, rfile);

	if(err)
	{
		ANKI_RESOURCE_LOGE("Resource file not found: %s", filename.cstr());
		deleteInstance(m_pool, rfile);
	}
	else
	{
		ANKI_ASSERT(rfile);
		filePtr.reset(rfile);
	}

	return err;
}

Error ResourceFilesystem::openFileInternal(const ResourceFilename& filename, ResourceFile*& rfile)
{
	rfile = nullptr;

	// Search for the fname in reverse order
	for(const Path& p : m_paths)
	{
		for(const String& pfname : p.m_files)
		{
			if(pfname != filename)
			{
				continue;
			}

			// Found
			if(p.m_isArchive)
			{
				ZipResourceFile* file = newInstance<ZipResourceFile>(m_pool, &m_pool);
				rfile = file;

				ANKI_CHECK(file->open(p.m_path.toCString(), filename));
			}
			else
			{
				StringRaii newFname(&m_pool);
				newFname.sprintf("%s/%s", &p.m_path[0], &filename[0]);

				CResourceFile* file = newInstance<CResourceFile>(m_pool, &m_pool);
				rfile = file;
				ANKI_CHECK(file->m_file.open(newFname, FileOpenFlag::kRead));

#if 0
				printf("Opening asset %s\n", &newFname[0]);
#endif
			}
		}

		if(rfile)
		{
			break;
		}
	} // end for all paths

	// File not found? On Win/Linux try to find it outside the resource dirs. On Android try the archive
	if(!rfile)
	{
		CResourceFile* file = newInstance<CResourceFile>(m_pool, &m_pool);
		rfile = file;

		FileOpenFlag openFlags = FileOpenFlag::kRead;
#if ANKI_OS_ANDROID
		openFlags |= FileOpenFlag::kSpecial;
#endif
		ANKI_CHECK(file->m_file.open(filename, openFlags));

#if !ANKI_OS_ANDROID
		ANKI_RESOURCE_LOGW(
			"Loading resource outside the resource paths/archives. This is only OK for tools and debugging: %s",
			filename.cstr());
#endif
	}

	return Error::kNone;
}

} // end namespace anki
