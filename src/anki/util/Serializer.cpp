// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Serializer.h>

namespace anki
{

Error BinarySerializer::doDynamicArrayBasicType(const void* arr, PtrSize size, U32 alignment, PtrSize memberOffset)
{
	check();

	if(size == 0)
	{
		ANKI_ASSERT(arr == nullptr);
		// Do nothing
	}
	else
	{
		ANKI_ASSERT(arr);

		// Move file pos to the end of the file (allocate space)
		PtrSize arrayFilePos = getAlignedRoundUp(alignment, m_eofPos);
		m_eofPos = arrayFilePos + size;

		// Store the pointer for later
		const PtrSize structFilePos = m_structureFilePos.getBack();
		PointerInfo pinfo;
		pinfo.m_filePos = structFilePos + memberOffset;
		pinfo.m_value = arrayFilePos - m_beginOfDataFilePos;
		m_pointerFilePositions.emplaceBack(m_alloc, pinfo);

		// Write the array
		ANKI_CHECK(m_file->seek(arrayFilePos, FileSeekOrigin::BEGINNING));
		ANKI_CHECK(m_file->write(arr, size));
	}

	check();
	return Error::NONE;
}

} // end namespace anki
