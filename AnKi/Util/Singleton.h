// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <utility>

namespace anki {

/// @addtogroup util_patterns
/// @{

/// If class inherits that it will become a singleton.
template<typename T>
class MakeSingleton
{
public:
	static T& getSingleton()
	{
		return *m_global;
	}

	template<typename... TArgs>
	static void allocateSingleton(TArgs&&... args)
	{
		if(m_global == nullptr)
		{
			m_global = new T(std::forward<TArgs>(args)...);
		}
	}

	static void freeSingleton()
	{
		delete m_global;
		m_global = nullptr;
	}

private:
	static T* m_global;
};

template<typename T>
T* MakeSingleton<T>::m_global = nullptr;
/// @}

} // end namespace anki
