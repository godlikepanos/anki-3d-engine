// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Tracer.h>
#include <ZLib/contrib/minizip/unzip.h>

namespace anki {

/// C resource file
class CResourceFile final : public ResourceFile
{
public:
	File m_file;

	CResourceFile(GenericMemoryPoolAllocator<U8> alloc)
		: ResourceFile(alloc)
	{
	}

	ANKI_USE_RESULT Error read(void* buff, PtrSize size) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.read(buff, size);
	}

	ANKI_USE_RESULT Error readAllText(StringAuto& out) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.readAllText(out);
	}

	ANKI_USE_RESULT Error readU32(U32& u) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.readU32(u);
	}

	ANKI_USE_RESULT Error readF32(F32& f) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);
		return m_file.readF32(f);
	}

	ANKI_USE_RESULT Error seek(PtrSize offset, FileSeekOrigin origin) override
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

	ZipResourceFile(GenericMemoryPoolAllocator<U8> alloc)
		: ResourceFile(alloc)
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

	ANKI_USE_RESULT Error open(const CString& archive, const CString& archivedFname)
	{
		// Open archive
		m_archive = unzOpen(&archive[0]);
		if(m_archive == nullptr)
		{
			ANKI_RESOURCE_LOGE("Failed to open archive");
			return Error::FILE_ACCESS;
		}

		// Locate archived
		const int caseSensitive = 1;
		if(unzLocateFile(m_archive, &archivedFname[0], caseSensitive) != UNZ_OK)
		{
			ANKI_RESOURCE_LOGE("Failed to locate file in archive");
			return Error::FILE_ACCESS;
		}

		// Open file
		if(unzOpenCurrentFile(m_archive) != UNZ_OK)
		{
			ANKI_RESOURCE_LOGE("unzOpenCurrentFile() failed");
			return Error::FILE_ACCESS;
		}

		// Get size just in case
		unz_file_info zinfo;
		zinfo.uncompressed_size = 0;
		unzGetCurrentFileInfo(m_archive, &zinfo, nullptr, 0, nullptr, 0, nullptr, 0);
		m_size = zinfo.uncompressed_size;
		ANKI_ASSERT(m_size != 0);

		return Error::NONE;
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

	ANKI_USE_RESULT Error read(void* buff, PtrSize size) override
	{
		ANKI_TRACE_SCOPED_EVENT(RSRC_FILE_READ);

		I64 readSize = unzReadCurrentFile(m_archive, buff, U32(size));

		if(I64(size) != readSize)
		{
			ANKI_RESOURCE_LOGE("File read failed");
			return Error::FILE_ACCESS;
		}

		return Error::NONE;
	}

	ANKI_USE_RESULT Error readAllText(StringAuto& out) override
	{
		ANKI_ASSERT(m_size);
		out.create('?', m_size);
		return read(&out[0], m_size);
	}

	ANKI_USE_RESULT Error readU32(U32& u) override
	{
		// Assume machine and file have same endianness
		ANKI_CHECK(read(&u, sizeof(u)));
		return Error::NONE;
	}

	ANKI_USE_RESULT Error readF32(F32& u) override
	{
		// Assume machine and file have same endianness
		ANKI_CHECK(read(&u, sizeof(u)));
		return Error::NONE;
	}

	ANKI_USE_RESULT Error seek(PtrSize offset, FileSeekOrigin origin) override
	{
		// Rewind if needed
		if(origin == FileSeekOrigin::BEGINNING)
		{
			if(unzCloseCurrentFile(m_archive) || unzOpenCurrentFile(m_archive))
			{
				ANKI_RESOURCE_LOGE("Rewind failed");
				return Error::FUNCTION_FAILED;
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

		return Error::NONE;
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
		p.m_files.destroy(m_alloc);
		p.m_path.destroy(m_alloc);
	}

	m_paths.destroy(m_alloc);
	m_cacheDir.destroy(m_alloc);
}

Error ResourceFilesystem::init(const ConfigSet& config, const CString& cacheDir)
{
	StringListAuto paths(m_alloc);
	paths.splitString(config.getRsrcDataPaths(), ':');

	StringListAuto excludedStrings(m_alloc);
	excludedStrings.splitString(config.getRsrcDataPathExcludedStrings(), ':');

	// Workaround the fact that : is used in drives in Windows
#if ANKI_OS_WINDOWS
	StringListAuto paths2(m_alloc);
	StringListAuto::Iterator it = paths.getBegin();
	while(it != paths.getEnd())
	{
		const String& s = *it;
		StringListAuto::Iterator it2 = it + 1;
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
		ANKI_RESOURCE_LOGE("Config option \"rsrc_dataPaths\" is empty");
		return Error::USER_DATA;
	}

#if ANKI_OS_ANDROID
	// Add the files of the .apk
	ANKI_CHECK(addNewPath("APK package", excludedStrings, true));
#endif

	for(auto& path : paths)
	{
		ANKI_CHECK(addNewPath(path.toCString(), excludedStrings));
	}

	addCachePath(cacheDir);

	return Error::NONE;
}

void ResourceFilesystem::addCachePath(const CString& path)
{
	Path p;
	p.m_path.create(m_alloc, path);
	p.m_isCache = true;

	m_paths.emplaceBack(m_alloc, std::move(p));
}

Error ResourceFilesystem::addNewPath(const CString& filepath, const StringListAuto& excludedStrings, Bool special)
{
	U32 fileCount = 0; // Count files manually because it's slower to get that number from the list
	static const CString extension(".ankizip");

	auto rejectPath = [&](CString p) -> Bool {
		for(const String& s : excludedStrings)
		{
			if(p.find(s) != CString::NPOS)
			{
				return true;
			}
		}

		return false;
	};

	PtrSize pos;
	Path path;
	if(special)
	{
		// Android apk, read the file that contains the directory structure

		// Read the file
		File dirStructure;
		ANKI_CHECK(dirStructure.open("DirStructure.txt", FileOpenFlag::READ | FileOpenFlag::SPECIAL));
		StringAuto txt(m_alloc);
		ANKI_CHECK(dirStructure.readAllText(txt));

		StringListAuto filenames(m_alloc);
		filenames.splitString(txt, '\n');

		// Create the Path
		for(const String& filename : filenames)
		{
			if(!rejectPath(filename))
			{
				path.m_files.pushBack(m_alloc, filename);
				++fileCount;
			}
		}

		path.m_isSpecial = true;
	}
	else if((pos = filepath.find(extension)) != CString::NPOS && pos == filepath.getLength() - extension.getLength())
	{
		// It's an archive

		// Open
		unzFile zfile = unzOpen(&filepath[0]);
		if(!zfile)
		{
			ANKI_RESOURCE_LOGE("Failed to open archive");
			return Error::FILE_ACCESS;
		}

		// List files
		if(unzGoToFirstFile(zfile) != UNZ_OK)
		{
			unzClose(zfile);
			ANKI_RESOURCE_LOGE("unzGoToFirstFile() failed. Empty archive?");
			return Error::FILE_ACCESS;
		}

		do
		{
			Array<char, 1024> filename;

			unz_file_info info;
			if(unzGetCurrentFileInfo(zfile, &info, &filename[0], filename.getSize(), nullptr, 0, nullptr, 0) != UNZ_OK)
			{
				unzClose(zfile);
				ANKI_RESOURCE_LOGE("unzGetCurrentFileInfo() failed");
				return Error::FILE_ACCESS;
			}

			const Bool itsADir = info.uncompressed_size == 0;
			if(!itsADir && !rejectPath(&filename[0]))
			{
				path.m_files.pushBackSprintf(m_alloc, "%s", &filename[0]);
				++fileCount;
			}
		} while(unzGoToNextFile(zfile) == UNZ_OK);

		unzClose(zfile);

		path.m_isArchive = true;
	}
	else
	{
		// It's simple directory

		ANKI_CHECK(walkDirectoryTree(filepath, m_alloc, [&, this](const CString& fname, Bool isDir) -> Error {
			if(!isDir && !rejectPath(fname))
			{
				path.m_files.pushBackSprintf(m_alloc, "%s", fname.cstr());
				++fileCount;
			}

			return Error::NONE;
		}));
	}

	ANKI_ASSERT(path.m_files.getSize() == fileCount);
	if(fileCount == 0)
	{
		ANKI_RESOURCE_LOGW("Ignoring empty resource path: %s", &filepath[0]);
	}
	else
	{
		path.m_path.sprintf(m_alloc, "%s", &filepath[0]);
		m_paths.emplaceFront(m_alloc, std::move(path));

		ANKI_RESOURCE_LOGI("Added new data path \"%s\" that contains %u files", &filepath[0], fileCount);
	}

	return Error::NONE;
}

Error ResourceFilesystem::openFile(const ResourceFilename& filename, ResourceFilePtr& filePtr)
{
	ResourceFile* rfile = nullptr;
	Error err = Error::NONE;

	// Search for the fname in reverse order
	for(const Path& p : m_paths)
	{
		// Check if it's cache
		if(p.m_isCache)
		{
			StringAuto newFname(m_alloc);
			newFname.sprintf("%s/%s", &p.m_path[0], &filename[0]);

			if(fileExists(newFname.toCString()))
			{
				// In cache

				CResourceFile* file = m_alloc.newInstance<CResourceFile>(m_alloc);
				rfile = file;

				err = file->m_file.open(&newFname[0], FileOpenFlag::READ);
			}
		}
		else
		{
			// In data path or archive

			for(const String& pfname : p.m_files)
			{
				if(pfname != filename)
				{
					continue;
				}

				// Found
				if(p.m_isArchive)
				{
					ZipResourceFile* file = m_alloc.newInstance<ZipResourceFile>(m_alloc);
					rfile = file;

					err = file->open(p.m_path.toCString(), filename);
				}
				else
				{
					StringAuto newFname(m_alloc);
					if(!p.m_isSpecial)
					{
						newFname.sprintf("%s/%s", &p.m_path[0], &filename[0]);
					}
					else
					{
						newFname.sprintf("%s", &filename[0]);
					}

					CResourceFile* file = m_alloc.newInstance<CResourceFile>(m_alloc);
					rfile = file;

					FileOpenFlag fopenFlags = FileOpenFlag::READ;
					if(p.m_isSpecial)
					{
						fopenFlags |= FileOpenFlag::SPECIAL;
					}

					err = file->m_file.open(&newFname[0], fopenFlags);

#if 0
					printf("Opening asset %s\n", &newFname[0]);
#endif
				}
			}
		} // end if cache

		if(rfile)
		{
			break;
		}
	} // end for all paths

	if(err)
	{
		m_alloc.deleteInstance(rfile);
		return err;
	}

	// File not found? Try to find it outside the resource dirs
	if(!rfile && fileExists(filename))
	{
		CResourceFile* file = m_alloc.newInstance<CResourceFile>(m_alloc);
		err = file->m_file.open(filename, FileOpenFlag::READ);
		if(err)
		{
			m_alloc.deleteInstance(file);
			return err;
		}

		rfile = file;
		ANKI_RESOURCE_LOGW(
			"Loading resource outside the resource paths/archives. This is only OK for tools and debugging: %s",
			filename.cstr());
	}

	if(!rfile)
	{
		ANKI_RESOURCE_LOGE("File not found: %s", &filename[0]);
		return Error::USER_DATA;
	}

	// Done
	filePtr.reset(rfile);
	return Error::NONE;
}

} // end namespace anki
