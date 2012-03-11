#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H

#include "anki/scene/Property.h"
#include <vector>
#include <boost/range/iterator_range.hpp>


namespace anki {


class ModelPatchBase;
class Material;
class MaterialVariable;
class Light;


/// @addtogroup Scene
/// @{

/// XXX
template<typename T>
class MaterialVariableProperty: public ReadCowPointerProperty<T>
{
public:
	typedef T Value;
	typedef ReadCowPointerProperty<T> Base;

	/// @name Constructors/Destructor
	/// @{
	MaterialVariableProperty(const char* name, const Value* x, bool buildin_)
		: Base(name, x), buildin(buildin_)
	{}
	/// @}

	/// @name Accessors
	/// @{
	bool isBuildIn() const
	{
		return buildin;
	}
	/// @}

private:
	bool buildin;
};


/// Renderable interface
///
/// Implemented by renderable scene nodes
class Renderable
{
public:
	/// Renderable flags
	enum RenderableFlag
	{
		RF_NONE = 0,
		RF_VISIBLE = 1 ///< Visible or not. The visibility tester sets it
	};

	typedef std::vector<PropertyBase*> Properties;
	typedef boost::iterator_range<Properties::iterator> MutableRange;
	typedef boost::iterator_range<Properties::const_iterator> ConstRange;

	Renderable()
		: flags(RF_NONE)
	{}

	virtual ~Renderable()
	{}

	/// Access to VAOs
	virtual const ModelPatchBase& getModelPatchBase() const = 0;

	/// Access the material
	virtual const Material& getMaterial() const = 0;

	virtual Light* getLight()
	{
		return nullptr;
	}

	MutableRange getProperties()
	{
		return MutableRange(props.begin(), props.end());
	}

	ConstRange getProperties() const
	{
		return ConstRange(props.begin(), props.end());
	}

	/// @name Flag manipulation
	/// @{
	void enableFlag(RenderableFlag flag, bool enable = true)
	{
		flags = enable ? flags | flag : flags & ~flag;
	}
	void disableFlag(RenderableFlag flag)
	{
		enableFlag(flag, false);
	}
	bool isFlagEnabled(RenderableFlag flag) const
	{
		return flags & flag;
	}
	uint getFlagsBitmask() const
	{
		return flags;
	}
	/// @}

protected:
	void init(PropertyMap& pmap);

private:
	Properties props;
	uint flags; ///< Bitmask
};


/// @}


} // end namespace


#endif
