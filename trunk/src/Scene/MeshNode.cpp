#include "MeshNode.h"
#include "Resource.h"
#include "Mesh.h"
#include "Material.h"
#include "SkelNode.h"
#include "Skeleton.h"
#include "MeshSkelNodeCtrl.h"
#include "SkelAnimCtrl.h"
#include "App.h"
#include "MainRenderer.h"
#include "MeshData.h" // For some sizeof


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void MeshNode::init(const char* filename)
{
	mesh.loadRsrc(filename);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void MeshNode::render(Material& mtl, Vao& vao) const
{
	GLint loc;
	GLint locs[64];
	int locsNum = 0;

	// if we have skeleton controller
	if(meshSkelCtrl)
	{
		RASSERT_THROW_EXCEPTION(!mtl.hasHWSkinning()); // it has skel controller but no skinning

		// first the uniforms
		mtl.stdUniVars[Material::SUV_SKINNING_ROTATIONS]->setMat3(&meshSkelCtrl->skelNode->boneRotations[0],
		                                                          meshSkelCtrl->skelNode->skeleton->bones.size());

		mtl.stdUniVars[Material::SUV_SKINNING_TRANSLATIONS]->setVec3(&meshSkelCtrl->skelNode->boneTranslations[0],
		                                                             meshSkelCtrl->skelNode->skeleton->bones.size());
	}


	vao.bind();
	glDrawElements(GL_TRIANGLES, mesh->getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
	vao.unbind();
}
