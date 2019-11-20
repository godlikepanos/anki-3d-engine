// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <anki/util/Array.h>

namespace anki
{

/// ClassB class.
class ClassB
{
public:
	Array<U8, 3> m_array;
	U8 m_darraySize;
	U32* m_darray;

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializer.writeArray(&m_array[0], 3);
		serializer.writeValue(m_darraySize);
		serializer.writeDynamicArray(m_darray, m_darraySize);
	}
};

/// ClassA class.
class ClassA
{
public:
	Array<U8, 2> m_array;
	U32 m_u32;
	U64 m_u64;
	ClassB* m_darray;

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializer.writeArray(&m_array[0], 2);
		serializer.writeValue(m_u32);
		serializer.writeValue(m_u64);
		serializer.writeDynamicArray(m_darray, m_u32);
	}
};

} // end namespace anki
