#ifndef SKIN_NODE_H
#define SKIN_NODE_H

#include "SceneNode.h"
#include "SkinPatchNode.h"
#include "Vec.h"


class Skin;


/// A skin scene node
class SkinNode: public SceneNode
{
	public:
		SkinNode(): SceneNode(SNT_SKIN, true, NULL) {}

		void init(const char* filename);

	private:
		RsrcPtr<Skin> skin; ///< The resource
		Vec<SkinPatchNode*> patches;
};


#endif
