#include "anki/scene/MaterialRuntimeVariable.h"
#include "anki/resource/MaterialUserVariable.h"


namespace anki {


//==============================================================================
// ConstructVisitor::operator() <TextureResourcePointer >                            =
//==============================================================================
template <>
void MaterialRuntimeVariable::ConstructVisitor::
	operator()<TextureResourcePointer >(const TextureResourcePointer& x) const
{
	udvr.data = &x;
}


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialRuntimeVariable::MaterialRuntimeVariable(
	const MaterialUserVariable& umv_)
:	umv(umv_)
{
	// Initialize the data using a visitor
	boost::apply_visitor(ConstructVisitor(*this), umv.getDataVariant());
}


//==============================================================================
// Destructor                                                                  =
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
