#include "anki/scene/MaterialRuntime.h"
#include "anki/resource/Material.h"
#include "anki/resource/Texture.h"
#include "anki/resource/Resource.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
// MaterialRuntimeVariable                                                     =
//==============================================================================

//==============================================================================
template <>
void MaterialRuntimeVariable::ConstructVisitor::
	operator()<TextureResourcePointer >(const TextureResourcePointer& x) const
{
	var.data = &x;
}


//==============================================================================
MaterialRuntimeVariable::MaterialRuntimeVariable(
	const MaterialVariable& mvar_)
	: mvar(mvar_), buildinId(-1)
{
	// Initialize the data using a visitor
	boost::apply_visitor(ConstructVisitor(*this), mvar.getVariant());
}


//==============================================================================
MaterialRuntimeVariable::~MaterialRuntimeVariable()
{}



//==============================================================================
template<>
MaterialRuntimeVariable::ConstPtrRsrcPtrTexture&
	MaterialRuntimeVariable::getValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>()
{
	throw ANKI_EXCEPTION("You shouldn't call this getter");
	return boost::get<ConstPtrRsrcPtrTexture>(data);
}


//==============================================================================
template<>
void MaterialRuntimeVariable::setValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>(
	const ConstPtrRsrcPtrTexture& v)
{
	throw ANKI_EXCEPTION("You shouldn't call this setter");
	boost::get<ConstPtrRsrcPtrTexture>(data) = v;
}


//==============================================================================
// MaterialRuntime                                                             =
//==============================================================================

//==============================================================================
MaterialRuntime::MaterialRuntime(const Material& mtl_)
	: mtl(mtl_)
{
	// Copy props
	MaterialProperties& me = *this;
	const MaterialProperties& he = mtl.getBaseClass();
	me = he;

	// Create vars
	BOOST_FOREACH(const MaterialVariable& var, mtl.getVariables())
	{
		MaterialRuntimeVariable* varr = new MaterialRuntimeVariable(var);
		vars.push_back(varr);
		varNameToVar[varr->getMaterialVariable().getName().c_str()] = varr;
	}
}


//==============================================================================
MaterialRuntime::~MaterialRuntime()
{}


//==============================================================================
MaterialRuntimeVariable& MaterialRuntime::findVariableByName(
	const char* name)
{
	VariablesHashMap::iterator it = varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw ANKI_EXCEPTION("Cannot get material runtime variable "
			"with name \"" + name + '\"');
	}
	return *(it->second);
}


//==============================================================================
const MaterialRuntimeVariable& MaterialRuntime::findVariableByName(
	const char* name) const
{
	VariablesHashMap::const_iterator it = varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw ANKI_EXCEPTION("Cannot get material runtime variable "
			"with name \"" + name + '\"');
	}
	return *(it->second);
}


} // end namespace
