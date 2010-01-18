#include "mesh_node.h"
#include "resource.h"
#include "mesh.h"
#include "renderer.h"
#include "material.h"
#include "skel_node.h"
#include "skeleton.h"
#include "skel_controller.h"
#include "skel_anim_controller.h"


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void mesh_node_t::Init( const char* filename )
{
	mesh = rsrc::meshes.Load( filename );
	material = rsrc::materials.Load( mesh->material_name.c_str() );
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
void mesh_node_t::Render()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	// if we have skeleton controller
	if( skel_controller )
	{
		// first the uniforms
		glUniformMatrix3fv( material->uni_locs.skinning_rotations, skel_controller->skel_node->skeleton->bones.size(), 1,
		                    &(skel_controller->skel_node->skel_anim_controller->bone_rotations[0])[0] );
		glUniform3fv( material->uni_locs.skinning_translations, skel_controller->skel_node->skeleton->bones.size(),
		              &(skel_controller->skel_node->skel_anim_controller->bone_translations[0])[0] );

		// then the attributes
		DEBUG_ERR( material->attrib_locs.vert_weight_bones_num == -1 );

		mesh->vbos.vert_weights.Bind();
		glEnableVertexAttribArray( material->attrib_locs.vert_weight_bones_num );
		glVertexAttribPointer( material->attrib_locs.vert_weight_bones_num, 1, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(0) );
		glEnableVertexAttribArray( material->attrib_locs.vert_weight_bone_ids );
		glVertexAttribPointer( material->attrib_locs.vert_weight_bone_ids, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(4) );
		glEnableVertexAttribArray( material->attrib_locs.vert_weight_weights );
		glVertexAttribPointer( material->attrib_locs.vert_weight_weights, 4, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(20) );
	}

	if( material->attrib_locs.position != -1 )
	{
		mesh->vbos.vert_coords.Bind();
		glVertexAttribPointer( material->attrib_locs.position, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attrib_locs.position );
	}

	if( material->attrib_locs.normal != -1 )
	{
		mesh->vbos.vert_normals.Bind();
		glVertexAttribPointer( material->attrib_locs.normal, 3, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attrib_locs.normal );
	}

	if( material->attrib_locs.tex_coords != -1 )
	{
		mesh->vbos.tex_coords.Bind();
		glVertexAttribPointer( material->attrib_locs.tex_coords, 2, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attrib_locs.tex_coords );
	}

	if( material->attrib_locs.tanget != -1 )
	{
		mesh->vbos.vert_tangents.Bind();
		glVertexAttribPointer( material->attrib_locs.tanget, 4, GL_FLOAT, false, 0, NULL );
		glEnableVertexAttribArray( material->attrib_locs.tanget );
	}

	mesh->vbos.vert_indeces.Bind();

	glDrawElements( GL_TRIANGLES, mesh->vert_indeces.size(), GL_UNSIGNED_SHORT, 0 );

	// disable
	if( material->attrib_locs.position != -1 ) glDisableVertexAttribArray( material->attrib_locs.position );
	if( material->attrib_locs.normal != -1 ) glDisableVertexAttribArray( material->attrib_locs.normal );
	if( material->attrib_locs.tex_coords != -1 ) glDisableVertexAttribArray( material->attrib_locs.tex_coords );
	if( material->attrib_locs.tanget != -1 ) glDisableVertexAttribArray( material->attrib_locs.tanget );

	if( skel_controller )
	{
		glDisableVertexAttribArray( material->attrib_locs.vert_weight_bones_num );
		glDisableVertexAttribArray( material->attrib_locs.vert_weight_bone_ids );
		glDisableVertexAttribArray( material->attrib_locs.vert_weight_weights );
	}

	vbo_t::UnbindAllTargets();
	glPopMatrix();
}


//=====================================================================================================================================
// RenderDepth                                                                                                                        =
//=====================================================================================================================================
void mesh_node_t::RenderDepth()
{
	glPushMatrix();
	r::MultMatrix( transformation_wspace );

	mesh->vbos.vert_coords.Bind();
	glVertexPointer( 3, GL_FLOAT, 0, NULL );

	if( material->grass_map )
	{
		mesh->vbos.tex_coords.Bind();
		glTexCoordPointer( 2, GL_FLOAT, 0, NULL );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}

	// if we have skeleton controller
	if( skel_controller )
	{
		// first the uniforms
		glUniformMatrix3fv( material->uni_locs.skinning_rotations, skel_controller->skel_node->skeleton->bones.size(), 1,
		                    &(skel_controller->skel_node->skel_anim_controller->bone_rotations[0])[0] );
		glUniform3fv( material->uni_locs.skinning_translations, skel_controller->skel_node->skeleton->bones.size(),
		              &(skel_controller->skel_node->skel_anim_controller->bone_translations[0])[0] );

		// then the attributes
		DEBUG_ERR( material->attrib_locs.vert_weight_bones_num == -1 );

		mesh->vbos.vert_weights.Bind();
		glEnableVertexAttribArray( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(0) );
		glVertexAttribPointer( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(0), 1, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(0) );
		glEnableVertexAttribArray(r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(1) );
		glVertexAttribPointer( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(1), 4, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(4) );
		glEnableVertexAttribArray( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(2) );
		glVertexAttribPointer( r::is::shadows::shdr_depth_hw_skinning->GetAttributeLocation(2), 4, GL_FLOAT, GL_FALSE, sizeof(mesh_t::vertex_weight_t), BUFFER_OFFSET(20) );
	}

	mesh->vbos.vert_indeces.Bind();

	glEnableClientState( GL_VERTEX_ARRAY );

	glDrawElements( GL_TRIANGLES, mesh->vert_indeces.size(), GL_UNSIGNED_SHORT, 0 );

	// disable
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	if( skel_controller )
	{
		glDisableVertexAttribArray( material->attrib_locs.vert_weight_bones_num );
		glDisableVertexAttribArray( material->attrib_locs.vert_weight_bone_ids );
		glDisableVertexAttribArray( material->attrib_locs.vert_weight_weights );
	}

	vbo_t::UnbindAllTargets();

	glPopMatrix();
}
