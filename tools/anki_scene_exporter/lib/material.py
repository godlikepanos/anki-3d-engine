# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE Panagiotis Christopoulos Charitos and contributors
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
				if (slot.texture.image != None):
					bl_images.append(slot.texture.image)
	return bl_images

