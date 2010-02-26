#include "MeshNode.h"
#include "Resource.h"
#include "Mesh.h"
#include "renderer.h"
#include "Material.h"
#include "SkelNode.h"
#include "Skeleton.h"
#include "MeshSkelNodeCtrl.h"
#include "SkelAnimCtrl.h"


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void MeshNode::init( const char* filename )
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
// deinit                                                                                                                             =
//=====================================================================================================================================
void MeshNode::deinit()
{
	rsrc::meshes.unload( mesh );
	rsrc::materials.unload( material );
}


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
/// Called in material or blending stages
void MeshNode::render( Material* mtl ) const
{
	glPushMatrix();
	r::multMatrix( transformationWspace );

	// if we have skeleton controller
	if( meshSkelCtrl )
	{
		// first the uniforms
		glUniformMatrix3fv( mtl->uniLocs.skinningRotations, meshSkelCtrl->skelNode->skeleton->bones.size(), 1,
		                    &(meshSkelCtrl->skelNode->skelAnimCtrl->boneRotations[0])[0] );
		glUniform3fv( mtl->uniLocs.skinningTranslations, meshSkelCtrl->skelNode->skeleton->bones.size(),
		              &(meshSkelCtrl->skelNode->skelAnimCtrl->boneTranslations[0])[0] );

		// then the attributes
		DEBUG_ERR( !mtl->hasHWSkinning() ); // it has skel controller but no skinning

		mesh->vbos.vertWeights.bind();
		glEnableVertexAttribArray( mtl->attribLocs.vertWeightBonesNum );
		glVertexAttribPointer( mtl->attribLocs.vertWeightBonesNum, 1, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(0) );
		glEnableVertexAttribArray( mtl->attribLocs.vertWeightBoneIds );
		glVertexAttribPointer( mtl->attribLocs.vertWeightBoneIds, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(4) );
		glEnableVertexAttribArray( mtl->attribLocs.vertWeightWeights );
		glVertexAttribPointer( mtl->attribLocs.vertWeightWeights, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexWeight), BUFFER_OFFSET(20) );
	}

	if( mtl->attribLocs.position != -1 )
	{
		mesh->vbos.vertCoords.bind();
		glVertexAttribPointer( mtl->attribLocs.position, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.position );
	}

	if( mtl->attribLocs.normal != -1 )
	{
		mesh->vbos.vertNormals.bind();
		glVertexAttribPointer( mtl->attribLocs.normal, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.normal );
	}

	if( mtl->attribLocs.texCoords != -1 )
	{
		mesh->vbos.texCoords.bind();
		glVertexAttribPointer( mtl->attribLocs.texCoords, 2, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.texCoords );
	}

	if( mtl->attribLocs.tanget != -1 )
	{
		mesh->vbos.vertTangents.bind();
		glVertexAttribPointer( mtl->attribLocs.tanget, 4, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attribLocs.tanget );
	}

	mesh->vbos.vertIndeces.bind();

	glDrawElements( GL_TRIANGLES, mesh->vertIndeces.size(), GL_UNSIGNED_SHORT, 0 );

	// disable
	if( mtl->attribLocs.position != -1 ) glDisableVertexAttribArray( mtl->attribLocs.position );
	if( mtl->attribLocs.normal != -1 ) glDisableVertexAttribArray( mtl->attribLocs.normal );
	if( mtl->attribLocs.texCoords != -1 ) glDisableVertexAttribArray( mtl->attribLocs.texCoords );
	if( mtl->attribLocs.tanget != -1 ) glDisableVertexAttribArray( mtl->attribLocs.tanget );

	if( meshSkelCtrl )
	{
		glDisableVertexAttribArray( mtl->attribLocs.vertWeightBonesNum );
		glDisableVertexAttribArray( mtl->attribLocs.vertWeightBoneIds );
		glDisableVertexAttribArray( mtl->attribLocs.vertWeightWeights );
	}

	Vbo::unbindAllTargets();
	glPopMatrix();
}
