// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Serializer.h>

namespace anki {

namespace detail {

class alignas(ANKI_SAFE_ALIGNMENT) BinarySerializerHeader
{
public:
	Array<U8, 8> m_magic;
	PtrSize m_dataSize;
	PtrSize m_pointerArrayFilePosition; ///< Points to an array of file positions that contain pointers.
	PtrSize m_pointerCount; ///< The size of the above.
};

inline constexpr const char* kBinarySerializerMagic = "ANKIBIN1";

} // end namespace detail

template<typename T>
Error BinarySerializer::serializeInternal(const T& x, BaseMemoryPool& tmpPool, File& file)
{
	checkStruct<T>();

	// Misc
	m_file = &file;
	ANKI_ASSERT(m_file->tell() == 0);

	m_err = Error::kNone;
	m_pool = &tmpPool;

	// Write the empty header (will be filled later)
	detail::BinarySerializerHeader header = {};
	const PtrSize headerFilePos = m_file->tell();
	ANKI_CHECK(m_file->write(&header, sizeof(header)));

	// Set EOF file pos
	const PtrSize dataFilePos = headerFilePos + sizeof(header);
	ANKI_ASSERT(isAligned(ANKI_SAFE_ALIGNMENT, dataFilePos));
	m_eofPos = dataFilePos + sizeof(T);
	m_beginOfDataFilePos = dataFilePos;

	// Finaly, serialize
	ANKI_CHECK(m_file->write(&x, sizeof(T)));
	m_structureFilePos.emplaceBack(tmpPool, dataFilePos);
	doValue("root", 0, x);
	m_structureFilePos.popBack(tmpPool);
	if(m_err)
	{
		return m_err;
	}

	// Write all pointers. Do that now and not while writing the actual shader in order to avoid the file seeks
	DynamicArrayRaii<PtrSize> pointerFilePositions(&tmpPool);
	for(const PointerInfo& pointer : m_pointerFilePositions)
	{
		ANKI_CHECK(m_file->seek(pointer.m_filePos, FileSeekOrigin::kBeginning));
		ANKI_CHECK(m_file->write(&pointer.m_value, sizeof(pointer.m_value)));

		const PtrSize offsetAfterHeader = pointer.m_filePos - m_beginOfDataFilePos;
		ANKI_ASSERT(offsetAfterHeader + sizeof(void*) <= m_eofPos - m_beginOfDataFilePos);
		pointerFilePositions.emplaceBack(offsetAfterHeader);
	}

	// Write the pointer offsets
	if(pointerFilePositions.getSize() > 0)
	{
		ANKI_CHECK(m_file->seek(m_eofPos, FileSeekOrigin::kBeginning));
		ANKI_CHECK(m_file->write(&pointerFilePositions[0], pointerFilePositions.getSizeInBytes()));
		header.m_pointerCount = pointerFilePositions.getSize();
		header.m_pointerArrayFilePosition = m_eofPos;
	}

	// Write the header
	memcpy(&header.m_magic[0], detail::kBinarySerializerMagic, sizeof(header.m_magic));
	header.m_dataSize = m_eofPos - dataFilePos;
	ANKI_CHECK(m_file->seek(headerFilePos, FileSeekOrigin::kBeginning));
	ANKI_CHECK(m_file->write(&header, sizeof(header)));

	// Done
	m_file = nullptr;
	m_pointerFilePositions.destroy(tmpPool);
	m_pool = nullptr;
	return Error::kNone;
}

template<typename T>
Error BinarySerializer::doArrayComplexType(const T* arr, PtrSize size, PtrSize memberOffset)
{
	ANKI_ASSERT(arr && size > 0);
	check();
	checkStruct<T>();

	// Serialize pointers
	PtrSize structFilePos = m_structureFilePos.getBack() + memberOffset;
	for(PtrSize i = 0; i < size; ++i)
	{
		m_structureFilePos.emplaceBack(*m_pool, structFilePos);

		// Serialize the pointers
		SerializeFunctor<T>()(arr[i], *this);
		ANKI_CHECK(m_err);

		// Advance file pos
		m_structureFilePos.popBack(*m_pool);
		structFilePos += sizeof(T);
	}

	check();
	return Error::kNone;
}

template<typename T>
Error BinarySerializer::doDynamicArrayComplexType(const T* arr, PtrSize size, PtrSize memberOffset)
{
	check();
	checkStruct<T>();

	if(size == 0)
	{
		ANKI_ASSERT(arr == nullptr);
		// Do nothing
	}
	else
	{
		ANKI_ASSERT(arr);

		// Move file pos to the end of the file (allocate space)
		PtrSize arrayFilePos = getAlignedRoundUp(alignof(T), m_eofPos);
		m_eofPos = arrayFilePos + size * sizeof(T);

		// Store the pointer for later
		const PtrSize structFilePos = m_structureFilePos.getBack();
		PointerInfo pinfo;
		pinfo.m_filePos = structFilePos + memberOffset;
		pinfo.m_value = arrayFilePos - m_beginOfDataFilePos;
		m_pointerFilePositions.emplaceBack(*m_pool, pinfo);

		// Write the structures
		ANKI_CHECK(m_file->seek(arrayFilePos, FileSeekOrigin::kBeginning));
		ANKI_CHECK(m_file->write(&arr[0], sizeof(T) * size));

		// Basically serialize pointers
		for(PtrSize i = 0; i < size && !m_err; ++i)
		{
			m_structureFilePos.emplaceBack(*m_pool, arrayFilePos);

			SerializeFunctor<T>()(arr[i], *this);

			m_structureFilePos.popBack(*m_pool);
			arrayFilePos += sizeof(T);
		}
		ANKI_CHECK(m_err);
	}

	check();
	return Error::kNone;
}

template<typename T, typename TFile>
Error BinaryDeserializer::deserialize(T*& x, BaseMemoryPool& pool, TFile& file)
{
	x = nullptr;

	detail::BinarySerializerHeader header;
	ANKI_CHECK(file.read(&header, sizeof(header)));
	const PtrSize dataFilePos = sizeof(header);

	// Sanity checks
	{
		if(memcmp(&header.m_magic[0], detail::kBinarySerializerMagic, 8) != 0)
		{
			ANKI_UTIL_LOGE("Wrong magic work in header");
			return Error::kUserData;
		}

		if(header.m_dataSize < sizeof(T))
		{
			ANKI_UTIL_LOGE("Wrong data size");
			return Error::kUserData;
		}

		const PtrSize expectedSizeAfterHeader = header.m_dataSize + header.m_pointerCount * sizeof(void*);
		const PtrSize actualSizeAfterHeader = file.getSize() - dataFilePos;
		if(expectedSizeAfterHeader > actualSizeAfterHeader)
		{
			ANKI_UTIL_LOGE("File size doesn't match expectations");
			return Error::kUserData;
		}
	}

	// Allocate & read data
	U8* const baseAddress = static_cast<U8*>(pool.allocate(header.m_dataSize, ANKI_SAFE_ALIGNMENT));
	ANKI_CHECK(file.read(baseAddress, header.m_dataSize));

	// Fix pointers
	if(header.m_pointerCount)
	{
		ANKI_CHECK(file.seek(header.m_pointerArrayFilePosition, FileSeekOrigin::kBeginning));
		for(PtrSize i = 0; i < header.m_pointerCount; ++i)
		{
			// Read the location of the pointer
			PtrSize offsetFromBeginOfData;
			ANKI_CHECK(file.read(&offsetFromBeginOfData, sizeof(offsetFromBeginOfData)));
			if(offsetFromBeginOfData >= header.m_dataSize)
			{
				ANKI_UTIL_LOGE("Corrupt pointer");
				return Error::kUserData;
			}

			// Add to the location the actual base address
			U8* ptrLocation = baseAddress + offsetFromBeginOfData;
			PtrSize& ptrValue = *reinterpret_cast<PtrSize*>(ptrLocation);
			if(ptrValue >= header.m_dataSize)
			{
				ANKI_UTIL_LOGE("Corrupt pointer");
				return Error::kUserData;
			}

			ptrValue += ptrToNumber(baseAddress);
		}
	}

	// Done
	x = reinterpret_cast<T*>(baseAddress);
	return Error::kNone;
}

} // end namespace anki
