#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H

#include "anki/scene/Property.h"


namespace anki {


class ModelPatchBase;
class Material;
class MaterialVariable;


/// @addtogroup Scene
/// @{


/// XXX
template<typename T>
class MaterialVariableReadCowPointerProperty: public ReadCowPointerProperty<T>
{
public:
	typedef T Value;
	typedef ReadCowPointerProperty<T> Base;

	/// @name Constructors/Destructor
	/// @{
	MaterialVariableReadCowPointerProperty(const char* name, const Value* x)
		: Base(name, x)
	{}
	/// @}
private:
};


/// Renderable interface
///
/// Implemented by renderable scene nodes
class Renderable
{
public:
	/// Access to VAOs
	virtual const ModelPatchBase& getModelPatchBase() const = 0;

	/// Access the material
	virtual const Material& getMaterial() const = 0;

protected:
	void init(PropertyMap& pmap) const;
};
/// @}


} // end namespace


#endif
