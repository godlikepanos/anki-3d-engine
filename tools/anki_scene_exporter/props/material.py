# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order


import bpy
from bpy.types import PropertyGroup
from bpy.props import FloatVectorProperty, BoolProperty, StringProperty

class ANKI_Properties(PropertyGroup):
	shader_stand 		= BoolProperty(default=False)
	indirect_illum 		= BoolProperty(default=False)
	shader_rendering	= BoolProperty(default=False)
	shader_shadow		= BoolProperty(default=False)
	shader_sss 			= BoolProperty(default=False)
	shader_transparency	= BoolProperty(default=False)
	shader_reflection	= BoolProperty(default=False)
	shader_strand		= BoolProperty(default=False)

	diff_shader 		= BoolProperty(default=False)
	diff_image 			= BoolProperty(default=False)
	diff_path			= StringProperty()
	diff_uvs			= BoolProperty(default=False)
	diff_sampling		= BoolProperty(default=False)
	diff_influence 		= BoolProperty(default=False)
	diff_ramp 			= BoolProperty(default=False)
	diff_img_display	= BoolProperty(default=False)

	spec_shader 		= BoolProperty(default=False)
	spec_image 			= BoolProperty(default=False)
	spec_path			= StringProperty()
	spec_uvs			= BoolProperty(default=False)
	spec_sampling		= BoolProperty(default=False)
	spec_influence 		= BoolProperty(default=False)
	spec_ramp 			= BoolProperty(default=False)
	spec_img_display	= BoolProperty(default=False)

	tran_shader 		= BoolProperty(default=False)
	tran_image 			= BoolProperty(default=False)
	tran_path			= StringProperty()
	tran_uvs			= BoolProperty(default=False)
	tran_influence 		= BoolProperty(default=False)

	norm_shader 		= BoolProperty(default=False)
	norm_image 			= BoolProperty(default=False)
	norm_path			= StringProperty()
	norm_uvs			= BoolProperty(default=False)
	norm_influence 		= BoolProperty(default=False)

	refl_shader 		= BoolProperty(default=False)
	refl_image 			= BoolProperty(default=False)
	refl_path			= StringProperty()
	refl_uvs			= BoolProperty(default=False)
	refl_influence 		= BoolProperty(default=False)