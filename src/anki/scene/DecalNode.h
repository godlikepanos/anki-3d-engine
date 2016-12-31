// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/collision/Obb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Node that has a decal component.
class DecalNode : public SceneNode
{
	friend class DecalMoveFeedbackComponent;
	friend class DecalShapeFeedbackComponent;

public:
	DecalNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~DecalNode();

	ANKI_USE_RESULT Error init();

private:
	Obb m_obbW; ///< OBB in world space.

	void onMove(MoveComponent& movec);
	void onDecalUpdated(DecalComponent& decalc);
	void updateObb(const Transform& trf, const DecalComponent& decalc);
};
/// @}

} // end namespace anki
