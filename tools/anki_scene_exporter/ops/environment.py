# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE Panagiotis Christopoulos Charitos and contributors
# keep methods in alphabetical order

# blender imports
import bpy
import mathutils
from bpy.types import Operator
from bpy.props import StringProperty

# local imports
from ..lib import preferences as PREF
from ..lib import environment as ENV

bl_info = {"author": "A. A. Kalugin Jr."}

class OBJECT_OT_anki_preference_set(Operator):
	"""Set anki preferences operator """
	bl_idname = "scene.anki_preference_set"
	bl_label = "Anki Preferences Setter"
	bl_options = {'REGISTER', 'UNDO'}

	def execute(self, context):
		#get the environment variables
		addon_prefs = PREF.get_anki_exporter_preferences(context)
		ENV.ENVIRO = addon_prefs.anki_environment
		env_dct, export_dct, tools_dct = ENV.get_environment()
		ENV.set_environment(env_dct, tools_dct)

		# Set project paths
		addon_prefs.tools_path = tools_dct['tools_path']
		addon_prefs.export_path = export_dct['export_src_data']
		addon_prefs.map_path = export_dct['export_map_path']
		addon_prefs.texture_path = export_dct['export_texture_path']

		return {'FINISHED'}
