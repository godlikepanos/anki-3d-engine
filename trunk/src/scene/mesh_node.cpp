#include "mesh_node.h"
#include "resource.h"
#include "mesh.h"
#include "renderer.h"
#include "material.h"
#include "skel_node.h"
#include "skeleton.h"
#include "mesh_skel_ctrl.h"
#include "skel_anim_ctrl.h"


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void mesh_node_t::Init( const char* filename )
{
	mesh = rsrc::meshes.Load( filename );
	material = rsrc::materials.Load( mesh->material_name.c_str() );

	// sanity checks
	if( material->attrib_locs.tex_coords != -1 && mesh->vbos.tex_coords.GetGLID() == 0 )
	{
		ERROR( "The shader program needs information that the mesh do not have" );
	}
}


//=====================================================================================================================================
// Deinit                                                                                                                             =
//=====================================================================================================================================
void mesh_node_t::Deinit()
{
	rsrc::meshes.Unload( mesh );
	rsrc::materials.Unload( material );
}


//=====================================================================================================================================
// Render                                                                                                                             =
//=====================================================================================================================================
/// Called in material or blending stages
void mesh_node_t::Render( material_t* mtl ) const
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	// if we have skeleton controller
	if( mesh_skel_ctrl )
	{
		// first the uniforms
		glUniformMatrix3fv( mtl->uni_locs.skinning_rotations, mesh_skel_ctrl->skel_node->skeleton->bones.size(), 1,
		                    &(mesh_skel_ctrl->skel_node->skel_anim_ctrl->bone_rotations[0])[0] );
		glUniform3fv( mtl->uni_locs.skinning_translations, mesh_skel_ctrl->skel_node->skeleton->bones.size(),
		              &(mesh_skel_ctrl->skel_node->skel_anim_ctrl->bone_translations[0])[0] );

		// then the attributes
		DEBUG_ERR( !mtl->HasHWSkinning() );

		mesh->vbos.vert_weights.Bind();
		glEnableVertexAttribArray( mtl->attrib_locs.vert_weight_bones_num );
		glVertexAttribPointer( mtl->attrib_locs.vert_weight_bones_num, 1, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(0) );
		glEnableVertexAttribArray( mtl->attrib_locs.vert_weight_bone_ids );
		glVertexAttribPointer( mtl->attrib_locs.vert_weight_bone_ids, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(4) );
		glEnableVertexAttribArray( mtl->attrib_locs.vert_weight_weights );
		glVertexAttribPointer( mtl->attrib_locs.vert_weight_weights, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(20) );
	}

	if( mtl->attrib_locs.position != -1 )
	{
		mesh->vbos.vert_coords.Bind();
		glVertexAttribPointer( mtl->attrib_locs.position, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attrib_locs.position );
	}

	if( mtl->attrib_locs.normal != -1 )
	{
		mesh->vbos.vert_normals.Bind();
		glVertexAttribPointer( mtl->attrib_locs.normal, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attrib_locs.normal );
	}

	if( mtl->attrib_locs.tex_coords != -1 )
	{
		mesh->vbos.tex_coords.Bind();
		glVertexAttribPointer( mtl->attrib_locs.tex_coords, 2, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attrib_locs.tex_coords );
	}

	if( mtl->attrib_locs.tanget != -1 )
	{
		mesh->vbos.vert_tangents.Bind();
		glVertexAttribPointer( mtl->attrib_locs.tanget, 4, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( mtl->attrib_locs.tanget );
	}

	mesh->vbos.vert_indeces.Bind();

	glDrawElements( GL_TRIANGLES, mesh->vert_indeces.size(), GL_UNSIGNED_SHORT, 0 );

	// disable
	if( mtl->attrib_locs.position != -1 ) glDisableVertexAttribArray( mtl->attrib_locs.position );
	if( mtl->attrib_locs.normal != -1 ) glDisableVertexAttribArray( mtl->attrib_locs.normal );
	if( mtl->attrib_locs.tex_coords != -1 ) glDisableVertexAttribArray( mtl->attrib_locs.tex_coords );
	if( mtl->attrib_locs.tanget != -1 ) glDisableVertexAttribArray( mtl->attrib_locs.tanget );

	if( mesh_skel_ctrl )
	{
		glDisableVertexAttribArray( mtl->attrib_locs.vert_weight_bones_num );
		glDisableVertexAttribArray( mtl->attrib_locs.vert_weight_bone_ids );
		glDisableVertexAttribArray( mtl->attrib_locs.vert_weight_weights );
	}

	vbo_t::UnbindAllTargets();
	glPopMatrix();
}
