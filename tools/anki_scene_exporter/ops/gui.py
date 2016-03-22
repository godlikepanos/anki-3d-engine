import bpy
from bpy.types import Operator

class BSI_PP_home_op(Operator):
	"""BL Mesh Edit"""
	bl_idname = "bsi.pp_home_op"
	bl_label = "BSI Home Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_prop_panel(panel = "BSI_PP_home")
		return {'FINISHED'}

class BSI_PP_motiontracking(Operator):
	"""BSI Motiontracking Panel Op"""
	bl_idname = "bsi.panel_motiontracking"
	bl_label = "Motiontracking Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_prop_panel(panel = "VIEW3D_PP_motiontracking")
		return {'FINISHED'}

class BSI_PP_viewproperties(Operator):
	"""BSI View Properties Panel"""
	bl_idname = "bsi.panel_viewproperties"
	bl_label = "View Properties Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_prop_panel(panel = "VIEW3D_PP_viewproperties")
		return {'FINISHED'}

class VIEW3D_PP_meshproperties(Operator):
	"""BSI Mesh Properties Panel"""
	bl_idname = "bsi.panel_meshproperties"
	bl_label = "Mesh Properties Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_prop_panel(panel = "VIEW3D_PP_meshproperties")
		return {'FINISHED'}

class BSI_TP_meshedit_op(Operator):
	"""BL Mesh Edit"""
	bl_idname = "bsi.panel_meshedit"
	bl_label = "Mesh Edit Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_tool_panel(panel = "VIEW3D_TP_meshedit")
		return {'FINISHED'}

class BSI_TP_greasepencil_op(Operator):
	"""BL Gease Pencil Panel"""
	bl_idname = "bsi.panel_greasepencil"
	bl_label = "Grease Pencil Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_tool_panel(panel = "VIEW3D_TP_greasepencil")
		return {'FINISHED'}

class BSI_TP_rigidbody_op(Operator):
	"""BL Rigid Body Panel"""
	bl_idname = "bsi.panel_rigidbody"
	bl_label = "Rigid Body Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_tool_panel(panel = "VIEW3D_TP_rigidbody")
		return {'FINISHED'}

class BSI_TP_animation_op(Operator):
	"""BL Animation Panel"""
	bl_idname = "bsi.panel_animation"
	bl_label = "Animation Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_tool_panel(panel = "VIEW3D_TP_animation")
		return {'FINISHED'}

class BSI_TP_relation_op(Operator):
	"""BL Relation Panel"""
	bl_idname = "bsi.panel_relation"
	bl_label = "Relation Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_tool_panel(panel = "VIEW3D_TP_relation")
		return {'FINISHED'}

class VIEW3D_TP_addobject(Operator):
	"""BL Add Object Panel"""
	bl_idname = "bsi.panel_addobject"
	bl_label = "Add Object Panel"
	bl_options = {'REGISTER'}

	def execute(self, context):
		bpy.ops.bsi.view3d_dynamic_tool_panel(panel = "VIEW3D_TP_addobject")
		return {'FINISHED'}

