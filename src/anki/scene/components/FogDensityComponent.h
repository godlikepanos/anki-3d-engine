// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/collision/Aabb.h>
#include <anki/collision/Sphere.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Fog density component. Controls the fog density.
class FogDensityComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::FOG_DENSITY;

	FogDensityComponent()
		: SceneComponent(CLASS_TYPE)
	{
	}

	void setAabb(const Vec4& aabbMin, const Vec4& aabbMax)
	{
		m_aabbMin = aabbMin;
		m_aabbMax = aabbMax;
		m_box = true;
	}

	void setSphere(F32 radius)
	{
		m_sphereRadius = radius;
		m_box = false;
	}

	Bool isAabb() const
	{
		return m_box == true;
	}

	void getAabb(Vec4& aabbMin, Vec4& aabbMax) const
	{
		ANKI_ASSERT(isAabb());
		aabbMin = m_aabbMin;
		aabbMax = m_aabbMax;
	}

	void getSphere(F32& radius) const
	{
		ANKI_ASSERT(!isAabb());
		radius = m_sphereRadius;
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

	void updatePosition(const Vec4& pos)
	{
		m_worldPos = pos;
	}

	void setupFogDensityQueueElement(FogDensityQueueElement& el) const
	{
		el.m_density = m_density;
		el.m_isBox = m_box;
		if(m_box)
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

private:
	Vec4 m_aabbMin{0.0f};

	union
	{
		Vec4 m_aabbMax{1.0f};
		F32 m_sphereRadius;
	};

	Vec4 m_worldPos{0.0f};

	F32 m_density = 1.0f;
	Bool m_box = false;
};

} // end namespace anki