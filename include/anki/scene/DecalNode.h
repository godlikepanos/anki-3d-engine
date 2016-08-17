// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Node that has a decal component.
class DecalNode : public SceneNode
{
public:
	DecalNode(SceneGraph* scene);

	~DecalNode();

	ANKI_USE_RESULT Error init(const CString& name);

private:
};
/// @}

} // end namespace anki
