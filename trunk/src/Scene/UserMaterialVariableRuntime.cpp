#include "UserVariableRuntime.h"
#include "Resources/UserVariable.h"


//==============================================================================
// ConstructVisitor::operator() <RsrcPtr<Texture> >                            =
//==============================================================================
template <>
void UserVariableRuntime::ConstructVisitor::
	operator()<RsrcPtr<Texture> >(const RsrcPtr<Texture>& x) const
{
	udvr.data = &x;
}


//==============================================================================
// Constructor                                                                 =
//==============================================================================
UserVariableRuntime::UserVariableRuntime(
	const UserVariable& umv_)
:	umv(umv_)
{
	// Initialize the data using a visitor
	boost::apply_visitor(ConstructVisitor(*this), umv.getDataVariant());
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
UserVariableRuntime::~UserVariableRuntime()
{}


//==============================================================================
// Specialized Accessors                                                       =
//==============================================================================

template<>
UserVariableRuntime::ConstPtrRsrcPtrTexture&
	UserVariableRuntime::getValue<
	UserVariableRuntime::ConstPtrRsrcPtrTexture>()
{
	throw EXCEPTION("You shouldn't call this getter");
	return boost::get<ConstPtrRsrcPtrTexture>(data);
}


template<>
void UserVariableRuntime::setValue<
	UserVariableRuntime::ConstPtrRsrcPtrTexture>(
	const ConstPtrRsrcPtrTexture& v)
{
	throw EXCEPTION("You shouldn't call this setter");
	boost::get<ConstPtrRsrcPtrTexture>(data) = v;
}
