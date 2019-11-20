// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Serializer.h>

namespace anki
{

Error BinarySerializer::alignFilePos(U32 alignment)
{
	ANKI_ASSERT(alignment <= ANKI_SAFE_ALIGNMENT);

	if(!isAligned(alignment, m_filePos))
	{
		alignRoundUp(alignment, m_filePos);
		ANKI_CHECK(m_file->seek(m_filePos, FileSeekOrigin::BEGINNING));
	}

	return Error::NONE;
}

Error BinarySerializer::writeArrayBasicType(const void* arr, PtrSize size, U32 alignment)
{
	ANKI_ASSERT(arr && size > 0);
	check();

	ANKI_CHECK(alignFilePos(alignment));

	ANKI_CHECK(m_file->write(arr, size));
	m_filePos += size;

	check();
	return Error::NONE;
}

Error BinarySerializer::writeDynamicArrayBasicType(const void* arr, PtrSize size, U32 alignment)
{
	ANKI_ASSERT(arr);
	check();

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
		ANKI_CHECK(m_file->seek(m_filePos, FileSeekOrigin::BEGINNING));
		m_eofPos = m_filePos + size;

		// Write data
		ANKI_CHECK(m_file->write(arr, size));

		// Store the pos
		pointerPos = numberToPtr<void*>(m_filePos);

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
