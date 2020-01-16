// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	WeakArray<U32> m_darray;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_array", offsetof(ClassB, m_array), &self.m_array[0], 3);
		s.doValue("m_darray", offsetof(ClassB, m_darray), self.m_darray);
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
	WeakArray<ClassB> m_darray;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_array", offsetof(ClassA, m_array), &self.m_array[0], 2);
		s.doValue("m_u32", offsetof(ClassA, m_u32), self.m_u32);
		s.doValue("m_u64", offsetof(ClassA, m_u64), self.m_u64);
		s.doValue("m_darray", offsetof(ClassA, m_darray), self.m_darray);
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
