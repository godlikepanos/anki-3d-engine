#include "MtlUserDefinedVarRuntime.h"


//======================================================================================================================
// Visitor functors                                                                                                    =
//======================================================================================================================

void MtlUserDefinedVarRuntime::ConstructVisitor::operator()(float x) const
{
	udvr.data = x;
}

void MtlUserDefinedVarRuntime::ConstructVisitor::operator()(const Vec2& x) const
{
	udvr.data = x;
}

void MtlUserDefinedVarRuntime::ConstructVisitor::operator()(const Vec3& x) const
{
	udvr.data = x;
}

void MtlUserDefinedVarRuntime::ConstructVisitor::operator()(const Vec4& x) const
{
	udvr.data = x;
}

void MtlUserDefinedVarRuntime::ConstructVisitor::operator()(const RsrcPtr<Texture>& x) const
{
	udvr.data = &x;
}

void MtlUserDefinedVarRuntime::ConstructVisitor::operator()(MtlUserDefinedVar::Fai x) const
{
	udvr.data = x;
}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MtlUserDefinedVarRuntime::MtlUserDefinedVarRuntime(const MtlUserDefinedVar& rsrc_):
	rsrc(rsrc_)
{
	// Initialize the data using a visitor
	boost::apply_visitor(ConstructVisitor(*this), rsrc.getDataVariant());
}
