#ifndef _SKEL_NODE_H_
#define _SKEL_NODE_H_

#include "common.h"
#include "node.h"
#include "controller.h"
#include "gmath.h"

class skeleton_t;
class skel_anim_ctrl_t;


/// Skeleton node
class skel_node_t: public node_t
{
	public:
		skeleton_t* skeleton; ///< The skeleton resource
		skel_anim_ctrl_t* skel_anim_ctrl; ///< Hold the controller here as well

		skel_node_t();
		void Render();
		void Init( const char* filename );
		void Deinit();
};


#endif
