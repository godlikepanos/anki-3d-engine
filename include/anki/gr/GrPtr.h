// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GR_HANDLE_H
#define ANKI_GR_GR_HANDLE_H

#include "anki/gr/GrObject.h"
#include "anki/gr/GrManager.h"
#include "anki/util/Ptr.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Shared pointer for graphics objects.
///
/// @tparam T The type of the pointer
template<typename T>
class GrPtr: public IntrusivePtr<GrObject>
{
public:
	using Base = IntrusivePtr<GrObject>;

	/// Create an object and initialize the handle with that.
	/// @param manager The manager.
	void create(GrManager& manager)
	{
		// Create the object
		auto alloc = manager.getAllocator();
		T* ptr = alloc.template newInstance<T>(&manager);
		Base::reset(ptr);
	}

	const T& get() const
	{
		return static_cast<const T&>(Base::get());
	}

	T& get()
	{
		return static_cast<T&>(Base::get());
	}
};
/// @}

} // end namespace anki

#endif

