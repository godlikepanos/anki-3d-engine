# Blender imports
import bpy
from bpy.types import Operator
from bpy.props import StringProperty
from bpy_extras.io_utils import ImportHelper

# Local imports
from ..lib import gui as GUI
from ..lib import material as MAT
from ..lib import mesh as MESH


class ANKI_MATERIAL_clear_slot(Operator):
	"""Clears a given material stol"""
	bl_idname	= "anki.clear_texture_slot"
	bl_label	= "Clear Slot by ID"
	bl_options	= {'REGISTER', 'UNDO'}
	port_type 	= bpy.props.StringProperty(default = "diffuse")

	def execute(self, context):
		mat = context.active_object.active_material
		MAT.clear_texture_slot(mat, self.port_type)
		return {'FINISHED'}

class ANKI_MATERIAL_apply_image(Operator, ImportHelper):
	bl_idname 		= "anki.apply_texture_image"
	bl_label 		= "Apply Material Image"
	bl_options		= {'REGISTER', 'UNDO'}
	# filename_ext 	= ".jpg"
	filter_glob 	= GUI.image_filter_glob()
	filepath 		= GUI.image_file_path()
	mat_port_type 	= bpy.props.StringProperty(default ="diffuse")

	@classmethod
	def poll(cls, context):
		material = context.active_object.active_material
		return MAT.check_material(material)

	def execute(self, context):
		material = context.active_object.active_material
		img = MAT.get_image_node( self.filepath)
		texture_node = MAT.get_texture_node(self.filepath)
		texture_node.image = img

		texture_slot = MAT.get_texture_slot( material, self.mat_port_type)

		if self.mat_port_type == "diffuse":
			context.active_object.active_material.ANKI.diff_path = self.filepath
			MAT.set_diffuse_texture_attr(texture_slot, texture_node)

		if self.mat_port_type == "roughness":
			context.active_object.active_material.ANKI.spec_path = self.filepath
			texture_slot.texture = texture_node
			# texture_slot.use_map_specular = True
			# texture_slot.use_map_specular = 1.0

			texture_slot.use_map_color_diffuse = False
			texture_slot.use_map_hardness      = True
			# texture_slot.use_map_hardness      = 1.0

		MESH.set_textured(context.active_object, draw_texture = True)
		MAT.set_uv_texture_attr(texture_slot)

		return {'FINISHED'}

	def invoke(self, context, event):
		context.window_manager.fileselect_add(self)

		return {'RUNNING_MODAL'}
