# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

# system imports
import os
import sys

bl_info = {"author": "A. A. Kalugin Jr."}

################################ CONSTANTS ###############################
ENVIRO = ""
################################ CONSTANTS ###############################

def get_common_environment_path():
	"""
	Returns a string of the common path of a given environment variable
	"""
	path_list = os.getenv(ENVIRO).split(":")
	return os.path.commonprefix(path_list)

def get_environment():
	"""
	Returns three dictionaries of various environment paths.
	Build, export and tools in the anki path environment.
	"""
	environment_root = os.path.abspath(get_common_environment_path())
	# environment_root = (os.path.abspath(environment_root)) #clean the path
	# ANKI Environment
	anki_build_path       = "{0}/build".format(environment_root)
	anki_shaders_path     = "{0}/shaders".format(environment_root)
	anki_engine_data_path = "{0}/engine_data".format(environment_root)
	# Export Environment
	export_data_path      = "{0}/data".format(environment_root)
	export_src_data       = "{0}/src_data".format(export_data_path)
	export_map_path       = "{0}/map".format(export_data_path)
	export_temp_dea       = "{0}/tmp_scene.dae".format(export_src_data)
	export_texture_path   = "{0}/texture".format(export_data_path)
	# Tools Environment
	tool_etcpack_path     = "{0}/thirdparty/bin".format(environment_root)
	tools_path            = "{0}/tools".format(anki_build_path)
	tools_scene_path      = "{0}/tools/scene".format(environment_root)
	tools_texture_path    = "{0}/tools/texture".format(environment_root)

	# Make the Export Paths
	for _path in [export_src_data, export_map_path, export_texture_path]:
		if not os.path.exists(_path):
			print ("Making directory:", _path)
			os.makedirs(_path)

	# anki checked out path dictionary
	env_dct = {
					'anki_root_path':environment_root+'/',
					'anki_build_path':anki_build_path,
					'anki_engine_data_path':anki_engine_data_path,
					'anki_shaders_path':anki_shaders_path,
					}

	# export path dictionary
	export_dct = {
					'export_src_data':export_src_data,
					'export_map_path':export_map_path,
					'export_texture_path':export_texture_path,
					'export_temp_dea':export_temp_dea,
					}

	# tools path dictionary
	tools_dct = {
					'tool_etcpack_path':tool_etcpack_path,
					'tools_path':tools_path,
					'tools_scene_path':tools_scene_path,
					'tools_texture_path':tools_texture_path,
					}
	return env_dct, export_dct, tools_dct

def set_environment(anki_env_dct, tools_dct):
	"""
	Sets the environment variable.
	"""
	# Set the environment variable for anki
	environment_path = os.pathsep.join(anki_env_dct.values())
	os.environ[ENVIRO]=environment_path

	# Append the tools path to the $PATH
	tools_path = os.pathsep.join(tools_dct.values())

	# Create a clean PATH environment varaible
	os.environ["PATH"] += os.pathsep + tools_path
	path_l = os.getenv("PATH").split(os.pathsep)
	new_path = os.pathsep.join(set(path_l))
	os.environ["PATH"] = new_path
