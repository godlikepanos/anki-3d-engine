// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ResourceGroup.h"
#include "anki/gr/gl/ResourceGroupImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/Texture.h"
#include "anki/gr/Sampler.h"
#include "anki/gr/Buffer.h"

namespace anki {

//==============================================================================
ResourceGroup::ResourceGroup(GrManager* manager)
	: GrObject(manager)
{}

//==============================================================================
ResourceGroup::~ResourceGroup()
{}

//==============================================================================
void ResourceGroup::create(const ResourceGroupInitializer& init)
{
	m_impl.reset(getAllocator().newInstance<ResourceGroupImpl>(&getManager()));
	m_impl->create(init);
}

} // end namespace anki
