# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

bl_info = {"author": "A. A. Kalugin Jr."}

def get_anki_exporter_preferences(context):
	"""
	Gets the scene preferences.
	"""
	user_preferences = context.user_preferences
	return user_preferences.addons['anki_scene_exporter'].preferences