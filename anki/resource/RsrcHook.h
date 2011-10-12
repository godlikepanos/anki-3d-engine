#ifndef ANKI_RESOURCE_RSRC_HOOK_H
#define ANKI_RESOURCE_RSRC_HOOK_H

#include <string>


namespace anki {


/// Holds information about a resource
template<typename Type>
struct RsrcHook
{
	std::string uuid; ///< Unique identifier
	int referenceCounter;
	Type* resource;
};


} // end namespace


#endif
