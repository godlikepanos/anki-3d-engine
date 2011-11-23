#include "anki/scene/MaterialRuntimeVariable.h"
#include "anki/resource/MaterialVariable.h"


namespace anki {


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
:	mvar(mvar_)
{
	// Initialize the data using a visitor
	boost::apply_visitor(ConstructVisitor(*this), mvar.getVariant());
}


//==============================================================================
MaterialRuntimeVariable::~MaterialRuntimeVariable()
{}


//==============================================================================
// Specialized Accessors                                                       =
//==============================================================================

template<>
MaterialRuntimeVariable::ConstPtrRsrcPtrTexture&
	MaterialRuntimeVariable::getValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>()
{
	throw ANKI_EXCEPTION("You shouldn't call this getter");
	return boost::get<ConstPtrRsrcPtrTexture>(data);
}


template<>
void MaterialRuntimeVariable::setValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>(
	const ConstPtrRsrcPtrTexture& v)
{
	throw ANKI_EXCEPTION("You shouldn't call this setter");
	boost::get<ConstPtrRsrcPtrTexture>(data) = v;
}


} // end namespace
