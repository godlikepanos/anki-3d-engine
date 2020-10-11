// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/File.h>
#include <anki/util/WeakArray.h>

#pragma once

namespace anki
{

/// @addtogroup util_file
/// @{

#define _ANKI_SIMPLE_TYPE (std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_enum<T>::value)

/// Serialize functor. Used to add serialization code to classes that you can't add a serialize() method.
template<typename T>
class SerializeFunctor
{
public:
	template<typename TSerializer>
	void operator()(const T& x, TSerializer& serializer)
	{
		x.serialize(serializer);
	}
};

/// Deserialize functor. Used to add deserialization code to classes that you can't add a serialize() method.
template<typename T>
class DeserializeFunctor
{
public:
	template<typename TDeserializer>
	void operator()(T& x, TDeserializer& deserializer)
	{
		x.deserialize(deserializer);
	}
};

/// Specialization for WeakArray.
template<typename T, typename TSize>
class SerializeFunctor<WeakArray<T, TSize>>
{
public:
	template<typename TSerializer>
	void operator()(const WeakArray<T, TSize>& x, TSerializer& serializer)
	{
		const TSize size = x.getSize();
		serializer.doDynamicArray("m_array", 0, (x.getSize()) ? &x[0] : nullptr, size);
		serializer.doValue("m_size", sizeof(void*), size);
	}
};

/// Specialization for WeakArray.
template<typename T, typename TSize>
class DeserializeFunctor<WeakArray<T, TSize>>
{
public:
	template<typename TDeserializer>
	void operator()(WeakArray<T, TSize>& x, TDeserializer& deserializer)
	{
		TSize size;
		deserializer.doValue("m_size", sizeof(void*), size);
		T* arr = nullptr;
		if(size > 0)
		{
			deserializer.doDynamicArray("m_array", 0, arr, size);
		}
		x = WeakArray<T, TSize>(arr, size);
	}
};

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

	/// Write an array of complex values. Can't call this directly.
	template<typename T, ANKI_ENABLE(!_ANKI_SIMPLE_TYPE)>
	void doArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = doArrayComplexType(arr, size, memberOffset);
		}
	}

	/// Write an array of int or float types. Can't call this directly.
	template<typename T, ANKI_ENABLE(_ANKI_SIMPLE_TYPE)>
	void doArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		// Do nothing, it's already copied
	}

	/// Write a pointer. Can't call this directly.
	template<typename T>
	void doPointer(CString varName, PtrSize memberOffset, const T* ptr)
	{
		doDynamicArray(varName, memberOffset, ptr, (ptr) ? 1 : 0);
	}

	/// Write a dynamic array of complex types. Can't call this directly.
	template<typename T, ANKI_ENABLE(!_ANKI_SIMPLE_TYPE)>
	void doDynamicArray(CString varName, PtrSize memberOffset, const T* arr, PtrSize size)
	{
		if(!m_err)
		{
			m_err = doDynamicArrayComplexType(arr, size, memberOffset);
		}
	}

	/// Write a dynamic array of int and float values. Can't call this directly.
	template<typename T, ANKI_ENABLE(_ANKI_SIMPLE_TYPE)>
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
		PtrSize m_filePos; ///< Pointer location inside the file.
		PtrSize m_value; ///< Where it points to. It's an offset after the header.
	};

	File* m_file = nullptr;
	PtrSize m_eofPos; ///< A logical end of the file. Used for allocations.
	PtrSize m_beginOfDataFilePos; ///< Where the data are located in the file.
	GenericMemoryPoolAllocator<U8> m_alloc;
	DynamicArray<PointerInfo> m_pointerFilePositions; ///< Array of file positions that contain pointers.
	DynamicArray<PtrSize> m_structureFilePos;
	Error m_err = Error::NONE;

	template<typename T>
	ANKI_USE_RESULT Error doArrayComplexType(const T* arr, PtrSize size, PtrSize memberOffset);

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
		static_assert(!std::is_polymorphic<T>::value, "Only PODs are supported in this serializer");
		static_assert(alignof(T) <= ANKI_SAFE_ALIGNMENT, "Alignments can't exceed ANKI_SAFE_ALIGNMENT");
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

	/// Read a single value. Can't call this directly.
	template<typename T>
	void doValue(CString varName, PtrSize memberOffset, T& x)
	{
		// Do nothing
	}

	/// Read an array. Can't call this directly.
	template<typename T>
	void doArray(CString varName, PtrSize memberOffset, T* arr, PtrSize size)
	{
		// Do nothing
	}

	/// Read a pointer. Can't call this directly.
	template<typename T>
	void doPointer(CString varName, PtrSize memberOffset, T* ptr)
	{
		// Do nothing
	}

	/// Read a dynamic array of complex types. Can't call this directly.
	template<typename T>
	void doDynamicArray(CString varName, PtrSize memberOffset, T* arr, PtrSize size)
	{
		// Do nothing
	}
};
/// @}

#undef _ANKI_SIMPLE_TYPE

} // end namespace anki

#include <anki/util/Serializer.inl.h>
