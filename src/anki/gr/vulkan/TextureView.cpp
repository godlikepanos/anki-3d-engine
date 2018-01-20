// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/TextureView.h>
#include <anki/gr/vulkan/TextureViewImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

TextureView* TextureView::newInstance(GrManager* manager, const TextureViewInitInfo& init)
{
	return TextureViewImpl::newInstanceHelper(manager, init);
}

} // end namespace anki
