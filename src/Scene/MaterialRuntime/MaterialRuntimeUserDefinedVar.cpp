#include "MaterialRuntimeUserDefinedVar.h"


//======================================================================================================================
// ConstructVisitor::operator() <RsrcPtr<Texture> >                                                                    =
//======================================================================================================================
template <>
void MaterialRuntimeUserDefinedVar::ConstructVisitor::operator()<RsrcPtr<Texture> >(const RsrcPtr<Texture>& x) const
{
	udvr.data = x.get();
}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MaterialRuntimeUserDefinedVar::MaterialRuntimeUserDefinedVar(const MtlUserDefinedVar& rsrc_):
	rsrc(rsrc_)
{
	// Initialize the data using a visitor
	boost::apply_visitor(ConstructVisitor(*this), rsrc.getDataVariant());
}
