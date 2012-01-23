#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H


namespace anki {


class ModelPatchBase;
class Material;
class PropertyMap;


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

	/// Get the material runtime. Dont access it from the ModelPatchBase
	/// because the lights dont have one
	virtual Material& getMaterial() = 0;

	/// Access to property map to get the values of the shader variables
	virtual PropertyMap& getPropertyMap() = 0;

private:

};
/// @}


} // end namespace


#endif
