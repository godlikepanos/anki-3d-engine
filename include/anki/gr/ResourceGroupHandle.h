// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_RESOURCE_GROUP_HANDLE_H
#define ANKI_GR_RESOURCE_GROUP_HANDLE_H

#include "anki/gr/GrHandle.h"
#include "anki/gr/ResourceGroupCommon.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// A collection of resource bindings.
class ResourceGroupHandle: public GrHandle<ResourceGroupImpl>
{
public:
	using Base = GrHandle<ResourceGroupImpl>;
	using Initializer = ResourceGroupInitializer;

	ResourceGroupHandle();

	~ResourceGroupHandle();

	/// Create resource group.
	ANKI_USE_RESULT Error create(GrManager* manager, const Initializer& init);
};
/// @}

} // end namespace anki

#endif

