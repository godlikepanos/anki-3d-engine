// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/SkeletonEditorUi.h>
#include <AnKi/Util/Xml.h>

namespace anki {

// Colors of the capsule and constraint drawing. The UI uses the same so it's clear what each input changes
constexpr ImU32 kCapsuleColor = IM_COL32(80, 255, 80, 255);
constexpr ImU32 kTwistAxisColor = IM_COL32(255, 220, 0, 255);
constexpr ImU32 kPlaneAxisColor = IM_COL32(255, 80, 80, 255);
constexpr ImU32 kNormalAxisColor = IM_COL32(80, 140, 255, 255);
constexpr ImU32 kSwingConeColor = IM_COL32(170, 130, 255, 255);
constexpr ImU32 kTwistRangeColor = IM_COL32(255, 0, 255, 255);

void SkeletonEditorUi::open(const SkeletonResource& resource)
{
	m_open = true;
	m_filepath = resource.getFilename();
	resetView();
	initCache(resource);
}

void SkeletonEditorUi::resetView()
{
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_zoom = 1.0f;
	m_pan = Vec2(0.0f);
}

void SkeletonEditorUi::drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags)
{
	if(!m_open)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Skeleton Editor", &m_open, windowFlags))
	{
		// Left: The skeleton view
		if(ImGui::BeginChild("Skeleton", Vec2(ImGui::GetContentRegionAvail().x * 0.7f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
		{
			drawSkeletonControls();

			if(m_selectedBone)
			{
				drawBoneProperties(*m_selectedBone);
			}

			// The canvas has its own child so the drawing is clipped to it and doesn't go over the controls
			if(ImGui::BeginChild("Canvas", Vec2(0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar))
			{
				drawSkeleton();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Right: The bone hierarchy
		if(ImGui::BeginChild("Hierarchy", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
			{
				for(CachedBone& bone : m_bones)
				{
					if(bone.m_parent == nullptr)
					{
						drawBoneTree(bone);
					}
				}

				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void SkeletonEditorUi::drawSkeletonControls()
{
	if(ImGui::Button(ICON_MDI_CONTENT_SAVE " Save"))
	{
		if(saveCache())
		{
			ANKI_LOGE("Unable to save the skeleton file. Ignoring save");
		}
		ResourceManager::getSingleton().refreshFileUpdateTimes();
	}

	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_RESTORE))
	{
		resetView();
	}
	ImGui::SetItemTooltip("Reset view. Left drag rotates, middle drag pans and the wheel zooms");

	ImGui::SameLine();
	const Bool wasShowingOnlySelectedBone = m_showOnlySelectedBone;
	if(wasShowingOnlySelectedBone)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
	}
	if(ImGui::Button(ICON_MDI_EYE_OFF))
	{
		m_showOnlySelectedBone = !m_showOnlySelectedBone;
	}
	if(wasShowingOnlySelectedBone)
	{
		ImGui::PopStyleColor();
	}
	ImGui::SetItemTooltip("Hide all bones except the selected");

	// Copy paste of the selected bone's physics properties. Mainly used to copy the left side of the skeleton to the right
	ImGui::BeginDisabled(m_selectedBone == nullptr);
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_CONTENT_COPY))
	{
		m_copiedPhysics = m_selectedBone->m_physics;
		m_hasCopiedPhysics = true;
	}
	ImGui::SetItemTooltip("Copy the capsule and the constraint of the selected bone");

	ImGui::BeginDisabled(!m_hasCopiedPhysics);
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_CONTENT_PASTE))
	{
		m_selectedBone->m_physics = m_copiedPhysics;
	}
	ImGui::SetItemTooltip("Paste to the selected bone");

	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_FLIP_HORIZONTAL))
	{
		// Mixamo's right bones have the same axes as the left ones but with X flipped. Flipping X negates positions on X and rotations
		// around Y and Z
		auto mirror = [](const Euler& e) {
			return Euler(e.x, -e.y, -e.z);
		};

		BonePhysics& physics = m_selectedBone->m_physics;
		physics = m_copiedPhysics;
		physics.m_capsulePosition.x = -physics.m_capsulePosition.x;
		physics.m_capsuleRotation = mirror(physics.m_capsuleRotation);
		physics.m_constraintRotation = mirror(physics.m_constraintRotation);
		physics.m_constraintParentRotation = mirror(physics.m_constraintParentRotation);
		physics.m_twistMinAngle = -m_copiedPhysics.m_twistMaxAngle;
		physics.m_twistMaxAngle = -m_copiedPhysics.m_twistMinAngle;
	}
	ImGui::SetItemTooltip("Paste mirrored to the selected bone. For copying to the bone of the other side. Assumes the 2 sides differ only in "
						  "the direction of the X axis");
	ImGui::EndDisabled();
	ImGui::EndDisabled();
}

void SkeletonEditorUi::drawBoneProperties(CachedBone& bone)
{
	// Angles are stored in radians but edited in degrees. Each input has the color of what it changes in the drawing
	auto dragAngles = [](const Char* label, Euler& angles, ImU32 color, const Char* tooltip) {
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		Vec3 degrees(toDegrees(angles.x), toDegrees(angles.y), toDegrees(angles.z));
		if(ImGui::DragFloat3(label, &degrees.x, 0.25f, -360.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			angles = Euler(toRad(degrees.x), toRad(degrees.y), toRad(degrees.z));
		}
		ImGui::PopStyleColor();
		ImGui::SetItemTooltip("%s", tooltip);
	};

	auto dragAngle = [](const Char* label, F32& angle, F32 minDegrees, F32 maxDegrees, ImU32 color, const Char* tooltip) {
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		F32 degrees = toDegrees(angle);
		if(ImGui::DragFloat(label, &degrees, 0.25f, minDegrees, maxDegrees, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			angle = toRad(degrees);
		}
		ImGui::PopStyleColor();
		ImGui::SetItemTooltip("%s", tooltip);
	};

	ImGui::TextUnformatted(bone.m_name.cstr());

	// Capsule
	BonePhysics& physics = bone.m_physics;
	ImGui::PushStyleColor(ImGuiCol_Text, kCapsuleColor);
	ImGui::DragFloat("Capsule Radius", &physics.m_capsuleRadius, 0.01f, 0.0f, kMaxF32, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SetItemTooltip("0 means no capsule");
	ImGui::DragFloat("Capsule Half Height", &physics.m_capsuleHalfHeight, 0.01f, 0.0f, kMaxF32, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SetItemTooltip("Half the length of the cylinder part without the caps");
	ImGui::DragFloat3("Capsule Position", &physics.m_capsulePosition.x, 0.025f);
	ImGui::SetItemTooltip("The center of the capsule in bone space");
	ImGui::PopStyleColor();
	dragAngles("Capsule Rotation", physics.m_capsuleRotation, kCapsuleColor, "In bone space. With zero rotation the capsule lies along +Y");

	// Constraint to the parent. The root doesn't have one
	if(bone.m_parent == nullptr)
	{
		return;
	}

	dragAngles("Constraint Rotation", physics.m_constraintRotation, ImGui::GetColorU32(ImGuiCol_Text),
			   "In bone space. With zero rotation the twist axis (yellow) is +Y and the plane axis (red) is +X. The normal axis is blue");
	dragAngles("Constraint Parent Rotation", physics.m_constraintParentRotation, kSwingConeColor,
			   "Extra rotation of the parent's side. Moves the cone away from the bone so a joint can bend one way. X: around the plane axis (knees, "
			   "elbows). Z: sideways. Y: rotates the twist range and the cone. To move the twist range prefer changing Twist Min/Max");

	ImGui::PushStyleColor(ImGuiCol_Text, kTwistRangeColor);
	F32 twistMin = toDegrees(physics.m_twistMinAngle);
	F32 twistMax = toDegrees(physics.m_twistMaxAngle);
	if(ImGui::DragFloatRange2("Twist Min/Max", &twistMin, &twistMax, 0.25f, -180.0f, 180.0f, "%.3f", nullptr, ImGuiSliderFlags_AlwaysClamp))
	{
		physics.m_twistMinAngle = toRad(twistMin);
		physics.m_twistMaxAngle = toRad(twistMax);
	}
	ImGui::PopStyleColor();
	ImGui::SetItemTooltip("Rotation around the twist axis in degrees");

	dragAngle("Plane Angle", physics.m_planeAngle, 0.0f, 180.0f, kSwingConeColor, "Half cone angle in degrees. Swing around the plane axis");
	dragAngle("Normal Angle", physics.m_normalAngle, 0.0f, 180.0f, kSwingConeColor, "Half cone angle in degrees. Swing around the normal axis");
}

void SkeletonEditorUi::drawSkeleton()
{
	if(m_bones.getSize() == 0)
	{
		return;
	}

	const Vec2 regionMin = ImGui::GetCursorScreenPos();
	const Vec2 regionSize = ImGui::GetContentRegionAvail();
	const Vec2 regionCenter = regionMin + regionSize / 2.0f;

	// Mouse controls. The invisible button makes the canvas an item so it gets the clicks and drags instead of the window
	ImGui::InvisibleButton("##Canvas", regionSize.max(1.0f), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
	ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY); // Don't let the wheel scroll the parent windows
	const ImGuiIO& io = ImGui::GetIO();
	if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		constexpr F32 kRadiansPerPixel = 0.01f;
		m_yaw += io.MouseDelta.x * kRadiansPerPixel;
		m_pitch = clamp(m_pitch + io.MouseDelta.y * kRadiansPerPixel, -kPi / 2.0f, kPi / 2.0f);
	}

	if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
	{
		// The pan is in pixels at zoom 1 and screen Y points down
		m_pan.x -= io.MouseDelta.x / m_zoom;
		m_pan.y += io.MouseDelta.y / m_zoom;
	}

	if(ImGui::IsItemHovered() && io.MouseWheel != 0.0f)
	{
		// Zoom towards the cursor by moving the pan so the point under it stays in place
		const Vec2 cursor = Vec2(io.MousePos) - regionCenter;
		const Vec2 cursorView(cursor.x, -cursor.y);
		const F32 newZoom = m_zoom * pow(1.1f, io.MouseWheel);
		m_pan += cursorView * (1.0f / m_zoom - 1.0f / newZoom);
		m_zoom = newZoom;
	}

	// Orthographic view that rotates around the Y and then the X axis. Fit the bounds' diagonal into the region so the skeleton fits at any rotation
	Vec3 boundsMin(kMaxF32);
	Vec3 boundsMax(kMinF32);
	for(const CachedBone& bone : m_bones)
	{
		boundsMin = boundsMin.min(bone.m_worldTransform.getOrigin().xyz);
		boundsMax = boundsMax.max(bone.m_worldTransform.getOrigin().xyz);
	}

	const Vec3 boundsCenter = (boundsMin + boundsMax) / 2.0f;
	const F32 boundsDiagonal = (boundsMax - boundsMin).length();
	const F32 fitScale = min(regionSize.x, regionSize.y) * 0.9f / max(boundsDiagonal, kEpsilonf);
	const F32 sinYaw = sin(m_yaw);
	const F32 cosYaw = cos(m_yaw);
	const F32 sinPitch = sin(m_pitch);
	const F32 cosPitch = cos(m_pitch);

	auto toScreen = [&](Vec3 pos) -> Vec2 {
		const Vec3 p = pos - boundsCenter;

		// Yaw around Y then pitch around X. Positive pitch looks from above
		const F32 x = p.x * cosYaw + p.z * sinYaw;
		const F32 z = -p.x * sinYaw + p.z * cosYaw;
		const F32 y = p.y * cosPitch - z * sinPitch;

		const Vec2 view = (Vec2(x, y) * fitScale - m_pan) * m_zoom; // Pan moves the camera, so the skeleton moves the opposite way
		return regionCenter + Vec2(view.x, -view.y); // Screen Y points down
	};

	auto isHidden = [&](const CachedBone& bone) {
		return m_showOnlySelectedBone && &bone != m_selectedBone;
	};

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// Capsules. Orthographic projection keeps sizes so the outline is always a 2D stadium
	for(const CachedBone& bone : m_bones)
	{
		const BonePhysics& physics = bone.m_physics;
		if(physics.m_capsuleRadius <= 0.0f || isHidden(bone))
		{
			continue;
		}

		// The capsule's segment in bone space
		const Vec3 halfSegment = Mat3(physics.m_capsuleRotation) * Vec3(0.0f, physics.m_capsuleHalfHeight, 0.0f);

		const Transform& boneTrf = bone.m_worldTransform;
		const Vec2 a = toScreen(boneTrf.transform(physics.m_capsulePosition - halfSegment));
		const Vec2 b = toScreen(boneTrf.transform(physics.m_capsulePosition + halfSegment));
		const F32 r = physics.m_capsuleRadius * boneTrf.getScale().x * fitScale * m_zoom;
		const Vec2 ab = b - a;
		if(ab.length() > kEpsilonf)
		{
			const F32 angle = atan2(ab.y, ab.x);
			drawList->PathArcTo(a, r, angle + kPi / 2.0f, angle + kPi * 1.5f); // Half circle around a
			drawList->PathArcTo(b, r, angle - kPi / 2.0f, angle + kPi / 2.0f); // Half circle around b
			drawList->PathStroke(kCapsuleColor, ImDrawFlags_Closed, 1.5f); // Closing the path draws the 2 straight lines
		}
		else
		{
			drawList->AddCircle(a, r, kCapsuleColor, 0, 1.5f);
		}
	}

	// Bones
	for(const CachedBone& bone : m_bones)
	{
		if(isHidden(bone))
		{
			continue;
		}

		const Vec2 pos = toScreen(bone.m_worldTransform.getOrigin().xyz);

		if(bone.m_parent)
		{
			drawList->AddLine(toScreen(bone.m_parent->m_worldTransform.getOrigin().xyz), pos, IM_COL32(200, 200, 200, 255), 2.0f);
		}

		drawList->AddCircleFilled(pos, 3.0f, IM_COL32(255, 160, 0, 255));
	}

	// Draw the selected bone last so it's on top
	if(m_selectedBone == nullptr)
	{
		return;
	}

	const Transform& boneTrf = m_selectedBone->m_worldTransform;
	const Vec3 head = boneTrf.getOrigin().xyz;
	const Vec2 pos = toScreen(head);

	// The bone's segments go from its head to the heads of its children
	for(U32 i = 0; i < m_selectedBone->m_childrenCount; ++i)
	{
		drawList->AddLine(pos, toScreen(m_selectedBone->m_children[i]->m_worldTransform.getOrigin().xyz), IM_COL32(0, 200, 255, 255), 3.0f);
	}

	drawList->AddCircleFilled(pos, 6.0f, IM_COL32(0, 200, 255, 255));

	// The constraint to the parent. It pivots at the bone's head
	if(m_selectedBone->m_parent == nullptr)
	{
		return;
	}

	const BonePhysics& physics = m_selectedBone->m_physics;
	const Mat3 boneRot = boneTrf.getRotation().getRotationPart();

	// The bone's side of the constraint (Jolt's body 2)
	const Mat3 frame = boneRot * Mat3(physics.m_constraintRotation);
	const Vec3 twistAxis = frame * Vec3(0.0f, 1.0f, 0.0f);
	const Vec3 planeAxis = frame * Vec3(1.0f, 0.0f, 0.0f);
	const Vec3 normalAxis = twistAxis.cross(planeAxis); // Same as Jolt

	// The parent's side of the constraint (Jolt's body 1). The limits are measured in this frame
	const Mat3 parentFrame = frame * Mat3(physics.m_constraintParentRotation);
	const Vec3 parentTwistAxis = parentFrame * Vec3(0.0f, 1.0f, 0.0f);
	const Vec3 parentPlaneAxis = parentFrame * Vec3(1.0f, 0.0f, 0.0f);
	const Vec3 parentNormalAxis = parentTwistAxis.cross(parentPlaneAxis);

	const F32 size = boundsDiagonal * 0.1f;

	// Axes
	drawList->AddLine(pos, toScreen(head + twistAxis * size), kTwistAxisColor, 2.0f);
	drawList->AddLine(pos, toScreen(head + planeAxis * size), kPlaneAxisColor, 2.0f);
	drawList->AddLine(pos, toScreen(head + normalAxis * size), kNormalAxisColor, 2.0f);

	// Swing cone. The rim is the edge of Jolt's limit: (q.y / sin(plane / 2))^2 + (q.z / sin(normal / 2))^2 = 1 where q is the swing quaternion
	// in the parent's constraint space (x is twist, y is plane, z is normal). Rotate the bone's direction and not the twist axis so it shows where
	// the bone can actually point. The cone's center is the bone's direction moved by the parent rotation. A bone rotation R is allowed when
	// parentFrame^-1 * R * frame is, so R = (swing around the parent's axes) * (parentFrame * frame^-1)
	const Vec3 boneDir = boneRot * Vec3(0.0f, 1.0f, 0.0f);
	const Vec3 coneCenterDir = parentFrame * frame.transpose() * boneDir;
	const F32 sinHalfPlane = sin(physics.m_planeAngle / 2.0f);
	const F32 sinHalfNormal = sin(physics.m_normalAngle / 2.0f);
	constexpr U32 kRimPointCount = 32;
	Array<ImVec2, kRimPointCount> rimPoints;
	for(U32 i = 0; i < kRimPointCount; ++i)
	{
		const F32 theta = 2.0f * kPi * F32(i) / F32(kRimPointCount);
		const F32 qy = sinHalfPlane * cos(theta);
		const F32 qz = sinHalfNormal * sin(theta);
		const F32 qw = sqrt(max(1.0f - qy * qy - qz * qz, 0.0f));

		// Rotate the cone's center by the swing quaternion: v' = v + 2w(u x v) + 2u x (u x v)
		const Vec3 u = parentPlaneAxis * qy + parentNormalAxis * qz;
		const Vec3 uCrossV = u.cross(coneCenterDir);
		const Vec3 dir = coneCenterDir + uCrossV * (2.0f * qw) + u.cross(uCrossV) * 2.0f;
		rimPoints[i] = toScreen(head + dir * size);
	}

	// Don't mix AddLine with PathLineTo because AddLine uses and clears the same path
	drawList->AddPolyline(&rimPoints[0], kRimPointCount, kSwingConeColor, ImDrawFlags_Closed, 1.5f);
	for(U32 i = 0; i < kRimPointCount; i += kRimPointCount / 4)
	{
		drawList->AddLine(pos, rimPoints[i], kSwingConeColor, 1.0f);
	}

	// Twist range. It's measured from the parent's plane axis so the red plane axis shows where the bone's twist is now
	constexpr U32 kArcPointCount = 32;
	drawList->PathLineTo(pos);
	for(U32 i = 0; i <= kArcPointCount; ++i)
	{
		const F32 phi = linearInterpolate(physics.m_twistMinAngle, physics.m_twistMaxAngle, F32(i) / F32(kArcPointCount));
		const Vec3 dir = parentPlaneAxis * cos(phi) + parentNormalAxis * sin(phi);
		drawList->PathLineTo(toScreen(head + dir * size * 0.5f));
	}
	drawList->PathStroke(kTwistRangeColor, ImDrawFlags_Closed, 1.5f);
}

void SkeletonEditorUi::drawBoneTree(CachedBone& bone)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::PushID(&bone);

	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NavLeftJumpsToParent
								   | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DrawLinesToNodes | ImGuiTreeNodeFlags_DefaultOpen;
	if(bone.m_childrenCount == 0)
	{
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
	}

	if(&bone == m_selectedBone)
	{
		treeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	const Bool nodeOpen = ImGui::TreeNodeEx("", treeFlags, "%s", bone.m_name.cstr());

	if(ImGui::IsItemClicked())
	{
		m_selectedBone = &bone;
	}

	if(nodeOpen)
	{
		for(U32 i = 0; i < bone.m_childrenCount; ++i)
		{
			drawBoneTree(*bone.m_children[i]);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void SkeletonEditorUi::initCache(const SkeletonResource& resource)
{
	m_bones.destroy();
	m_selectedBone = nullptr;

	ConstWeakArray<Bone> inBones = resource.getBones();
	m_bones.resize(inBones.getSize());

	for(const Bone& inBone : inBones)
	{
		CachedBone& outBone = m_bones[inBone.getIndex()];

		outBone.m_name = inBone.getName();
		outBone.m_transform = inBone.getTransform();
		outBone.m_vertTrf = inBone.getVertexTransform();
		BonePhysics& physics = outBone.m_physics;
		physics.m_capsuleRadius = inBone.getCapsuleRadius();
		physics.m_capsuleHalfHeight = inBone.getCapsuleHalfHeight();
		physics.m_capsulePosition = inBone.getCapsulePosition();
		physics.m_capsuleRotation = inBone.getCapsuleRotation();
		physics.m_constraintRotation = inBone.getConstraintRotation();
		physics.m_constraintParentRotation = inBone.getConstraintParentRotation();
		physics.m_twistMinAngle = inBone.getTwistMinAngle();
		physics.m_twistMaxAngle = inBone.getTwistMaxAngle();
		physics.m_planeAngle = inBone.getPlaneAngle();
		physics.m_normalAngle = inBone.getNormalAngle();

		if(inBone.getParent())
		{
			outBone.m_parent = &m_bones[inBone.getParent()->getIndex()];
		}

		for(const Bone* inChild : inBone.getChildren())
		{
			ANKI_ASSERT(outBone.m_childrenCount < kMaxChildrenPerBone);
			outBone.m_children[outBone.m_childrenCount++] = &m_bones[inChild->getIndex()];
		}
	}

	// The parents are set now so the world transforms can be computed
	auto toTransform = [](const Mat3x4& m) {
		return Transform(Mat4(m, Vec4(0.0f, 0.0f, 0.0f, 1.0f)));
	};

	for(CachedBone& bone : m_bones)
	{
		bone.m_worldTransform = toTransform(bone.m_transform);
		for(const CachedBone* parent = bone.m_parent; parent; parent = parent->m_parent)
		{
			bone.m_worldTransform = toTransform(parent->m_transform).combineTransformations(bone.m_worldTransform);
		}
	}
}

Error SkeletonEditorUi::saveCache()
{
	const ResourceString diskFilepath = ResourceFilesystem::getSingleton().getDiskFilepath(m_filepath);

	File file;
	ANKI_CHECK(file.open(diskFilepath, FileOpenFlag::kWrite));

	auto writeMatrix = [&](const Char* attribName, const Mat3x4& m) -> Error {
		ANKI_CHECK(file.writeTextf("\t\t\t%s=\"", attribName));
		for(U32 i = 0; i < 12; ++i)
		{
			ANKI_CHECK(file.writeTextf((i < 11) ? "%f " : "%f", m[i]));
		}
		return file.writeText("\"\n");
	};

	auto writeAngles = [&](const Char* attribName, const Euler& angles) -> Error {
		return file.writeTextf("\t\t\t%s=\"%f %f %f\"\n", attribName, toDegrees(angles.x), toDegrees(angles.y), toDegrees(angles.z));
	};

	auto writeAngle = [&](const Char* attribName, F32 angle) -> Error {
		return file.writeTextf("\t\t\t%s=\"%f\"\n", attribName, toDegrees(angle));
	};

	ANKI_CHECK(file.writeTextf("%s\n<skeleton>\n", ResourceXmlDocument::kXmlHeader.cstr()));
	ANKI_CHECK(file.writeText("\t<bones>\n"));

	// Keep the order of the bones because it's their index
	for(const CachedBone& bone : m_bones)
	{
		ANKI_CHECK(file.writeTextf("\t\t<bone name=\"%s\"\n", bone.m_name.cstr()));
		if(bone.m_parent)
		{
			ANKI_CHECK(file.writeTextf("\t\t\tparent=\"%s\"\n", bone.m_parent->m_name.cstr()));
		}

		ANKI_CHECK(writeMatrix("boneTransform", bone.m_vertTrf));
		ANKI_CHECK(writeMatrix("transform", bone.m_transform));

		const BonePhysics& physics = bone.m_physics;
		if(physics.m_capsuleRadius > 0.0f)
		{
			const Vec3& pos = physics.m_capsulePosition;
			ANKI_CHECK(file.writeTextf("\t\t\tcapsuleRadius=\"%f\"\n", physics.m_capsuleRadius));
			ANKI_CHECK(file.writeTextf("\t\t\tcapsuleHalfHeight=\"%f\"\n", physics.m_capsuleHalfHeight));
			ANKI_CHECK(file.writeTextf("\t\t\tcapsulePosition=\"%f %f %f\"\n", pos.x, pos.y, pos.z));
			ANKI_CHECK(writeAngles("capsuleRotation", physics.m_capsuleRotation));
		}

		// Skip the constraint when everything is zero to keep the file short
		const Euler& crot = physics.m_constraintRotation;
		const Euler& cprot = physics.m_constraintParentRotation;
		const Bool hasConstraint = crot.x != 0.0f || crot.y != 0.0f || crot.z != 0.0f || cprot.x != 0.0f || cprot.y != 0.0f || cprot.z != 0.0f
								   || physics.m_twistMinAngle != 0.0f || physics.m_twistMaxAngle != 0.0f || physics.m_planeAngle != 0.0f
								   || physics.m_normalAngle != 0.0f;
		if(bone.m_parent && hasConstraint)
		{
			ANKI_ASSERT(physics.m_twistMinAngle <= physics.m_twistMaxAngle && "The loader rejects it");
			ANKI_CHECK(writeAngles("constraintRotation", crot));
			ANKI_CHECK(writeAngles("constraintParentRotation", cprot));
			ANKI_CHECK(writeAngle("twistMinAngle", physics.m_twistMinAngle));
			ANKI_CHECK(writeAngle("twistMaxAngle", physics.m_twistMaxAngle));
			ANKI_CHECK(writeAngle("planeAngle", physics.m_planeAngle));
			ANKI_CHECK(writeAngle("normalAngle", physics.m_normalAngle));
		}

		ANKI_CHECK(file.writeText("\t\t/>\n"));
	}

	ANKI_CHECK(file.writeText("\t</bones>\n"));
	ANKI_CHECK(file.writeText("</skeleton>\n"));

	return Error::kNone;
}

} // end namespace anki
