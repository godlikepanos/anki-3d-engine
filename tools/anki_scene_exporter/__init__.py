# Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

# keep methods in alphabetical order

bl_info = {
	"name": "Anki Scene Exporter",
	"author": "A. A. Kalugin Jr.",
	"version": (0, 1),
	"blender": (2, 65, 0),
	"location": "Anki Preferences",
	"description": "Anki Scene Exporter",
	"warning": "",
	"wiki_url": "",
	"tracker_url": "",
	"category": "User"}
# set up the

if "bpy" in locals():
	import importlib

	importlib.reload(ops.environment)
	importlib.reload(ops.export)
	importlib.reload(props.export_props)
	importlib.reload(ui.gui)

else:
	from .ops import environment
	from .ops import export
	from .props import export_props
	from .ui import gui

import bpy, os
from bpy.types import AddonPreferences
from bpy.props import StringProperty
from .lib import environment as ENV


################################ CONSTANTS ###############################
ENV.ENVIRO = "ANKI_DATA_PATH"
################################ CONSTANTS ###############################

class SCENE_anki_scene_exporter(AddonPreferences):
	# this must match the addon name, use '__package__'
	# when defining this in a submodule of a python package.
	bl_idname = __name__


	anki_environment = StringProperty(
			name="Anki Environment",
			default=ENV.ENVIRO,
			)
	anki_project_path = StringProperty(
		name="Project Path",
		subtype='FILE_PATH',
		)
	export_path = StringProperty(
			name="Source Path",
			default="Source Path:",
			)
	im_convert = StringProperty(
			name="ImageMagick Convert Path",
			default="/usr/bin/convert",
			subtype='FILE_PATH',
			)
	tool_etcpack_path = StringProperty(
			name="Third Party etc pack",
			default="",
			subtype='FILE_PATH',
			)
	tools_path = StringProperty(
			name="Tools Path",
			default="Tools Path:",
			)
	map_path = StringProperty(
			name="Map Path",
			default="Map Path:",
			)
	texture_path = StringProperty(
			name="Texture Path",
			default="Texture Path:{}",
			)
	temp_dea = StringProperty(
			name="Temp Dea Path",
			)
	# Try to get/set the environment variables
	path_list = os.getenv(ENV.ENVIRO)
	environment_root = None
	if path_list:
		# Environment variable exists get the common path.
		environment_root = ENV.get_common_environment_path()
		anki_project_path[1]['default'] = environment_root

		env_dct, export_dct, tools_dct = ENV.get_environment()
		# ENV.set_environment(env_dct, tools_dct)

		# check converter if does not exit remove default path
		if not os.path.exists(im_convert[1]['default']):
			im_convert[1]['default'] = ""

		#check etcpack tool and fill the path
		tool_etcpack = tools_dct['tool_etcpack_path']
		if os.path.exists(tool_etcpack):
			tool_etcpack_path[1]['default'] = tool_etcpack

		# Set the gui
		tools_path[1]['default'] = tools_dct['tools_path']
		export_path[1]['default'] = export_dct['export_src_data']
		map_path[1]['default'] = export_dct['export_map_path']
		texture_path[1]['default'] = export_dct['export_texture_path']
		temp_dea[1]['default'] = export_dct['export_temp_dea']

	def draw(self, context):
		layout = self.layout
		msg = "Preferences are driven by {} environment variable".format(ENV.ENVIRO)
		layout.label(text=msg)
		layout.prop(self, "anki_project_path")
		layout.prop(self, "im_convert")
		layout.prop(self, "tool_etcpack_path")

		split = layout.split()
		col = split.column()
		col.label(text='Source Path:')
		col.label(text='Map Path:')
		col.label(text='Texture Path:')
		col.label(text='Tools Path:')

		col = split.column()
		col.label(text=self.export_path)
		col.label(text=self.map_path)
		col.label(text=self.texture_path)
		col.label(text=self.tools_path)

		layout.operator("scene.anki_preference_set", text="Set Preferences")


def register():
	bpy.utils.register_module(__name__)


def unregister():
	bpy.utils.unregister_module(__name__)


if __name__ == "__main__":
	register()

