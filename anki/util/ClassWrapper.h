// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup util_misc
/// @{

/// A wrapper template to compensate for the fact that some classes get initialized in the constructor but that's not
/// always desirable. One solution is to use a pointer and dynamic allocation but that creates an indirection and it
/// might cause bad cache locality. With this wrapper is scenario is avoided.
template<typename TClass>
class ClassWrapper
{
public:
	ClassWrapper()
	{
	}

	/// Call the constructor of the TClass.
	template<typename... TArgs>
	void init(TArgs&&... args)
	{
		::new(&m_data[0]) TClass(std::forward<TArgs>(args)...);
	}

	/// Call the destructor of the TClass.
	void destroy()
	{
		reinterpret_cast<TClass*>(&m_data[0])->~TClass();
	}

	/// Access the instance.
	TClass* operator->()
	{
		return reinterpret_cast<TClass*>(&m_data[0]);
	}

	/// Access the instance.
	const TClass* operator->() const
	{
		return reinterpret_cast<const TClass*>(&m_data[0]);
	}

	/// Access the instance.
	TClass& operator*()
	{
		return *reinterpret_cast<TClass*>(&m_data[0]);
	}

	/// Access the instance.
	const TClass& operator*() const
	{
		return *reinterpret_cast<const TClass*>(&m_data[0]);
	}

	/// Access the instance.
	TClass* get()
	{
		return reinterpret_cast<TClass*>(&m_data[0]);
	}

	/// Access the instance.
	const TClass* get() const
	{
		return reinterpret_cast<const TClass*>(&m_data[0]);
	}

private:
	/// The data as a POD with correct size and alignment.
	alignas(alignof(TClass)) Array<U8, sizeof(TClass)> m_data;
};
/// @}

} // end namespace anki