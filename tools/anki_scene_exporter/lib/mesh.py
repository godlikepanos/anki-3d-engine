import bpy
from bpy import context

def add_uv_channel(obj):
	scn = bpy.context.scene
	scn.objects.active = obj
	bpy.ops.mesh.uv_texture_add()

def get_textures(obj):
	#build a list of images, one per material
	images = []
	#get the textures from the mats
	for slot in obj.material_slots:
		if slot.material is None:
			continue
		has_image = False
		textures = zip(slot.material.texture_slots, slot.material.use_textures)
		for tex_slot, enabled in textures:
			if enabled and tex_slot is not None:
				tex = tex_slot.texture
				if tex.type == 'IMAGE':
					images.append(tex.image)
					has_image = True
					break

		if not has_image:
			print('not an image in', slot.name)
			images.append(None)
	return images

def get_uv_channel(mesh):
	r"""add uvs if none exist"""
	print (mesh)
	if len(mesh.uv_textures) == 0:
		return False
	return True

def set_textured(obj, draw_texture = True):
	images = get_textures(obj)
	mesh = obj.data

	uv_channel = get_uv_channel(mesh)
	if len(images) > 0 and not uv_channel:
		add_uv_channel(obj)

	if len(images):
		if obj.active_material != None:
			for t in  mesh.uv_textures:
				if t.active:
					uvtex = t.data
					for f in mesh.polygons:
						if draw_texture:
							#check that material had an image!
							if images[f.material_index] is not None:
								uvtex[f.index].image = images[f.material_index]
							else:
								uvtex[f.index].image = None
						else:
							uvtex[f.index].image = None
		mesh.update()


class MeshUtilities(object):
	# obj = bpy.context.object
	def __init__(self):
		self.cntx = bpy.context

	def get_selection_mode(self):
		settings = bpy.context.tool_settings
		if bpy.context.mode == 'EDIT_MESH':
			sub_mod = settings.mesh_select_mode
			if sub_mod[0] == True: #Verts
				return "VERT"
			if sub_mod[1] == True: #Edge
				return "EDGE"
			if sub_mod[2] == True: #Face
				return "FACE"
		return "OBJECT"

	def get_sub_selection(self, mesh_node):
		r''' Assumed that obj.type == MESH '''
		mesh_node.update_from_editmode() # Loads edit-mode data into ect data
		mode = self.get_selection_mode()
		if self.get_selection_mode() == "FACE":
			return [p for p in mesh_node.data.polygons if p.select]
		if self.get_selection_mode() == "EDGE":
			return [e for e in mesh_node.data.edges if e.select]
		if self.get_selection_mode() == "VERTEX":
			return [v for v in mesh_node.data.vertices if v.select]
		return []

	def get_modifier(self, obj, mod_type):
		r''' Returns a modifier of type '''
		for mod in obj.modifiers:
			if mod.type == mod_type:
				return mod
		return False

	def set_origin_to_selected(self, obj):
		obj.update_from_editmode()
		if bpy.context.mode == 'EDIT_MESH':
			saved_location = bpy.context.scene.cursor_location.copy()
			bpy.ops.view3d.snap_cursor_to_selected()
			bpy.ops.object.mode_set(mode = 'OBJECT')
			bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
			bpy.context.scene.cursor_location = saved_location
			bpy.ops.object.mode_set(mode = 'EDIT')
			return True
		if bpy.context.mode == 'OBJECT':
			bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY')
			return True
		return False

	def set_modifier(self, obj, name):
		r''' '''
		return obj.modifiers.new("bsi-subsurf", name)

	def is_editable(self, ob):
		editable = ['MESH','CURVE']
		if bpy.context.active_object.type in editable:
			return True
		return False

	def get_textures(self, obj):
		#build a list of images, one per material
		images = []
		#get the textures from the mats
		for slot in obj.material_slots:
			if slot.material is None:
				continue
			has_image = False
			textures = zip(slot.material.texture_slots, slot.material.use_textures)
			for tex_slot, enabled in textures:
				if enabled and tex_slot is not None:
					tex = tex_slot.texture
					if tex.type == 'IMAGE':
						images.append(tex.image)
						has_image = True
						break

			if not has_image:
				print('not an image in', slot.name)
				images.append(None)
		return images

	def get_uv_channel(self, mesh):
		r"""add uvs if none exist"""
		print (mesh)
		if len(mesh.uv_textures) == 0:
			return False
		return True

	def add_uv_channel(self, obj):
		scn = bpy.context.scene
		scn.objects.active = obj
		bpy.ops.mesh.uv_texture_add()

	def set_textured(self, obj, draw_texture = True):
		images = self.get_textures(obj)
		mesh = obj.data

		uv_channel = self.get_uv_channel(mesh)
		if len(images) > 0 and not uv_channel:
			self.add_uv_channel(obj)

		if len(images):
			if obj.active_material != None:
				for t in  mesh.uv_textures:
					if t.active:
						uvtex = t.data
						for f in mesh.polygons:
							if draw_texture:
								#check that material had an image!
								if images[f.material_index] is not None:
									uvtex[f.index].image = images[f.material_index]
								else:
									uvtex[f.index].image = None
							else:
								uvtex[f.index].image = None
			mesh.update()

	def set_objects_textured(self, draw_texture = True):
		for ob in bpy.data.objects:
			if ob.type == 'MESH':
				self.set_textured(ob, draw_texture)