// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourceFilesystem.h"
#include "anki/util/Filesystem.h"
#include <contrib/minizip/unzip.h>

namespace anki {

//==============================================================================
// File classes                                                                =
//==============================================================================

/// C resource file
class CResourceFile final: public ResourceFile
{
public:
	File m_file;

	CResourceFile(GenericMemoryPoolAllocator<U8> alloc)
		: ResourceFile(alloc)
	{}

	ANKI_USE_RESULT Error read(void* buff, PtrSize size)
	{
		return m_file.read(buff, size);
	}

	ANKI_USE_RESULT Error readAllText(
		GenericMemoryPoolAllocator<U8> alloc, String& out)
	{
		return m_file.readAllText(alloc, out);
	}

	ANKI_USE_RESULT Error readU32(U32& u)
	{
		return m_file.readU32(u);
	}

	ANKI_USE_RESULT Error readF32(F32& f)
	{
		return m_file.readF32(f);
	}

	ANKI_USE_RESULT Error seek(PtrSize offset, SeekOrigin origin)
	{
		return m_file.seek(offset, origin);
	}
};

/// ZIP file
class ZipResourceFile final: public ResourceFile
{
public:
	unzFile m_archive = nullptr;
	PtrSize m_size = 0;

	ZipResourceFile(GenericMemoryPoolAllocator<U8> alloc)
		: ResourceFile(alloc)
	{}

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

