#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "common.h"


/// Scenegraph node controller (A)
class Controller
{
	public:
		enum Type
		{ 
			CT_SKEL_ANIM, 
			CT_SKEL,
			CT_MATERIAL,
			CT_LIGHT_MTL,
			CT_TRF,
			CT_LIGHT
		};
	
	PROPERTY_R( Type, type, getType ) ///< Once the type is set nothing can change it

	public:
		Controller( Type type_ );
		virtual ~Controller();
		virtual void update( float time ) = 0;
};


#endif
