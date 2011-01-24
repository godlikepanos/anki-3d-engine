#ifndef SCENE_NODE_PATCH_H
#define SCENE_NODE_PATCH_H

#include "SceneNode.h"


class Vao;
class Material;


/// Abstract class, patch of a compound SceneNode derivative.
/// Despite of what the name suggests the class is not a SceneNode derivative.
class SceneNodePatch
{
	public:
		SceneNodePatch(const SceneNode& father_): father(father_) {}

		virtual const Vao& getCpVao() const = 0; ///< Get color pass VAO
		virtual const Vao& getDpVao() const = 0; ///< Get depth pass VAO
		virtual const Material& getCpMtl() const = 0;  ///< Get color pass material
		virtual const Material& getDpMtl() const = 0;  ///< Get depth pass material
		const Transform& getWorldTransform() const {return father.getWorldTransform();}

	private:
		const SceneNode& father;
};


#endif
