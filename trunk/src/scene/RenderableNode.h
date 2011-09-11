#ifndef RENDERABLE_NODE_H
#define RENDERABLE_NODE_H

#include "SceneNode.h"
#include "rsrc/MaterialCommon.h"


class Vao;
class Material;
class MaterialRuntime;


/// Abstract class that acts as an interface for the renderable objects of the
/// scene
class RenderableNode: public SceneNode
{
	public:
		RenderableNode(ClassId cid, Scene& scene, ulong flags,
			SceneNode* parent);
		virtual ~RenderableNode();

		static bool classof(const SceneNode* x);

		/// Get VAO depending the rendering pass
		virtual const Vao& getVao(PassType p) const = 0;

		/// Get vert ids number for rendering
		virtual uint getVertIdsNum() const = 0;

		/// Get the material resource
		virtual const Material& getMaterial() const = 0;

		/// Get the material runtime
		virtual MaterialRuntime& getMaterialRuntime() = 0;

		/// Const version of getMaterialRuntime
		virtual const MaterialRuntime& getMaterialRuntime() const = 0;
};


inline bool RenderableNode::classof(const SceneNode* x)
{
	return x->getClassId() == CID_RENDERABLE_NODE ||
		x->getClassId() == CID_PATCH_NODE ||
		x->getClassId() == CID_MODEL_PATCH_NODE ||
		x->getClassId() == CID_SKIN_PATCH_NODE;
}


#endif
