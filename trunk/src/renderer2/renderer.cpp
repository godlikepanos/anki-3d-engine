/*
#include "renderer.hpp"


float renderer_t::quad_vert_cords [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };


//=====================================================================================================================================
// UpdateMatrices                                                                                                                     =
//=====================================================================================================================================
void renderer_t::UpdateMatrices()
{
	const mat4_t& M = matrices.model;
	const mat4_t& V = matrices.view;
	const mat4_t& P = matrices.projection;
	mat4_t& MV = matrices.model_view;
	mat4_t& MVP = matrices.model_view_projection;
	mat3_t& N = matrices.normal;

	mat4_t& Mi = matrices.model_inv;
	mat4_t& Vi = matrices.view_inv;
	mat4_t& Pi = matrices.projection_inv;
	mat4_t& MVi = matrices.model_view_inv;
	mat4_t& MVPi = matrices.model_view_projection_inv;
	mat3_t& Ni = matrices.normal_inv;

	// matrices
	MV = V * M;
	MVP = P * MV;
	N = MV.GetRotationPart();

	// inv matrices
	Mi = M.GetInverseTransformation();
	Vi = V.GetInverseTransformation();
	Pi = P.GetInverse();
	MV = Mi * Vi;
	MVPi = MVi * Pi;
	Ni = MVi.GetRotationPart();
}


//=====================================================================================================================================
// DrawQuad                                                                                                                           =
//=====================================================================================================================================
void renderer_t::DrawQuad()
{
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, quad_vert_cords );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableClientState( GL_VERTEX_ARRAY );
}
*/
