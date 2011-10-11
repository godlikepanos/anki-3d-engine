#ifndef ANKI_RESOURCE_RSRC_HOOK_H
#define ANKI_RESOURCE_RSRC_HOOK_H

#include <string>


/// Holds information about a resource
template<typename Type>
struct RsrcHook
{
	std::string uuid; ///< Unique identifier
	int referenceCounter;
	Type* resource;
};


#endif
