/*
#include <limits>
#include "map.h"
#include "Resources/Mesh.h"
#include "Scanner.h"
#include "Parser.h"
#include "Resource.h"
#include "Scene/Camera.h"



=======================================================================================================================================
CreateRoot                                                                     =
=======================================================================================================================================

void octree_t::CreateRoot(const Vec<Mesh*>& meshes)
{
	DEBUG_ERR(root); // root should be NULL

	/// get the root's aabb size
	Vec3 min(numeric_limits<float>::max()), max(numeric_limits<float>::min());

	for(uint m=0; m<meshes.size(); m++)
	{
		Mesh* cmesh = meshes[m];
		for(uint v=0; v<cmesh->vertCoords.size(); v++)
		{
			const Vec3& vertCoords = cmesh->vertCoords[v];
			for(int i=0; i<3; i++)
			{
				if(vertCoords[i] > max[i])
					max[i] = vertCoords[i];
				else if(vertCoords[i] < min[i])
					min[i] = vertCoords[i];
			} // end for 3 times
		} // end for all mesh verts
	} // end for all meshes


	/// create a new node
	node_t* node = new node_t;
	node->bounding_box.min = min;
	node->bounding_box.max = max;


	/// create the face and vert ids
	DEBUG_ERR(node->face_ids.size() != 0 || node->vertIds.size() != 0 || node->meshes.size() != 0); // vectors not empty. wrong node init

	node->face_ids.resize(meshes.size());
	node->vertIds.resize(meshes.size());
	node->meshes.resize(meshes.size());

	for(uint m=0; m<meshes.size(); m++)
	{
		Mesh* cmesh = meshes[m];

		// first set the mesh
		node->meshes[m] = cmesh;

		// then set the face_ids
		node->face_ids[m].resize(cmesh->tris.size());
		for(uint f=0; f<cmesh->tris.size(); f++)
			node->face_ids[m][f] = f; // simple as taking a shit

		// and last the verts
	}

	/// set root
	root = node;
}




=======================================================================================================================================
GetFacesNum                                                                    =
=======================================================================================================================================

uint octree_t::node_t::GetFacesNum() const
{
	int count = 0;
	for(uint i=0; i<meshes.size(); i++)
	{
		count += meshes[i]->tris.size();
	}
	return count;
}



=======================================================================================================================================
IsSubdivHeuristicMet                                                           =
returns true when the used difined heuristic is met that sais that we can subdivide the node. Long story short it returns true when   =
we can subdivide the node further                                              =
=======================================================================================================================================

bool octree_t::IsSubdivHeuristicMet(node_t* node) const
{
	if(node->GetFacesNum() < 100) return false;

	return true;
}




=======================================================================================================================================
SubdivideNode                                                                  =
subdivides the node and creates max 8 children and then subdivides the children=
=======================================================================================================================================

void octree_t::SubdivideNode(node_t* node)
{
	if(!IsSubdivHeuristicMet(node)) return;

	// subdivide the children
	for(int i=0; i<8; i++)
	{
		if(node->childs[i] == NULL) continue;

		SubdivideNode(node->childs[i]);
	}
}



=======================================================================================================================================
CreateTree                                                                     =
=======================================================================================================================================

void octree_t::CreateTree(const Vec<Mesh*>& meshes)
{
	CreateRoot(meshes);
	SubdivideNode(root);
}



=======================================================================================================================================
CheckNodeAgainstFrustum                                                        =
the func checks the node and returns if its inside the cameras fruntum. It returns 0 if the cube is not inside, 1 if partialy         =
inside and 2 if totaly inside                                                  =
=======================================================================================================================================

uint octree_t::CheckNodeAgainstFrustum(node_t* node, const Camera& cam) const
{
	int points_outside_frustum_num = 0;
	const aabb_t& box = node->bounding_box;
	Vec3 box_points[] = { box.max, Vec3(box.min.x, box.max.y, box.max.z), Vec3(box.min.x, box.min.y, box.max.z), Vec3(box.max.x, box.min.y, box.max.z),
	                        box.min, Vec3(box.min.x, box.max.y, box.min.z), Vec3(box.min.x, box.min.y, box.min.z), Vec3(box.max.x, box.min.y, box.min.z), };

	for(int i=0; i<8; i++)
	{
		for(int j=0; j<6; j++)
		{
			const plane_t& plane = cam.wspaceFrustumPlanes[j];

			if(plane.Test(box_points[i]) < 0.0)
			{
				++points_outside_frustum_num;
				continue;
			}
		}
	}

	if(points_outside_frustum_num == 8) return 0;
	if(points_outside_frustum_num < 8) return 1;
	return 2;
}




=======================================================================================================================================
map                                                                            =
=======================================================================================================================================




=======================================================================================================================================
load                                                                           =
=======================================================================================================================================

bool map_t::load(const char* filename)
{
	DEBUG_ERR(meshes.size() != 0); // meshes vector should be empty

	Scanner scanner;
	const Scanner::Token* token;
	if(!scanner.loadFile(filename)) return false;

	do
	{
		token = &scanner.getNextToken();

		// strings is what we want in this case... please let it be G-Strings
		if(token->getCode() == Scanner::TC_STRING)
		{
			RsrcPtr<Mesh> mesh = RsrcMngr::meshes.load(token->getValue().getString());
			if(!mesh.get()) return false;

			meshes.push_back(mesh);
		}
		// end of file
		else if(token->getCode() == Scanner::TC_EOF)
		{
			break;
		}
		// other crap
		else
		{
			PARSE_ERR_UNEXPECTED();
			return false;
		}

	}while(true);

	return true;
}

*/
