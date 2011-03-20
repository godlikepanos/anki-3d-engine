#ifndef RENDERABLE_NODE_H
#define RENDERABLE_NODE_H

#include "SceneNode.h"


class Vao;
class Material;
class MaterialRuntime;


/// Abstract class that acts as an interface for the renderable objects of the scene
class RenderableNode: public SceneNode
{
	public:
		RenderableNode(SceneNode* parent);

		virtual const Vao& getCpVao() const = 0; ///< Get color pass VAO
		virtual const Vao& getDpVao() const = 0; ///< Get depth pass VAO
		virtual uint getVertIdsNum() const = 0;  ///< Get vert ids number for rendering
		virtual const Material& getCpMtl() const = 0;  ///< Get color pass material
		virtual const Material& getDpMtl() const = 0;  ///< Get depth pass material
		virtual MaterialRuntime& getCpMtlRun() = 0;
		virtual MaterialRuntime& getDpMtlRun() = 0;
		virtual const MaterialRuntime& getCpMtlRun() const = 0;
		virtual const MaterialRuntime& getDpMtlRun() const = 0;
};


inline RenderableNode::RenderableNode(SceneNode* parent):
	SceneNode(SNT_RENDERABLE, false, parent)
{}


#endif
