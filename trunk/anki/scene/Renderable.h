#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H


namespace anki {


class PassLevelKey;
class MaterialRuntime;
class Vao;
class Transform;


/// @addtogroup Scene
/// @{

/// Renderable interface
///
/// Implemented by renderable scene nodes
class Renderable
{
public:
	/// Get VAO depending the rendering pass
	virtual const Vao& getVao(const PassLevelKey& k) = 0;

	/// Get vert ids number for rendering
	virtual uint getVertexIdsNum(const PassLevelKey& k) = 0;

	/// Get the material runtime
	virtual MaterialRuntime& getMaterialRuntime() = 0;

	/// Get current transform
	virtual const Transform& getWorldTransform(const PassLevelKey& k) = 0;

	/// Get previous transform
	virtual const Transform& getPreviousWorldTransform(
		const PassLevelKey& k) = 0;
};
/// @}


} // end namespace


#endif
