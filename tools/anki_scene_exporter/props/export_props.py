# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

# blender imports
import bpy
from bpy.props import BoolProperty

bl_info = {"author": "A. A. Kalugin Jr."}

bpy.types.Scene.UseViewport = BoolProperty(
	name = "Use Viewport",
	description = "True or False?",
	default= True)

bpy.types.Scene.ExportAllTextures = BoolProperty(
	name = "Export All Textures",
	description = "True or False?",
	default= False)

bpy.types.Scene.UpdateTextures = BoolProperty(
	name = "Update Textures",
	description = "True or False?",
	default= True)