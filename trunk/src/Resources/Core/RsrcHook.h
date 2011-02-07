#ifndef RSRC_HOOK_H
#define RSRC_HOOK_H

#include <string>


/// Holds information about a resource
template<typename Type>
struct RsrcHook
{
	RsrcHook(const char* uuid, int reference, Type* resource);

	std::string uuid; ///< Unique identifier
	int referenceCounter;
	Type* resource;
};


template<typename Type>
RsrcHook<Type>::RsrcHook(const char* uuid_, int referenceCounter_, Type* resource_): 
	uuid(uuid_),
	referenceCounter(referenceCounter_),
	resource(resource_)
{}


#endif
