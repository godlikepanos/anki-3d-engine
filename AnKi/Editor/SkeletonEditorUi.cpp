// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/SkeletonEditorUi.h>

namespace anki {

void SkeletonEditorUi::open(const SkeletonResource& resource)
{
	m_open = true;
	m_yaw = 0.0f;
	m_zoom = 1.0f;
	m_pan = Vec2(0.0f);
	initCache(resource);
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
			drawSkeleton();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Right: The bone hierarchy
		if(ImGui::BeginChild("Hierarchy", Vec2(0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
			{
				for(const CachedBone& bone : m_bones)
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

void SkeletonEditorUi::drawSkeleton()
{
	// Controls
	if(ImGui::Button(ICON_MDI_ROTATE_LEFT))
	{
		m_yaw -= toRad(15.0f);
	}
	ImGui::SetItemTooltip("Rotate left");
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_ROTATE_RIGHT))
	{
		m_yaw += toRad(15.0f);
	}
	ImGui::SetItemTooltip("Rotate right");
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_MAGNIFY_MINUS))
	{
		m_zoom /= 1.25f;
	}
	ImGui::SetItemTooltip("Zoom out");
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_MAGNIFY_PLUS))
	{
		m_zoom *= 1.25f;
	}
	ImGui::SetItemTooltip("Zoom in");
	ImGui::SameLine();
	constexpr F32 kPanStep = 20.0f;
	if(ImGui::Button(ICON_MDI_ARROW_LEFT))
	{
		m_pan.x -= kPanStep;
	}
	ImGui::SetItemTooltip("Pan left");
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_ARROW_RIGHT))
	{
		m_pan.x += kPanStep;
	}
	ImGui::SetItemTooltip("Pan right");
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_ARROW_UP))
	{
		m_pan.y += kPanStep;
	}
	ImGui::SetItemTooltip("Pan up");
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_ARROW_DOWN))
	{
		m_pan.y -= kPanStep;
	}
	ImGui::SetItemTooltip("Pan down");
	ImGui::SameLine();
	if(ImGui::Button(ICON_MDI_RESTORE))
	{
		m_yaw = 0.0f;
		m_zoom = 1.0f;
		m_pan = Vec2(0.0f);
	}
	ImGui::SetItemTooltip("Reset view");

	if(m_bones.getSize() == 0)
	{
		return;
	}

	auto computeBonePosition = [](const CachedBone& bone) -> Vec3 {
		Transform trf = bone.m_transform;
		for(const CachedBone* parent = bone.m_parent; parent; parent = parent->m_parent)
		{
			trf = parent->m_transform.combineTransformations(trf);
		}
		return trf.getOrigin().xyz;
	};

	// Orthographic view that rotates around the Y axis. Fit the bounds' diagonal into the region so the skeleton fits at any rotation
	Vec3 boundsMin(kMaxF32);
	Vec3 boundsMax(kMinF32);
	for(const CachedBone& bone : m_bones)
	{
		const Vec3 pos = computeBonePosition(bone);
		boundsMin = boundsMin.min(pos);
		boundsMax = boundsMax.max(pos);
	}

	const Vec2 regionMin = ImGui::GetCursorScreenPos();
	const Vec2 regionSize = ImGui::GetContentRegionAvail();
	const Vec2 regionCenter = regionMin + regionSize / 2.0f;
	const Vec3 boundsCenter = (boundsMin + boundsMax) / 2.0f;
	const F32 fitScale = min(regionSize.x, regionSize.y) * 0.9f / max((boundsMax - boundsMin).length(), kEpsilonf);
	const F32 sinYaw = sin(m_yaw);
	const F32 cosYaw = cos(m_yaw);

	auto toScreen = [&](Vec3 pos) -> Vec2 {
		const Vec3 p = pos - boundsCenter;
		const Vec2 view =
			(Vec2(p.x * cosYaw + p.z * sinYaw, p.y) * fitScale - m_pan) * m_zoom; // Pan moves the camera, so the skeleton moves the opposite way
		return regionCenter + Vec2(view.x, -view.y); // Screen Y points down
	};

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	for(const CachedBone& bone : m_bones)
	{
		const Vec2 pos = toScreen(computeBonePosition(bone));

		if(bone.m_parent)
		{
			drawList->AddLine(toScreen(computeBonePosition(*bone.m_parent)), pos, IM_COL32(200, 200, 200, 255), 2.0f);
		}

		drawList->AddCircleFilled(pos, 3.0f, IM_COL32(255, 160, 0, 255));
	}

	// Draw the selected bone last so it's on top
	if(m_selectedBone)
	{
		const Vec2 pos = toScreen(computeBonePosition(*m_selectedBone));

		if(m_selectedBone->m_parent)
		{
			drawList->AddLine(toScreen(computeBonePosition(*m_selectedBone->m_parent)), pos, IM_COL32(0, 200, 255, 255), 3.0f);
		}

		drawList->AddCircleFilled(pos, 6.0f, IM_COL32(0, 200, 255, 255));
	}
}

void SkeletonEditorUi::drawBoneTree(const CachedBone& bone)
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
		outBone.m_transform = Transform(Mat4(inBone.getTransform(), Vec4(0.0f, 0.0f, 0.0f, 1.0f)));
		outBone.m_vertTrf = Transform(Mat4(inBone.getVertexTransform(), Vec4(0.0f, 0.0f, 0.0f, 1.0f)));

		if(inBone.getParent())
		{
			outBone.m_parent = &m_bones[inBone.getParent()->getIndex()];
		}

		for(const Bone* inChild : inBone.getChildren())
		{
			outBone.m_children[outBone.m_childrenCount++] = &m_bones[inChild->getIndex()];
		}
	}
}

} // end namespace anki
