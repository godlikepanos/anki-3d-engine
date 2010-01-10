#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "common.h"


template<typename type_t> class controller_t
{
	public:
		enum type_e { CT_SKEL_ANIM, CT_SKEL };

		controller_t() {}
		virtual ~controller_t() {}
		virtual void Update() = 0;
};


#endif
