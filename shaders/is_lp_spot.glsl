#define _SPOT_LIGHT_

#pragma anki uniform ms_normal_fai 0
#pragma anki uniform ms_diffuse_fai 1
#pragma anki uniform ms_specular_fai 2
#pragma anki uniform ms_depth_fai 3
#pragma anki uniform planes 4
#pragma anki uniform light_pos 5
#pragma anki uniform light_inv_radius 6
#pragma anki uniform light_diffuse_col 7
#pragma anki uniform light_specular_col 8
#pragma anki uniform light_tex 9
#pragma anki uniform tex_projection_mat 10

#pragma anki include "shaders/is_lp_generic.glsl"
