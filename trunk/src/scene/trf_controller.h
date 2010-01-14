#ifndef _TRF_CONTROLLER_H_
#define _TRF_CONTROLLER_H_

#include "common.h"
#include "controller.h"


/// Transform controller
class trf_controller_t: public controller_t
{
	public:
		node_t* node;
	
		trf_controller_t( node_t* node_ ): controller_t( CT_TRF ), node(node_) {}
		void Update( float ) { /* ToDo */ }
};

#endif
