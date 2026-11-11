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

	// The physics properties of a bone. Same as in the Bone
	class BonePhysics
	{
	public:
		F32 m_capsuleRadius = 0.0f;
		F32 m_capsuleHalfHeight = 0.0f;
		Vec3 m_capsulePosition = Vec3(0.0f);
		Euler m_capsuleRotation;
		Euler m_constraintRotation;
		Euler m_constraintParentRotation;
		F32 m_twistMinAngle = 0.0f;
		F32 m_twistMaxAngle = 0.0f;
		F32 m_planeAngle = 0.0f;
		F32 m_normalAngle = 0.0f;
	};

	class CachedBone
	{
	public:
		String m_name;

		// Same as in the file
		Mat3x4 m_transform;
		Mat3x4 m_vertTrf;

		Transform m_worldTransform; // The editor can't move bones so it's computed once

		CachedBone* m_parent = nullptr;
		Array<CachedBone*, kMaxChildrenPerBone> m_children = {};
		U8 m_childrenCount = 0;

		BonePhysics m_physics;
	};

	DynamicArray<CachedBone> m_bones;
	CachedBone* m_selectedBone = nullptr;

	BonePhysics m_copiedPhysics;
	Bool m_hasCopiedPhysics = false;

	F32 m_yaw = 0.0f; // In radians
	F32 m_pitch = 0.0f; // In radians
	F32 m_zoom = 1.0f;
	Vec2 m_pan = Vec2(0.0f); // In pixels at zoom 1
	Bool m_showOnlySelectedBone = false;

	String m_filepath;

	void initCache(const SkeletonResource& resource);

	Error saveCache();

	void resetView();

	void drawSkeletonControls();

	void drawBoneProperties(CachedBone& bone);

	void drawSkeleton();

	void drawBoneTree(CachedBone& bone);
};

} // end namespace anki
