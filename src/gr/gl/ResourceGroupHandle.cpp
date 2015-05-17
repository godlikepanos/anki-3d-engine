// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ResourceGroupHandle.h"
#include "anki/gr/gl/ResourceGroupImpl.h"

namespace anki {

//==============================================================================
ResourceGroupHandle::ResourceGroupHandle()
{}

//==============================================================================
ResourceGroupHandle::~ResourceGroupHandle()
{}

//==============================================================================
Error ResourceGroupHandle::create(GrManager* manager, const Initializer& init)
{
	using Deleter = GrHandleDefaultDeleter<ResourceGroupImpl>;

	Base::create(*manager, Deleter());
	ANKI_CHECK(get().create(init));

	return ErrorCode::NONE;
}

} // end namespace anki
