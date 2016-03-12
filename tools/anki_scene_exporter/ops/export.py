# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE Panagiotis Christopoulos Charitos and contributors
# keep methods in alphabetical order

# system imports
import subprocess
import os

# blender imports
import bpy
from bpy.types import Operator
import mathutils

# local imports
from ..lib import gui as GUI
from ..lib import preferences as PREF
from ..lib import objects as OBJ

bl_info = {"author": "A. A. Kalugin Jr."}

class OBJECT_OT_anki_export(Operator):
		"""Export anki scene"""
		bl_idname = "scene.anki_export"
		bl_label = "Anki Export"
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
													include_material_textures=False,
													use_texture_copies=True,
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

				# Convert the exported collada
				relative_path = (addon_prefs.map_path.replace(addon_prefs.anki_project_path + os.sep, ""))
				cmd  = "ankisceneimp {0} {1} -rpath {2}  -flipyz".format(temp_dea, addon_prefs.map_path, relative_path)
				subprocess.call(cmd, shell=True)
				print (cmd)
				# Run the level in the game
				test_app = ("{0}/build/bled/bled".format(addon_prefs.anki_project_path))

				subprocess.call(test_app, shell=True)

				return {'FINISHED'}
