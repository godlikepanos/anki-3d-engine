#ifndef _SKEL_NODE_H_
#define _SKEL_NODE_H_

#include "common.h"
#include "node.h"
#include "controller.h"
#include "gmath.h"


/// Skeleton node
class skel_node_t: public node_t
{
	public:
		skeleton_t* skeleton;
		skel_anim_controller_t* skel_anim_controller;

		skel_node_t(): node_t( NT_SKELETON ) {}
		void Render();
		void Init( const char* filename );
		void Deinit();
};


#endif
