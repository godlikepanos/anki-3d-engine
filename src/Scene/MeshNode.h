#ifndef _MESH_NODE_H_
#define _MESH_NODE_H_

#include "Common.h"
#include "SceneNode.h"
#include "Material.h"
#include "RsrcPtr.h"
#include "Mesh.h"
#include "MeshSkelNodeCtrl.h"


/// Mesh scene node
class MeshNode: public SceneNode
{
	private:
		void render(Material* mtl) const; ///< Common code for render() and renderDepth()

	public:
		// resources
		RsrcPtr<Mesh> mesh;
		RsrcPtr<Material> material;
		RsrcPtr<Material> dpMaterial; ///< Depth pass material
		// controllers
		MeshSkelNodeCtrl* meshSkelCtrl;
		// funcs
		MeshNode();
		virtual void render() { render(material.get()); }
		virtual void renderDepth() { render(material->dpMtl.get()); }
		void init(const char* filename);
		void deinit();
};


inline MeshNode::MeshNode():
	SceneNode(NT_MESH),
	meshSkelCtrl(NULL)
{}


#endif
