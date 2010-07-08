/*
#ifndef _MAP_H_
#define _MAP_H_

#include "Common.h"
#include "collision.h"
#include "Vec.h"
#include "RsrcPtr.h"

class Mesh;
class Camera;



=======================================================================================================================================
octree_t                                                                                                               =
=======================================================================================================================================

class octree_t
{
	public:
		/// SceneNode class
		// the class does not contain complicated functions. It mainly holds the data
		class node_t
		{
			public:
				node_t* childs[8];
				aabb_t  bounding_box;

				Vec<RsrcPtr<Mesh> >   meshes;
				Vec< Vec<uint> > vertIds;
				Vec< Vec<uint> > face_ids;

				node_t() {}
				~node_t() { ToDo: when class is finalized add code }

				uint GetMeshesNum() const { return meshes.size(); }
				uint GetFacesNum() const;
		};
		/// end SceneNode class

	protected:
		// funcs for the tree creation
		bool IsSubdivHeuristicMet(node_t* node) const;
		void SubdivideNode(node_t* node);
		void CreateRoot(const Vec<Mesh*>& meshes);

		// frustum funcs
		uint CheckNodeAgainstFrustum(node_t* node, const Camera& cam) const;

	public:
		node_t* root;

		void CreateTree(const Vec<Mesh*>& meshes);
};



=======================================================================================================================================
map_t                                                                                                                  =
=======================================================================================================================================

class map_t
{
	public:
		Vec<Mesh*> meshes;
		octree_t             octree;

		bool load(const char* filename);
		void CreateOctree() { DEBUG_ERR(meshes.size() < 1); octree.CreateTree(meshes); };
};


#endif
*/
