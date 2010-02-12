#ifndef _TRF_SCRIPT_CTRL_H_
#define _TRF_SCRIPT_CTRL_H_

#include "common.h"
#include "controller.h"


/// Transform controller
class trf_script_ctrl_t: public controller_t
{
	public:
		node_t* node;
	
		trf_script_ctrl_t( node_t* node_ ): controller_t( CT_TRF ), node(node_) {}
		void Update( float ) { /* ToDo */ }
};

#endif
