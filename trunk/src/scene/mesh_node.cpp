#include "mesh_node.h"
#include "Resource.h"
#include "Mesh.h"
#include "renderer.h"
#include "Material.h"
#include "skel_node.h"
#include "Skeleton.h"
#include "mesh_skel_ctrl.h"
#include "skel_anim_ctrl.h"


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void mesh_node_t::Init( const char* filename )
{
	mesh = rsrc::meshes.load( filename );
	material = rsrc::materials.load( mesh->materialName.c_str() );

	// sanity checks
	if( material->attribLocs.texCoords != -1 && mesh->vbos.texCoords.getGlId() == 0 )
	{
		ERROR( "The shader program needs information that the mesh do not have" );
	}
}


//=====================================================================================================================================
// Deinit                                                                                                                             =
//=====================================================================================================================================
void mesh_node_t::Deinit()
{
	rsrc::meshes.unload( mesh );
	rsrc::materials.unload( material );
}


//=====================================================================================================================================
// Render                                                                                                                             =
//=====================================================================================================================================
/// Called in material or blending stages
void mesh_node_t::Render( Material* mtl ) const
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	// if we have skeleton controller
	if( mesh_skel_ctrl )
	{
		// first the uniforms
		glUniformMatrix3fv( mtl->uniLocs.skinningRotations, mesh_skel_ctrl->skel_node->skeleton->bones.size(), 1,
		                    &(mesh_skel_ctrl->skel_node->skel_anim_ctrl->bone_rotations[0])[0] );
		glUniform3fv( mtl->uniLocs.skinningTranslations, mesh_skel_ctrl->skel_node->skeleton->bones.size(),
		              &(mesh_skel_ctrl->skel_node->skel_anim_ctrl->Boneranslations[0])[0] );

		// then the attributes
		DEBUG_ERR( !mtl->hasHWSkinning() );

		mesh->vbos.vertWeights.Bind();
		glEnableVertexAttribArray( mtl->attribLocs.vertWeightBonesNum );
		glVertexAttribPointer( mtl->attribLocs.vertWeightBonesNum, 1, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(0) );
		glEnableVertexAttribArray( mtl->attribLocs.vertWeightBoneIds );
		glVertexAttribPointer( mtl->attribLocs.vertWeightBoneIds, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(4) );
		glEnableVertexAttribArray( mtl->attribLocs.vertWeightWeights );
		glVertexAttribPointer( mtl->attribLocs.vertWeightWeights, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(20) );
	}

	if( mtl->attribLocs.position != -1 )
	{
		mesh->vbos.vertCoords.Bind();
		glVertexAttribPointer( mtl->attribLocs.position, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.position );
	}

	if( mtl->attribLocs.normal != -1 )
	{
		mesh->vbos.vertNormals.Bind();
		glVertexAttribPointer( mtl->attribLocs.normal, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.normal );
	}

	if( mtl->attribLocs.texCoords != -1 )
	{
		mesh->vbos.texCoords.Bind();
		glVertexAttribPointer( mtl->attribLocs.texCoords, 2, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.texCoords );
	}

	if( mtl->attribLocs.tanget != -1 )
	{
		mesh->vbos.vertTangents.Bind();
		glVertexAttribPointer( mtl->attribLocs.tanget, 4, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.tanget );
	}

	mesh->vbos.vertIndeces.Bind();

	glDrawElements( GL_TRIANGLES, mesh->vertIndeces.size(), GL_UNSIGNED_SHORT, 0 );

	// disable
	if( mtl->attribLocs.position != -1 ) glDisableVertexAttribArray( mtl->attribLocs.position );
	if( mtl->attribLocs.normal != -1 ) glDisableVertexAttribArray( mtl->attribLocs.normal );
	if( mtl->attribLocs.texCoords != -1 ) glDisableVertexAttribArray( mtl->attribLocs.texCoords );
	if( mtl->attribLocs.tanget != -1 ) glDisableVertexAttribArray( mtl->attribLocs.tanget );

	if( mesh_skel_ctrl )
	{
		glDisableVertexAttribArray( mtl->attribLocs.vertWeightBonesNum );
		glDisableVertexAttribArray( mtl->attribLocs.vertWeightBoneIds );
		glDisableVertexAttribArray( mtl->attribLocs.vertWeightWeights );
	}

	vbo_t::UnbindAllTargets();
	glPopMatrix();
}
