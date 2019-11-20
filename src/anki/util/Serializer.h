// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/File.h>

#pragma once

namespace anki
{

/// @addtogroup util_file
/// @{

/// Serializes to binary files.
class BinarySerializer : public NonCopyable
{
public:
	/// Serialize a class.
	template<typename T>
	ANKI_USE_RESULT Error serialize(const T& x, GenericMemoryPoolAllocator<U8> tmpAllocator, File& file);

	/// Write a single value. Can't call this directly.
	template<typename T>
	void writeValue(const T& x)
	{
		writeArray(&x, 1);
	}

	/// Write an array of int and float values. Can't call this directly.
	template<typename T, ANKI_ENABLE(!std::is_integral<T>::value && !std::is_floating_point<T>::value)>
	void writeArray(const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = writeArrayComplexType(arr, size);
		}
	}

	/// Write an array of complex types. Can't call this directly.
	template<typename T, ANKI_ENABLE(std::is_integral<T>::value || std::is_floating_point<T>::value)>
	void writeArray(const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = writeArrayBasicType(arr, size * sizeof(T), alignof(T));
		}
	}

	/// Write a pointer. Can't call this directly.
	template<typename T>
	void writePointer(const T* ptr)
	{
		writeDynamicArray(ptr, 1);
	}

	/// Write a dynamic array of int and float values. Can't call this directly.
	template<typename T, ANKI_ENABLE(!std::is_integral<T>::value && !std::is_floating_point<T>::value)>
	void writeDynamicArray(const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = writeDynamicArrayComplexType(arr, size);
		}
	}

	/// Write a dynamic array of complex types. Can't call this directly.
	template<typename T, ANKI_ENABLE(std::is_integral<T>::value || std::is_floating_point<T>::value)>
	void writeDynamicArray(const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = writeDynamicArrayBasicType(arr, size * sizeof(T), alignof(T));
		}
	}

private:
	class Header;

	File* m_file = nullptr;
	PtrSize m_filePos; ///< The current file position. Should be the same as m_file->tell()
	PtrSize m_eofPos; ///< A logical end of the file. Used for allocations.
	Error m_err = Error::NONE;

	static constexpr const char* MAGIC = "ANKIBIN1";

	template<typename T>
	ANKI_USE_RESULT Error writeArrayComplexType(const T* arr, PtrSize size);

	ANKI_USE_RESULT Error writeArrayBasicType(const void* arr, PtrSize size, U32 alignment);

	template<typename T>
	ANKI_USE_RESULT Error writeDynamicArrayComplexType(const T* arr, PtrSize size);

	ANKI_USE_RESULT Error writeDynamicArrayBasicType(const void* arr, PtrSize size, U32 alignment);

	ANKI_USE_RESULT Error alignFilePos(U32 alignment);

	void check()
	{
		ANKI_ASSERT(m_file && "Can't call this function");
		ANKI_ASSERT(m_filePos == m_file->tell());
		ANKI_ASSERT(m_filePos < m_eofPos);
	}
};
/// @}

} // end namespace anki

#include <anki/util/Serializer.inl.h>
