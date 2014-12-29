// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Forward.h"
#include "anki/resource/Resource.h"
#include "anki/resource/TextureResource.h"
#include "anki/Collision.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Light component. It's a dummy component used to identify lights
class LightComponent: public SceneComponent
{
public:
	enum class LightType: U8
	{
		POINT,
		SPOT,
		COUNT
 	};

	LightComponent(SceneNode* node, LightType type);

	LightType getLightType() const
	{
		return m_type;
	}

	const Vec4& getDiffuseColor() const
	{
		return m_diffColor;
	}

	void setDiffuseColor(const Vec4& x)
	{
		m_diffColor = x;
	}

	const Vec4& getSpecularColor() const
	{
		return m_specColor;
	}

	void setSpecularColor(const Vec4& x)
	{
		m_specColor = x;
	}

	void setRadius(F32 x)
	{
		m_radius = x;
		m_dirty = true;
	}

	F32 getRadius() const
	{
		return m_radius;
	}

	void setDistance(F32 x)
	{
		m_distance = x;
		m_dirty = true;
	}

	F32 getDistance() const
	{
		return m_distance;
	}

	void setInnerAngle(F32 ang)
	{
		m_innerAngleCos = cos(ang / 2.0);
		m_innerAngle = ang;
		m_dirty = true;
	}

	F32 getInnerAngleCos() const
	{
		return m_innerAngleCos;
	}

	void setOuterAngle(F32 ang)
	{
		m_outerAngleCos = cos(ang / 2.0);
		m_outerAngle = ang;
		m_dirty = true;
	}

	F32 getOuterAngle() const
	{
		return m_outerAngle;
	}

	F32 getOuterAngleCos() const
	{
		return m_outerAngleCos;
	}

	Bool getShadowEnabled() const
	{
		return m_shadow;
	}

	void setShadowEnabled(const Bool x)
	{
		m_shadow = x;
	}

	U getShadowMapIndex() const
	{
		return static_cast<U>(m_shadowMapIndex);
	}

	void setShadowMapIndex(const U i)
	{
		ANKI_ASSERT(i < 0xFF);
		m_shadowMapIndex = static_cast<U8>(i);
	}

	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override;

	static constexpr Type getClassType()
	{
		return Type::LIGHT;
	}

private:
	LightType m_type;
	Vec4 m_diffColor = Vec4(0.5);
	Vec4 m_specColor = Vec4(0.5);
	union
	{
		F32 m_radius;
		F32 m_distance;
	};
	F32 m_innerAngleCos;
	F32 m_outerAngleCos;
	F32 m_outerAngle;
	F32 m_innerAngle;

	Bool8 m_shadow = false;
	U8 m_shadowMapIndex = 0xFF; ///< Used by the renderer

	Bool8 m_dirty = true;
};

/// Light scene node. It can be spot or point.
class Light: public SceneNode
{
	friend class LightFeedbackComponent;

public:
	Light(SceneGraph* scene);

	virtual ~Light();

	ANKI_USE_RESULT Error create(
		const CString& name, 
		LightComponent::LightType type,
		CollisionShape* shape);

	ANKI_USE_RESULT Error loadLensFlare(const CString& filename);

protected:
	/// Called when moved
	void onMoveUpdateCommon(MoveComponent& move);

	/// One of the frustums got updated
	void onShapeUpdateCommon(LightComponent& light);

	virtual void onMoveUpdate(MoveComponent& move) = 0;

	virtual void onShapeUpdate(LightComponent& light) = 0;
};

/// Point light
class PointLight: public Light
{
public:
	PointLight(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name);

	/// @name SceneNode virtuals
	/// @{
	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;
	/// @}

public:
	class ShadowData
	{
	public:
#if 0
		ShadowData(SceneNode* node)
		:	m_frustumComps{{
				{node, &m_frustums[0]}, {node, &m_frustums[1]},
				{node, &m_frustums[2]}, {node, &m_frustums[3]},
				{node, &m_frustums[4]}, {node, &m_frustums[5]}}}
		{}

		Array<PerspectiveFrustum, 6> m_frustums;
		Array<FrustumComponent, 6> m_frustumComps;
		Array<Transform, 6> m_localTrfs;
#endif
	};

	Sphere m_sphereW = Sphere(Vec4(0.0), 1.0);
	ShadowData* m_shadowData = nullptr;

	void onMoveUpdate(MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};

/// Spot light
class SpotLight: public Light
{
public:
	SpotLight(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name);

private:
	PerspectiveFrustum m_frustum;

	void onMoveUpdate(MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};
/// @}

} // end namespace anki

#endif
