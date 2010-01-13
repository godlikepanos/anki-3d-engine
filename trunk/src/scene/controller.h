#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "common.h"


/// Scenegraph node controller (A)
class controller_t
{
	public:
		enum type_e
		{ 
			CT_SKEL_ANIM, 
			CT_SKEL,
			CT_MATERIAL,
			CT_LIGHT_MTL,
			CT_TRF
		};
	
	PROPERTY_R( type_e, type, GetType ) ///< Once the type is set nothing can change it

	public:
		controller_t( type_e type_ ): type(type_) {}
		virtual ~controller_t() {}
		virtual void Update( float time ) = 0;
};


#endif
