// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Editor/EditorCommon.h>
#include <AnKi/Resource/SkeletonResource.h>

namespace anki {

// Editor window that draws a skeleton and its bone hierarchy
class SkeletonEditorUi : public EditorUiBase
{
public:
	void open(const SkeletonResource& resource);

	void drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags);

private:
	Bool m_open = false;

	class CachedBone
	{
	public:
		String m_name;

		Transform m_transform;
		Transform m_vertTrf;

		CachedBone* m_parent = nullptr;
		Array<CachedBone*, kMaxChildrenPerBone> m_children = {};
		U8 m_childrenCount = 0;
	};

	DynamicArray<CachedBone> m_bones;
	const CachedBone* m_selectedBone = nullptr;

	F32 m_yaw = 0.0f; // In radians
	F32 m_zoom = 1.0f;
	Vec2 m_pan = Vec2(0.0f); // In pixels at zoom 1

	void initCache(const SkeletonResource& resource);

	void drawSkeleton();

	void drawBoneTree(const CachedBone& bone);
};

} // end namespace anki
