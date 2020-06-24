// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ResourceFilesystem.h>
#include <anki/util/Filesystem.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/Tracer.h>
#include <contrib/minizip/unzip.h>

namespace anki
{

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
	paths.splitString(config.getString("rsrc_dataPaths"), ':');

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

	for(auto& path : paths)
	{
		ANKI_CHECK(addNewPath(path.toCString()));
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

Error ResourceFilesystem::addNewPath(const CString& path)
{
	U32 fileCount = 0;
	static const CString extension(".ankizip");

	auto pos = path.find(extension);
	if(pos != CString::NPOS && pos == path.getLength() - extension.getLength())
	{
		// It's an archive

		// Open
		unzFile zfile = unzOpen(&path[0]);
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

		Path p;
		p.m_isArchive = true;
		p.m_path.sprintf(m_alloc, "%s", &path[0]);

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

			// If compressed size is zero then it's a dir
			if(info.uncompressed_size > 0)
			{
				p.m_files.pushBackSprintf(m_alloc, "%s", &filename[0]);
				++fileCount;
			}
		} while(unzGoToNextFile(zfile) == UNZ_OK);

		m_paths.emplaceFront(m_alloc, std::move(p));
		unzClose(zfile);
	}
	else
	{
		// It's simple directory

		m_paths.emplaceFront(m_alloc, Path());
		Path& p = m_paths.getFront();
		p.m_path.sprintf(m_alloc, "%s", &path[0]);
		p.m_isArchive = false;

		struct UserData
		{
			ResourceFilesystem* m_sys;
			U32* m_fileCount;
		} ud{this, &fileCount};

		ANKI_CHECK(walkDirectoryTree(path, &ud, [](const CString& fname, void* ud, Bool isDir) -> Error {
			if(isDir)
			{
				return Error::NONE;
			}

			UserData* udd = static_cast<UserData*>(ud);
			ResourceFilesystem* self = udd->m_sys;

			Path& p = self->m_paths.getFront();
			p.m_files.pushBackSprintf(self->m_alloc, "%s", fname.cstr());

			++(*udd->m_fileCount);
			return Error::NONE;
		}));

		if(p.m_files.getSize() < 1)
		{
			ANKI_RESOURCE_LOGE("Directory is empty: %s", &path[0]);
			return Error::USER_DATA;
		}
	}

	ANKI_RESOURCE_LOGI("Added new data path \"%s\" that contains %u files", &path[0], fileCount);
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
					newFname.sprintf("%s/%s", &p.m_path[0], &filename[0]);

					CResourceFile* file = m_alloc.newInstance<CResourceFile>(m_alloc);
					rfile = file;

					err = file->m_file.open(&newFname[0], FileOpenFlag::READ);

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
