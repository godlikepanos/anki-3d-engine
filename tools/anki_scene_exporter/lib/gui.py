# Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

# keep methods in alphabetical order

# blender imports
import bpy

bl_info = {"author": "A. A. Kalugin Jr."}

def get_region3d(context):
	"""
	Gets the first region 3d viewport.
	"""
	for view in context.window.screen.areas:
		if view.type == 'VIEW_3D':
			return view.spaces[0].region_3d
	return None
