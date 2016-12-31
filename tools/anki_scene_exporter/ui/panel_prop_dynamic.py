# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

import bpy
from bpy.types import Panel, Operator
from bpy.props import StringProperty
from . import panel_layouts as PANEL


def _callback(scene, prop):
	print("Property '%s' of %s updated to value %s" % (prop, scene.name, getattr(scene, prop)))

def test(prop):
	def update(self, context):
		_callback(self, prop)
	return update


class ANKI_dynamic_prop_panel(Panel):
	bl_label = " "
	bl_idname = "ANKI_dynamic_prop_panel"
	bl_space_type = "VIEW_3D"
	bl_region_type = "UI"

	panel = "ANKI_PP_home"

	def draw_header(self, context):
		layout = self.layout
		layout.operator("anki.view3d_dynamic_prop_panel", icon='RADIO').panel="ANKI_PP_home"

	def draw(self, context):
		layout = self.layout
		dyn_panel = "PANEL.%s(layout, context)" % self.panel
		exec (dyn_panel)

class ANKI_VIEW3D_dynamic_prop_panel_op(Operator):
	"""ANKI dynamic props panel exec"""
	bl_idname = "anki.view3d_dynamic_prop_panel"
	bl_label = ""
	bl_options = {'REGISTER'}

	panel = StringProperty(
							name="panel",
							default="",
							description="string of a method panel"
							)

	def execute(self, context):
		dyn = bpy.types.ANKI_dynamic_prop_panel
		bpy.utils.unregister_class(dyn)
		dyn.panel = self.panel
		bpy.utils.register_class(dyn)

		return {'FINISHED'}

