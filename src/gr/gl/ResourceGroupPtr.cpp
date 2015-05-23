// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ResourceGroupPtr.h"
#include "anki/gr/gl/ResourceGroupImpl.h"

namespace anki {

//==============================================================================
ResourceGroupPtr::ResourceGroupPtr()
{}

//==============================================================================
ResourceGroupPtr::~ResourceGroupPtr()
{}

//==============================================================================
Error ResourceGroupPtr::create(GrManager* manager, const Initializer& init)
{
	Base::create(*manager);
	ANKI_CHECK(get().create(init));

	return ErrorCode::NONE;
}

} // end namespace anki
