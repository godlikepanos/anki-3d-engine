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
	U32* m_darray;
	U8 m_darraySize;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& serializer, TClass self)
	{
		serializer.doArray("m_array", offsetof(ClassB, m_array), &self.m_array[0], 3);
		serializer.doValue("m_darraySize", offsetof(ClassB, m_darraySize), self.m_darraySize);
		serializer.doDynamicArray("m_darray", offsetof(ClassB, m_darray), self.m_darray, self.m_darraySize);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ClassB&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ClassB&>(serializer, *this);
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

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& serializer, TClass self)
	{
		serializer.doArray("m_array", offsetof(ClassA, m_array), &self.m_array[0], 2);
		serializer.doValue("m_u32", offsetof(ClassA, m_u32), self.m_u32);
		serializer.doValue("m_u64", offsetof(ClassA, m_u64), self.m_u64);
		serializer.doDynamicArray("m_darray", offsetof(ClassA, m_darray), self.m_darray, self.m_u32);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ClassA&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ClassA&>(serializer, *this);
	}
};

} // end namespace anki
