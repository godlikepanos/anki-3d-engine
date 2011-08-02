#include "UserMaterialVariableRuntime.h"
#include "Resources/UserMaterialVariable.h"


//==============================================================================
// ConstructVisitor::operator() <RsrcPtr<Texture> >                            =
//==============================================================================
template <>
void UserMaterialVariableRuntime::ConstructVisitor::
	operator()<RsrcPtr<Texture> >(const RsrcPtr<Texture>& x) const
{
	udvr.data = &x;
}


//==============================================================================
// Constructor                                                                =
//==============================================================================
UserMaterialVariableRuntime::UserMaterialVariableRuntime(
	const UserMaterialVariable& umv_)
:	umv(umv_)
{
	// Initialize the data using a visitor
	boost::apply_visitor(ConstructVisitor(*this), umv.getDataVariant());
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
UserMaterialVariableRuntime::~UserMaterialVariableRuntime()
{}


//==============================================================================
// Specialized Accessors                                                       =
//==============================================================================

template<>
UserMaterialVariableRuntime::ConstPtrRsrcPtrTexture&
	UserMaterialVariableRuntime::getValue<
	UserMaterialVariableRuntime::ConstPtrRsrcPtrTexture>()
{
	throw EXCEPTION("You shouldn't call this getter");
	return boost::get<ConstPtrRsrcPtrTexture>(data);
}


template<>
void UserMaterialVariableRuntime::setValue<
	UserMaterialVariableRuntime::ConstPtrRsrcPtrTexture>(
	const ConstPtrRsrcPtrTexture& v)
{
	throw EXCEPTION("You shouldn't call this setter");
	boost::get<ConstPtrRsrcPtrTexture>(data) = v;
}
