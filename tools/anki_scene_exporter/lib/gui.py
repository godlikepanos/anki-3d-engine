# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

# blender imports
import os

import bpy
import bpy.utils.previews
from bpy.props import StringProperty

bl_info = {"author": "A. A. Kalugin Jr."}

def get_region3d(context):
	"""
	Gets the first region 3d viewport.
	"""
	view = get_view_3d(context)
	if view:
		return view.spaces[0].region_3d
	return None

def get_view_3d(context):
	"""
	Gets the first region 3d viewport.
	"""
	for view in context.window.screen.areas:
		if view.type == 'VIEW_3D':
			return view
	return None

def image_filter_glob():
	return StringProperty(
						   default="*.bmp;*.sgi;*.rgb;*.bw;*.png;*.jpg;*.jpeg;*.exr;*.hdr;*.jp2;*.j2c;*.tga",
						   options={'HIDDEN'}
						 )

def image_file_path():
	return StringProperty(
						  name="File Path",
						  description="File path use",
						  maxlen= 1024, default= ""
						 )
