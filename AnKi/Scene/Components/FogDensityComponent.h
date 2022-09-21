// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Sphere.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Fog density component. Controls the fog density.
class FogDensityComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(FogDensityComponent)

public:
	static constexpr F32 kMinShapeSize = 1.0_cm;

	FogDensityComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId())
		, m_isBox(true)
		, m_markedForUpdate(true)
	{
	}

	void setBoxVolumeSize(Vec3 sizeXYZ)
	{
		sizeXYZ = sizeXYZ.max(Vec3(kMinShapeSize));
		m_aabbMin = -sizeXYZ / 2.0f;
		m_aabbMax = sizeXYZ / 2.0f;
		m_isBox = true;
		m_markedForUpdate = true;
	}

	Vec3 getBoxVolumeSize() const
	{
		ANKI_ASSERT(isAabb());
		return m_aabbMax.xyz() - m_aabbMin.xyz();
	}

	Aabb getAabbWorldSpace() const
	{
		ANKI_ASSERT(isAabb());
		return Aabb(m_aabbMin + m_worldPos, m_aabbMax + m_worldPos);
	}

	void setSphereVolumeRadius(F32 radius)
	{
		m_sphereRadius = max(kMinShapeSize, radius);
		m_isBox = false;
		m_markedForUpdate = true;
	}

	F32 getSphereVolumeRadius() const
	{
		ANKI_ASSERT(isSphere());
		return m_sphereRadius;
	}

	Sphere getSphereWorldSpace() const
	{
		ANKI_ASSERT(isSphere());
		return Sphere(m_worldPos, m_sphereRadius);
	}

	Bool isAabb() const
	{
		return m_isBox == true;
	}

	Bool isSphere() const
	{
		return !m_isBox;
	}

	void setDensity(F32 d)
	{
		ANKI_ASSERT(d >= 0.0f);
		m_density = d;
	}

	F32 getDensity() const
	{
		return m_density;
	}

	void setWorldPosition(const Vec3& pos)
	{
		m_worldPos = pos;
		m_markedForUpdate = true;
	}

	void setupFogDensityQueueElement(FogDensityQueueElement& el) const
	{
		el.m_density = m_density;
		el.m_isBox = m_isBox;
		if(m_isBox)
		{
			el.m_aabbMin = (m_aabbMin + m_worldPos).xyz();
			el.m_aabbMax = (m_aabbMax + m_worldPos).xyz();
		}
		else
		{
			el.m_sphereCenter = m_worldPos.xyz();
			el.m_sphereRadius = m_sphereRadius;
		}
	}

	Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = m_markedForUpdate;
		m_markedForUpdate = false;
		return Error::kNone;
	}

private:
	Vec3 m_aabbMin = Vec3(0.0f);

	union
	{
		Vec3 m_aabbMax = Vec3(1.0f);
		F32 m_sphereRadius;
	};

	Vec3 m_worldPos = Vec3(0.0f);
	F32 m_density = 1.0f;

	Bool m_isBox : 1;
	Bool m_markedForUpdate : 1;
};

} // end namespace anki
