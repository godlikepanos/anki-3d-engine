#ifndef RSRC_HOOK_H
#define RSRC_HOOK_H

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
