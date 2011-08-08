#ifndef RENDERABLE_NODE_H
#define RENDERABLE_NODE_H

#include "SceneNode.h"


class Vao;
class Material;
class MaterialRuntime;


/// Abstract class that acts as an interface for the renderable objects of the
/// scene
class RenderableNode: public SceneNode
{
	public:
		RenderableNode(bool inheritParentTrfFlag, SceneNode* parent);

		virtual const Vao& getCpVao() const = 0; ///< Get color pass VAO
		virtual const Vao& getDpVao() const = 0; ///< Get depth pass VAO

		/// Get vert ids number for rendering
		virtual uint getVertIdsNum() const = 0;

		virtual const Material& getMaterial() const = 0;

		virtual MaterialRuntime& getMaterialRuntime() = 0;
		virtual const MaterialRuntime& getMaterialRuntime() const = 0;
};


inline RenderableNode::RenderableNode(bool inheritParentTrfFlag,
	SceneNode* parent)
:	SceneNode(SNT_RENDERABLE, inheritParentTrfFlag, parent)
{}


#endif
