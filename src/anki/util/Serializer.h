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
	/// @param x What to serialize.
	/// @param tmpAllocator A temp allocator for some memory needed.
	/// @param file The file to populate.
	template<typename T>
	ANKI_USE_RESULT Error serialize(const T& x, GenericMemoryPoolAllocator<U8> tmpAllocator, File& file)
	{
		const Error err = serializeInternal(x, tmpAllocator, file);
		if(err)
		{
			ANKI_UTIL_LOGE("There was a serialization error");
		}
		return err;
	}

	/// Write a single value. Can't call this directly.
	template<typename T>
	void doValue(CString varName, PtrSize memberOffset, const T& x)
	{
		doArray(varName, memberOffset, &x, 1);
	}

	/// Write an array of int and float values. Can't call this directly.
	template<typename T, ANKI_ENABLE(!std::is_integral<T>::value && !std::is_floating_point<T>::value)>
	void doArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = doArrayComplexType(arr, size);
		}
	}

	/// Write an array of complex types. Can't call this directly.
	template<typename T, ANKI_ENABLE(std::is_integral<T>::value || std::is_floating_point<T>::value)>
	void doArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		// Do nothing, it's already copied
	}

	/// Write a pointer. Can't call this directly.
	template<typename T>
	void doPointer(CString varName, PtrSize memberOffset, const T* ptr)
	{
		doDynamicArray(varName, memberOffset, ptr, 1);
	}

	/// Write a dynamic array of complex types. Can't call this directly.
	template<typename T, ANKI_ENABLE(!std::is_integral<T>::value && !std::is_floating_point<T>::value)>
	void doDynamicArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = doDynamicArrayComplexType(arr, size, memberOffset);
		}
	}

	/// Write a dynamic array of int and float values. Can't call this directly.
	template<typename T, ANKI_ENABLE(std::is_integral<T>::value || std::is_floating_point<T>::value)>
	void doDynamicArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = doDynamicArrayBasicType(arr, size * sizeof(T), alignof(T), memberOffset);
		}
	}

private:
	class PointerInfo
	{
	public:
		PtrSize m_filePos;
		PtrSize m_value;
	};

	File* m_file = nullptr;
	PtrSize m_eofPos; ///< A logical end of the file. Used for allocations.
	PtrSize m_beginOfDataFilePos; ///< Where the data are located in the file.
	GenericMemoryPoolAllocator<U8> m_alloc;
	DynamicArray<PointerInfo> m_pointerFilePositions; ///< Array of file positions that contain pointers.
	DynamicArray<PtrSize> m_structureFilePos;
	Error m_err = Error::NONE;

	template<typename T>
	ANKI_USE_RESULT Error doArrayComplexType(const T* arr, PtrSize size);

	template<typename T>
	ANKI_USE_RESULT Error doDynamicArrayComplexType(const T* arr, PtrSize size, PtrSize memberOffset);

	ANKI_USE_RESULT Error doDynamicArrayBasicType(const void* arr, PtrSize size, U32 alignment, PtrSize memberOffset);

	template<typename T>
	ANKI_USE_RESULT Error serializeInternal(const T& x, GenericMemoryPoolAllocator<U8> tmpAllocator, File& file);

	void check()
	{
		ANKI_ASSERT(m_file && "Can't call this function");
		ANKI_ASSERT(m_file->tell() <= m_eofPos);
	}

	template<typename T>
	static void checkStruct()
	{
		static_assert(std::is_pod<T>::value, "Only PODs are supported in this serializer");
	}
};

/// Deserializes binary files.
class BinaryDeserializer : public NonCopyable
{
public:
	/// Serialize a class.
	/// @param x The struct to read.
	/// @param allocator The allocator to use to allocate the new structures.
	/// @param file The file to read from.
	template<typename T>
	static ANKI_USE_RESULT Error deserialize(T*& x, GenericMemoryPoolAllocator<U8> allocator, File& file);

	/// Write a single value. Can't call this directly.
	template<typename T>
	void doValue(CString varName, PtrSize memberOffset, const T& x)
	{
		// Do nothing
	}

	/// Write an array. Can't call this directly.
	template<typename T>
	void doArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		// Do nothing
	}

	/// Write a pointer. Can't call this directly.
	template<typename T>
	void doPointer(CString varName, PtrSize memberOffset, const T* ptr)
	{
		// Do nothing
	}

	/// Write a dynamic array of complex types. Can't call this directly.
	template<typename T>
	void doDynamicArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		// Do nothing
	}
};
/// @}

} // end namespace anki

#include <anki/util/Serializer.inl.h>
