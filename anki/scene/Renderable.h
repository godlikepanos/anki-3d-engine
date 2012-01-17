#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H


namespace anki {


class PassLevelKey;
class MaterialRuntime;
class MeshBase;
class Transform;


/// @addtogroup Scene
/// @{

/// Renderable interface
///
/// Implemented by renderable scene nodes
class Renderable
{
public:
	/// Access to VAOs
	virtual const ModelPatchBase* getModelPatchBase() const
	{
		return NULL;
	}

	/// Get the material runtime
	virtual MaterialRuntime& getMaterialRuntime() = 0;

	/// Get current transform
	virtual const Transform* getWorldTransform(const PassLevelKey& k)
	{
		return NULL;
	}

	/// Get previous transform
	virtual const Transform* getPreviousWorldTransform(
		const PassLevelKey& k)
	{
		return NULL;
	}

	/// Get projection matrix (for lights)
	virtual const Mat4* getProjectionMatrix() const
	{
		return NULL;
	}

	/// Get view matrix (for lights)
	virtual const Mat4* getViewMatrix() const
	{
		return NULL;
	}
};
/// @}


} // end namespace


#endif
