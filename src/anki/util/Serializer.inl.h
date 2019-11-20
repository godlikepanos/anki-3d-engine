// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Serializer.h>

namespace anki
{

class alignas(ANKI_SAFE_ALIGNMENT) BinarySerializer::Header
{
public:
	Array<U8, 8> m_magic;
};

template<typename T>
Error BinarySerializer::serialize(const T& x, GenericMemoryPoolAllocator<U8> tmpAllocator, File& file)
{
	// Misc
	m_file = &file;
	m_err = Error::NONE;

	// Write header
	Header header = {};
	memcpy(&header.m_magic[0], MAGIC, sizeof(header.m_magic));
	ANKI_CHECK(m_file->write(&header, sizeof(header)));

	// Set file pos
	m_filePos = m_file->tell();
	ANKI_ASSERT(isAligned(ANKI_SAFE_ALIGNMENT, m_filePos));
	m_eofPos = m_filePos + sizeof(x);

	// Finaly, serialize
	writeValue(x);
	if(m_err)
	{
		ANKI_UTIL_LOGE("There was a serialization error");
	}

	// Done
	m_file = nullptr;
	return m_err;
}

template<typename T>
Error BinarySerializer::writeArrayComplexType(const T* arr, PtrSize size)
{
	ANKI_ASSERT(arr && size > 0);
	check();

	for(PtrSize i = 0; i < size; ++i)
	{
		ANKI_CHECK(alignFilePos(alignof(T)));
		arr[i].serialize(*this);
		ANKI_CHECK(m_err);
	}

	check();
	return Error::NONE;
}

template<typename T>
Error BinarySerializer::writeDynamicArrayComplexType(const T* arr, PtrSize size)
{
	ANKI_ASSERT(arr);
	check();

	const U32 alignment = alignof(T);

	// Write the array at the end of the file
	void* pointerPos;
	if(size == 0)
	{
		pointerPos = nullptr;
	}
	else
	{
		// Move file pos to a new place
		const PtrSize oldFilePos = m_filePos;
		m_filePos = m_eofPos;
		ANKI_CHECK(alignFilePos(alignment));
		pointerPos = numberToPtr<void*>(m_filePos);
		ANKI_CHECK(m_file->seek(m_filePos, FileSeekOrigin::BEGINNING));
		m_eofPos = m_filePos + size * sizeof(arr[0]);

		// Write data
		for(PtrSize i = 0; i < size; ++i)
		{
			arr[i].serialize(*this);
			ANKI_CHECK(m_err);
		}

		// Restore file pos
		m_filePos = oldFilePos;
		ANKI_CHECK(m_file->seek(m_filePos, FileSeekOrigin::BEGINNING));
	}

	// Write the pointer
	ANKI_CHECK(alignFilePos(alignof(void*)));
	ANKI_CHECK(m_file->write(&pointerPos, sizeof(pointerPos)));
	m_filePos += sizeof(pointerPos);
	ANKI_ASSERT(m_filePos <= m_eofPos);

	check();
	return Error::NONE;
}

} // end namespace anki
