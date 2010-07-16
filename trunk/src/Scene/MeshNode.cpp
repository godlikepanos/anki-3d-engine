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


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void MeshNode::init(const char* filename)
{
	mesh.loadRsrc(filename);
	material.loadRsrc(mesh->materialName.c_str());

	// sanity checks
	if(material->stdAttribVars[Material::SAV_TEX_COORDS]!=NULL && mesh->vbos.texCoords.getGlId()==0)
	{
		ERROR("The shader program (\"" << material->shaderProg->getRsrcName() <<
		       "\") needs texture coord information that the mesh (\"" <<
		       mesh->getRsrcName() << "\") doesn't have");
	}
}


//======================================================================================================================
// deinit                                                                                                              =
//======================================================================================================================
void MeshNode::deinit()
{
	//RsrcMngr::meshes.unload(mesh);
	//RsrcMngr::materials.unload(material);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void MeshNode::render(Material* mtl) const
{
	GLint loc;
	GLint locs[64];
	int locsNum = 0;

	// if we have skeleton controller
	if(meshSkelCtrl)
	{
		DEBUG_ERR(!mtl->hasHWSkinning()); // it has skel controller but no skinning

		// first the uniforms
		mtl->stdUniVars[Material::SUV_SKINNING_ROTATIONS]->setMat3(&meshSkelCtrl->skelNode->skelAnimCtrl->boneRotations[0],
		                                                              meshSkelCtrl->skelNode->skeleton->bones.size());

		mtl->stdUniVars[Material::SUV_SKINNING_TRANSLATIONS]->setVec3(&meshSkelCtrl->skelNode->skelAnimCtrl->boneTranslations[0],
		                                                                 meshSkelCtrl->skelNode->skeleton->bones.size());

		// then the attributes
		mesh->vbos.vertWeights.bind();

		loc = mtl->stdAttribVars[Material::SAV_VERT_WEIGHT_BONES_NUM]->getLoc();
		glEnableVertexAttribArray(loc);
		locs[locsNum++] = loc;
		glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(0));

		loc = mtl->stdAttribVars[Material::SAV_VERT_WEIGHT_BONE_IDS]->getLoc();
		glEnableVertexAttribArray(loc);
		locs[locsNum++] = loc;
		glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(4));

		loc = mtl->stdAttribVars[Material::SAV_VERT_WEIGHT_WEIGHTS]->getLoc();
		glEnableVertexAttribArray(loc);
		locs[locsNum++] = loc;
		glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(20));
	}

	if(mtl->stdAttribVars[Material::SAV_POSITION] != NULL)
	{
		mesh->vbos.vertCoords.bind();
		loc = mtl->stdAttribVars[Material::SAV_POSITION]->getLoc();
		glVertexAttribPointer(loc, 3, GL_FLOAT, false, 0, NULL);
		glEnableVertexAttribArray(loc);
		locs[locsNum++] = loc;
	}

	if(mtl->stdAttribVars[Material::SAV_NORMAL] != NULL)
	{
		mesh->vbos.vertNormals.bind();
		loc = mtl->stdAttribVars[Material::SAV_NORMAL]->getLoc();
		glVertexAttribPointer(loc, 3, GL_FLOAT, false, 0, NULL);
		glEnableVertexAttribArray(loc);
		locs[locsNum++] = loc;
	}

	if(mtl->stdAttribVars[Material::SAV_TEX_COORDS] != NULL)
	{
		mesh->vbos.texCoords.bind();
		loc = mtl->stdAttribVars[Material::SAV_TEX_COORDS]->getLoc();
		glVertexAttribPointer(loc, 2, GL_FLOAT, false, 0, NULL);
		glEnableVertexAttribArray(loc);
		locs[locsNum++] = loc;
	}

	if(mtl->stdAttribVars[Material::SAV_TANGENT] != NULL)
	{
		mesh->vbos.vertTangents.bind();
		loc = mtl->stdAttribVars[Material::SAV_TANGENT]->getLoc();
		glVertexAttribPointer(loc, 4, GL_FLOAT, false, 0, NULL);
		glEnableVertexAttribArray(loc);
		locs[locsNum++] = loc;
	}

	mesh->vbos.vertIndeces.bind();

	glDrawElements(GL_TRIANGLES, mesh->vertIndeces.size(), GL_UNSIGNED_SHORT, 0);

	// disable
	for(int i=0; i<locsNum; i++)
	{
		glDisableVertexAttribArray(locs[i]);
	}

	Vbo::unbindAllTargets();
}
