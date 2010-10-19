#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Properties.h"


/// Scenegraph node controller (A)
class Controller
{
	public:
		enum ControllerType
		{ 
			CT_SKEL_ANIM, 
			CT_SKEL,
			CT_MATERIAL,
			CT_LIGHT_MTL,
			CT_TRF,
			CT_LIGHT
		};
	
	PROPERTY_R(ControllerType, type, getType) ///< Once the type is set nothing can change it

	public:
		Controller(ControllerType type_);
		virtual ~Controller();
		virtual void update(float time) = 0;
};


#endif