	ANKI_USE_RESULT Error open(
		const CString& archive,
		const CString& archivedFname)
	{
		// Open archive
		m_archive = unzOpen(&archive[0]);
		if(m_archive == nullptr)
		{
			ANKI_LOGE("Failed to open archive");
			return ErrorCode::FILE_ACCESS;
		}

		// Locate archived
		const int caseSensitive = 1;
		if(unzLocateFile(m_archive, &archivedFname[0], caseSensitive) != UNZ_OK)
		{
			ANKI_LOGE("Failed to locate file in archive");
			return ErrorCode::FILE_ACCESS;
		}

		// Open file
		if(unzOpenCurrentFile(m_archive) != UNZ_OK)
		{
			ANKI_LOGE("unzOpenCurrentFile() failed");
			return ErrorCode::FILE_ACCESS;
		}

		// Get size just in case
		unz_file_info zinfo;
		zinfo.uncompressed_size = 0;
		unzGetCurrentFileInfo(
			m_archive, &zinfo, nullptr, 0, nullptr, 0, nullptr, 0);
		m_size = zinfo.uncompressed_size;
		ANKI_ASSERT(m_size != 0);

		return ErrorCode::NONE;
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

	ANKI_USE_RESULT Error read(void* buff, PtrSize size)
	{
		I64 readSize = unzReadCurrentFile(m_archive, buff, size);

		if(I64(size) != readSize)
		{
			ANKI_LOGE("File read failed");
			return ErrorCode::FILE_ACCESS;
		}

		return ErrorCode::NONE;
	}

	ANKI_USE_RESULT Error readAllText(
		GenericMemoryPoolAllocator<U8> alloc, String& out)
	{
		ANKI_ASSERT(m_size);
		out.create(alloc, '?', m_size);
		return read(&out[0], m_size);
	}

	ANKI_USE_RESULT Error readU32(U32& u)
	{
		// Assume machine and file have same endianness
		ANKI_CHECK(read(&u, sizeof(u)));
		return ErrorCode::NONE;
	}

	ANKI_USE_RESULT Error readF32(F32& u)
	{
		// Assume machine and file have same endianness
		ANKI_CHECK(read(&u, sizeof(u)));
		return ErrorCode::NONE;
	}

	ANKI_USE_RESULT Error seek(PtrSize offset, SeekOrigin origin)
	{
		// Rewind if needed
		if(origin == SeekOrigin::BEGINNING)
		{
			if(unzCloseCurrentFile(m_archive) || unzOpenCurrentFile(m_archive))
			{
				ANKI_LOGE("Rewind failed");
				return ErrorCode::FUNCTION_FAILED;
			}
		}

		// Move forward by reading dummy data
		Array<char, 128> buff;
		while(offset != 0)
		{
			PtrSize toRead = min(offset, sizeof(buff));
			ANKI_CHECK(read(&buff[0], toRead));
			offset -= toRead;
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// ResourceFilesystem                                                          =
//==============================================================================

//==============================================================================
ResourceFilesystem::~ResourceFilesystem()
{
	for(Path& p : m_paths)
	{
		p.m_files.destroy(m_alloc);
		p.m_path.destroy(m_alloc);
	}

	m_paths.destroy(m_alloc);
}

//==============================================================================
Error ResourceFilesystem::addNewPath(const CString& path)
{
	static const CString extension(".ankizip");

	auto pos = path.find(extension);
	if(pos != CString::NPOS && pos == path.getLength() - extension.getLength())
	{
		// It's an archive

		// Open
		unzFile zfile = unzOpen(&path[0]);
		if(!zfile)
		{
			ANKI_LOGE("Failed to open archive");
			return ErrorCode::FILE_ACCESS;
		}

		// List files
		if(unzGoToFirstFile(zfile) != UNZ_OK)
		{
			unzClose(zfile);
			ANKI_LOGE("unzGoToFirstFile() failed. Empty archive?");
			return ErrorCode::FILE_ACCESS;
		}

		Path p;
		p.m_isArchive = true;
		p.m_path.sprintf(m_alloc, "%s", &path[0]);

		do
		{
			Array<char, 1024> filename;

			unz_file_info info;
			if(unzGetCurrentFileInfo(zfile, &info, &filename[0],
				filename.getSize(), nullptr, 0, nullptr, 0) != UNZ_OK)
			{
				unzClose(zfile);
				ANKI_LOGE("unzGetCurrentFileInfo() failed");
				return ErrorCode::FILE_ACCESS;
			}

			// If compressed size is zero then it's a dir
			if(info.uncompressed_size > 0)
			{
				p.m_files.pushBackSprintf(m_alloc, "%s", &filename[0]);
			}
		} while(unzGoToNextFile(zfile) == UNZ_OK);

		m_paths.emplaceFront(m_alloc, std::move(p));
		unzClose(zfile);
	}
	else
	{
		// It's simple directory

		m_paths.emplaceFront(m_alloc, std::move(Path()));
		m_paths.getBack().m_path.sprintf(m_alloc, "%s", &path[0]);
		m_paths.getBack().m_isArchive = false;

		ANKI_CHECK(walkDirectoryTree(path, this,
			[](const CString& fname, void* ud, Bool isDir) -> Error
		{
			if(isDir)
			{
				return ErrorCode::NONE;
			}

			ResourceFilesystem* self = static_cast<ResourceFilesystem*>(ud);

			Path& p = self->m_paths.getBack();
			p.m_files.pushBackSprintf(self->m_alloc, "%s", &fname[0]);
			return ErrorCode::NONE;
		}));

		if(m_paths.getBack().m_files.getSize() < 1)
		{
			ANKI_LOGE("Directory is empty: %s", &path[0]);
			return ErrorCode::USER_DATA;
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error ResourceFilesystem::openFile(
	const CString& ufilename, ResourceFilePtr& filePtr)
{
	// Search for the fname in reverse order
	for(const Path& p : m_paths)
	{
		for(const String& filename : p.m_files)
		{
			if(filename != ufilename)
			{
				continue;
			}

			ResourceFile* rfile;
			Error err = ErrorCode::NONE;

			// Found
			if(p.m_isArchive)
			{
				ZipResourceFile* file =
					m_alloc.newInstance<ZipResourceFile>(m_alloc);
				rfile = file;

				err = file->open(p.m_path.toCString(), filename.toCString());
			}
			else
			{
				StringAuto newFname(m_alloc);
				newFname.sprintf("%s/%s", &p.m_path[0], &filename[0]);

				CResourceFile* file =
					m_alloc.newInstance<CResourceFile>(m_alloc);
				rfile = file;

				err = file->m_file.open(&newFname[0], File::OpenFlag::READ);
			}

			if(err)
			{
				m_alloc.deleteInstance(rfile);
				return err;
			}

			// Done
			filePtr.reset(rfile);
			return ErrorCode::NONE;
		}
	}

	ANKI_LOGE("File not found: %s", &ufilename[0]);
	return ErrorCode::USER_DATA;
}

} // end namespace anki
