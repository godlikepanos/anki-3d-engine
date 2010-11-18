#ifndef MESH_NODE_H
#define MESH_NODE_H

#include "SceneNode.h"
#include "Material.h"
#include "RsrcPtr.h"
#include "Mesh.h"
#include "MeshSkelNodeCtrl.h"


class Vao;


/// Mesh scene node
class MeshNode: public SceneNode
{
	private:
		void render(const Material& mtl, const Vao& vao) const; ///< Common code for render() and renderDepth()

	public:
		// resources
		RsrcPtr<Mesh> mesh;
		// controllers
		MeshSkelNodeCtrl* meshSkelCtrl;
		// funcs
		MeshNode();
		virtual void render() { render(*mesh->material.get(), *mesh->getVao()); }
		virtual void renderDepth() { render(mesh->material->getDepthMtl(), *mesh->getDepthVao()); }
		void init(const char* filename);
};


inline MeshNode::MeshNode():
	SceneNode(SNT_MESH),
	meshSkelCtrl(NULL)
{}


#endif
