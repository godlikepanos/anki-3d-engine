#ifndef _MESH_NODE_H_
#define _MESH_NODE_H_

#include "Common.h"
#include "SceneNode.h"
#include "Material.h"

class MeshSkelNodeCtrl;
class Mesh;


/// Mesh scene node
class MeshNode: public SceneNode
{
	private:
		void render(Material* mtl) const; ///< Common code for render() and renderDepth()

	public:
		// resources
		Mesh* mesh;
		Material* material;
		Material* dpMaterial; ///< Depth pass material
		// controllers
		MeshSkelNodeCtrl* meshSkelCtrl;
		// funcs
		MeshNode(): SceneNode(NT_MESH), material(NULL), dpMaterial(NULL), meshSkelCtrl(NULL) {}
		virtual void render() { render(material); }
		virtual void renderDepth() { render(material->dpMtl); }
		void init(const char* filename);
		void deinit();
};


#endif
