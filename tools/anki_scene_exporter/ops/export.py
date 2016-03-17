# Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

# system imports
import os
import subprocess
from subprocess import Popen
# blender imports
import bpy
from bpy.types import Operator
import mathutils

# local imports
from ..lib import gui as GUI
from ..lib import objects as OBJ
from ..lib import material as MAT
from ..lib import math as MTH
from ..lib import preferences as PREF

bl_info = {"author": "A. A. Kalugin Jr."}

class OBJECT_OT_anki_export_material(Operator):
		"""Export anki scene textures"""
		bl_idname = "scene.anki_export_textures"
		bl_label = "Export Textures"
		bl_options = {'REGISTER'}
		def execute(self, context):
			scn                = context.scene
			exported_log       = []
			error_log          = []
			commands           = []
			addon_prefs        = PREF.get_anki_exporter_preferences(context)
			py_image_converter = addon_prefs.tool_py_texture
			export_path        = addon_prefs.export_texture_path
			textures           = MAT.get_texture_images_nodes()

			# Process the blender texture slot nodes.
			for texture in textures:
				src_path = texture.filepath
				src_file_name = os.path.basename(src_path)

				# Check if the resolution of the image is power of twos
				res_x = (texture.size[0])
				res_y = (texture.size[1])
				if not MTH.is_power_of_two(res_x) and not MTH.is_power_of_two(res_y):
					msg = "Image {0} resolution not power of 2. x {1} y {2}"
					error = msg.format(texture.filepath, res_x, res_y)
					error_log.append(error)
					continue

				# Set up the command for texture export.
				dst_path = ("{0}/{1}.ankitex".format(export_path, os.path.splitext(src_file_name)[0] ))
				cmd = "{0} -i {1} -o {2}".format(py_image_converter, texture.filepath, dst_path)
				Popen(cmd, shell=True)
				src_time = os.path.getatime(src_path)
				dst_time = os.path.getatime(dst_path)
				if src_time > dst_time:
					export = "Running command: {0}".format(cmd)
					exported_log.append(export)
					commands.append(cmd)

			# Process the commands
			if len(commands) > 0:
				for cmd in commands:
					print ("Running command: {0}".format(cmd))
					Popen(cmd, shell=True)
			else:
				print("No commands to to process.")

			return {'FINISHED'}

class OBJECT_OT_anki_export_scene(Operator):
		"""Export anki scene"""
		bl_idname = "scene.anki_export_scene"
		bl_label = "Export Scene"
		bl_options = {'REGISTER'}

		def execute(self, context):
				scn = context.scene

				addon_prefs = PREF.get_anki_exporter_preferences(context)

				temp_dea =  addon_prefs.temp_dea

				view_mtx = None
				cam = None
				if scn.UseViewport:
						# Get camera matrix to viewport projection matrix
						cam = OBJ.get_camera()
						view_mtx = mathutils.Matrix(cam.matrix_world)
						r3d = GUI.get_region3d(context)
						cam.matrix_world = r3d.view_matrix.inverted()
						print ("Set camera to view projection matrix.")

				# Export the collada file to temp.dea
				bpy.ops.wm.collada_export(filepath=temp_dea,
													check_existing=True,
													filter_blender=False,
													filter_backup=False,
													filter_image=False,
													filter_movie=False,
													filter_python=False,
													filter_font=False,
													filter_sound=False,
													filter_text=False,
													filter_btx=False,
													filter_collada=True,
													filter_folder=True,
													filter_blenlib=False,
													filemode=8,
													display_type='DEFAULT',
													sort_method='FILE_SORT_ALPHA',
													apply_modifiers=False,
													export_mesh_type=0,
													export_mesh_type_selection='view',
													selected=False,
													include_children=False,
													include_armatures=False,
													include_shapekeys=True,
													deform_bones_only=False,
													active_uv_only=False,
													include_uv_textures=False,
													include_material_textures=True,
													use_texture_copies=False,
													triangulate=True,
													use_object_instantiation=True,
													sort_by_name=False,
													export_transformation_type=0,
													export_transformation_type_selection='matrix',
													open_sim=False)
				print ("Exported to:", temp_dea)

				# Restore Camera
				if scn.UseViewport:
						cam.matrix_world = view_mtx
						print ("Restored camera to original projection matrix.")

				# Format the command and convert the exported collada
				rel_map_path = (addon_prefs.export_map_path.replace(addon_prefs.anki_project_path, ""))
				rel_texture_path = (addon_prefs.export_texture_path.replace(addon_prefs.anki_project_path, ""))
				cmd_str = "ankisceneimp {0} {1} -rpath {2} -texrpath {3} -flipyz"
				cmd  = cmd_str.format(temp_dea,
									  addon_prefs.export_map_path,
									  rel_map_path,
									  rel_texture_path)

				subprocess.call(cmd, shell=True)

				# Run the level in the game
				bled_app = ("{0}/build/bled/bled".format(addon_prefs.anki_project_path))

				subprocess.call(bled_app, shell=True)

				return {'FINISHED'}

