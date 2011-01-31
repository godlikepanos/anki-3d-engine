#ifndef SKIN_NODE_H
#define SKIN_NODE_H

#include <boost/array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "SceneNode.h"
#include "SkinPatchNode.h"


class Skin;


/// A skin scene node
class SkinNode: public SceneNode
{
	public:
		SkinNode(): SceneNode(SNT_SKIN, true, NULL) {}

		void init(const char* filename);

	private:
		RsrcPtr<Skin> skin;
		//boost::ptr_vector<SkinNodePatch> patches;
};


#endif
