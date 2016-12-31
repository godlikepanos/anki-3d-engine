# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

# system imports
import os

# blender imports
import bpy, os
from bpy.types import Operator,  Panel, AddonPreferences
from bpy.props import StringProperty

# local imports
from ..lib import environment as ENV

bl_info = {"author": "A. A. Kalugin Jr."}

class AnkiPanel:
	bl_space_type = 'VIEW_3D'
	bl_region_type = 'TOOLS'

class VIEW3D_PT_prefs_anki(AnkiPanel, Panel):
	bl_category = "Anki"
	bl_label = "Export Preferences"

	def draw(self, context):
		scn = context.scene
		layout = self.layout
		col = layout.column(align=True)
		col.label(text="Texture Export:")
		row = col.row(align=True)
		row.prop(scn, "ExportAllTextures")
		row.prop(scn, "UpdateTextures")

class VIEW3D_PT_tools_anki(AnkiPanel, Panel):
	bl_category = "Anki"
	bl_label = "Exporter"

	def draw(self, context):
		scn = context.scene
		layout = self.layout

		col = layout.column(align=True)

		split = layout.split()
		col = split.column()
		col.prop(scn, "UseViewport", text="Viewport")
		col = split.column()
		col.operator("scene.anki_export_scene", icon='PLAY')
		col = layout.column(align=True)

		col.separator()

		col.label(text="Texture Export:")
		row = col.row(align=True)
		row.operator("scene.anki_export_textures", icon='IMAGE_COL')
