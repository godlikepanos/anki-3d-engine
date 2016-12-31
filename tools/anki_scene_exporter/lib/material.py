# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order

# blender imports
import bpy

bl_info = {"author": "A. A. Kalugin Jr."}

def get_texture_images_nodes():
	"""
	Gets the blender images using the materials
	"""
	bl_images = []
	mats = bpy.data.materials
	for mat in mats:
		for slot in mat.texture_slots:
			if slot:
				if slot.texture.type == 'IMAGE':
					if (slot.texture.image != None):
						bl_images.append(slot.texture.image)

	return bl_images

	def check_material(self, material):
		if material is not None:
			if material.use_nodes:
				if material.active_node_material is not None:
					return True
				return False
			return True
		return False
