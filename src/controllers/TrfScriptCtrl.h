#ifndef _TRF_SCRIPT_CTRL_H_
#define _TRF_SCRIPT_CTRL_H_

#include "common.h"
#include "Controller.h"


/// Transformation controlled by a script
class TrfScriptCtrl: public Controller
{
	public:
		Node* node;
	
		TrfScriptCtrl( Node* node_ ): Controller( CT_TRF ), node(node_) {}
		void Update( float ) { /* ToDo */ }
};

#endif
