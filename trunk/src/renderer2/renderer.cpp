/*
#include "renderer.hpp"


float renderer_t::quad_vert_cords [][2] = { {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0} };


//=====================================================================================================================================
// UpdateMatrices                                                                                                                     =
//=====================================================================================================================================
void renderer_t::UpdateMatrices()
{
	const Mat4& M = matrices.model;
	const Mat4& V = matrices.view;
	const Mat4& P = matrices.projection;
	Mat4& MV = matrices.model_view;
	Mat4& MVP = matrices.model_view_projection;
	Mat3& N = matrices.normal;

	Mat4& Mi = matrices.model_inv;
	Mat4& Vi = matrices.view_inv;
	Mat4& Pi = matrices.projection_inv;
	Mat4& MVi = matrices.model_view_inv;
	Mat4& MVPi = matrices.model_view_projection_inv;
	Mat3& Ni = matrices.normal_inv;

	// matrices
	MV = V * M;
	MVP = P * MV;
	N = MV.getRotationPart();

	// inv matrices
	Mi = M.getInverseTransformation();
	Vi = V.getInverseTransformation();
	Pi = P.getInverse();
	MV = Mi * Vi;
	MVPi = MVi * Pi;
	Ni = MVi.getRotationPart();
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
