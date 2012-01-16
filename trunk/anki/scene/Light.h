#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Spatial.h"
#include "anki/gl/Vao.h"


namespace anki {


/// Light scene node. It can be spot or point
///
/// Explaining the lighting model:
/// @code
/// Final intensity:                If = Ia + Id + Is
/// Ambient intensity:              Ia = Al * Am
/// Ambient intensity of light:     Al
/// Ambient intensity of material:  Am
/// Diffuse intensity:              Id = Dl * Dm * LambertTerm
/// Diffuse intensity of light:     Dl
/// Diffuse intensity of material:  Dm
/// LambertTerm:                    max(Normal dot Light, 0.0)
/// Specular intensity:             Is = Sm * Sl * pow(max(R dot E, 0.0), f)
/// Specular intensity of light:    Sl
/// Specular intensity of material: Sm
/// @endcode
class Light: public SceneNode, public Movable, public Renderable,
	public Spatial
{
public:
	enum LightType
	{
		LT_POINT,
		LT_SPOT
	};

	/// @name Constructors
	/// @{
	Light(LightType t,
		const char* name, Scene* scene, // Scene
		uint movableFlags, Movable* movParent, // Movable
		CollisionShape* cs, // Spatial
		const char* smoMeshFname) // SMO mesh
		: SceneNode(name, scene), Movable(movableFlags, movParent),
			Spatial(cs), type(t)
	{
		smoMesh.load(smoMeshFname);
		vao.create();
		vao.attachArrayBufferVbo(smoMesh->getVbo(Mesh::VBO_VERT_POSITIONS),
			0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		vao.attachElementArrayBufferVbo(
			smoMesh->getVbo(Mesh::VBO_VERT_INDECES));
	}
	/// @}

	virtual ~Light();

	/// @name Accessors
	/// @{
	LightType getLightType() const
	{
		return type;
	}
	/// @}

	/// @name Virtuals
	/// @{
	Movable* getMovable()
	{
		return this;
	}

	Renderable* getRenderable()
	{
		return this;
	}

	Spatial* getSpatial()
	{
		return this;
	}

	const Vao& getVao(const PassLevelKey& k)
	{
		ANKI_ASSERT(k.pass == 1 && "Call only in SMO");
		(void)k; // No warnings please
		return vao;
	}

	uint getVertexIdsNum(const PassLevelKey& k)
	{
		ANKI_ASSERT(k.pass == 1 && "Call only in SMO");
		(void)k; // No warnings please
		return smoMesh->getVertIdsNum();
	}

	MaterialRuntime& getMaterialRuntime()
	{
		return *mtlr;
	}
	/// @}

private:
	LightType type;
	boost::scoped_ptr<MaterialRuntime> mtlr;
	MeshResourcePointer smoMesh; ///< Model for SMO pases
	Vao smoVao;
};


/// Point light
class PointLight: public Light
{
public:
	PointLight(const char* name, Scene* scene, uint movableFlags,
		Movable* movParent)
		: Light(LT_POINT, name, scene, movableFlags, movParent, &frustum,
			"sadf")
	{}

private:
	PerspectiveFrustum frustum;
};


/// Spot light
class SpotLight: public Light, public Frustumable
{
public:

private:
};


} // end namespace


#endif
