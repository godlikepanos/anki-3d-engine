import bpy

from bpy.types import Panel

def shader_property_layout(context, layout, material):
	col = layout.column(align=True)
	row = col.row(align=True)
	row.operator("wm.anki_proxy_button", text="", emboss=True) # pushes to the right top buttons

	row.prop( material.ANKI, "shader_stand", text="",
			  icon=('STRANDS'
							if material.ANKI.shader_stand
							else'STRANDS' ),
			  emboss=True)

	row.prop( material.ANKI, "indirect_illum", text="",
			  icon=('MATERIAL'
							if material.ANKI.indirect_illum
							else 'MATERIAL' ),
			  emboss=True)

	row.prop( material.ANKI, "shader_rendering", text="",
			  icon=('RESTRICT_RENDER_OFF'
							if material.ANKI.shader_rendering
							else 'RESTRICT_RENDER_ON' ),
			  emboss=True)

	row.prop( material.ANKI, "shader_shadow", text="",
			  icon=('MATERIAL'
							if material.ANKI.shader_shadow
							else 'MATERIAL' ),
			  emboss=True)

	row.prop( material.ANKI, "shader_sss", text="",
			  icon=('MATERIAL'
							if material.ANKI.shader_sss
							else 'MATERIAL' ),
			  emboss=True)
	row.prop( material.ANKI, "shader_transparency", text="",
			  icon=('MATERIAL'
							if material.ANKI.shader_transparency
							else 'MATERIAL' ),
			  emboss=True)
	row.prop( material.ANKI, "shader_reflection", text="",
			  icon=('MATERIAL'
							if material.ANKI.shader_reflection
							else 'MATERIAL' ),
			  emboss=True)

	if material.ANKI.shader_stand:
		shader_strand_layout(context, layout, material)

	if material.ANKI.indirect_illum:
		shader_indirect_illum_layout(layout, material)

	if material.ANKI.shader_rendering:
		shader_rendering_layout(layout, material)

	if material.ANKI.shader_shadow:
		shader_shadow_layout(layout, material)

	if material.ANKI.shader_sss:
		shader_sss(layout, material)

	if material.ANKI.shader_transparency:
		shader_transparency(layout, material)

	if material.ANKI.shader_reflection:
		shader_reflection(layout, material)

class ANKI_material(Panel):
	"""Creates a Panel in the scene context of the properties editor"""
	bl_label = "Layout Demo"
	bl_idname = "SCENE_PT_layout"
	bl_space_type = 'VIEW_3D'
	bl_region_type = 'UI'
	bl_context = "scene"

	def draw(self, context):
		layout = self.layout
		scene = context.scene
		ob = context.active_object
		material = context.active_object.active_material

		is_sortable = (len(ob.material_slots) > 1)
		rows = 1
		if is_sortable:
			rows = 4
		layout.template_list("MATERIAL_UL_matslots", "",
		ob, "material_slots", ob, "active_material_index", rows=rows)
		layout.template_ID(ob, "active_material", new="material.new")

		if material!= None:
			#scene = context.scene
			#ob = context.active_object
			diff_texture_slot = material.texture_slots[0]
			ruff_texture_slot = material.texture_slots[1]
			nrml_texture_slot = material.texture_slots[2]
			refl_texture_slot = material.texture_slots[3]
			trns_texture_slot = material.texture_slots[4]

			# shader_property_layout(context, layout, material)

			##########################################################
			# DIFFUSE PORT
			##########################################################
			split = layout.split(percentage=0.1)
			row = split.row(align=True)
			row.prop(material, "diffuse_color", text="")
			row = split.row(align=True)
			row.prop(material, "diffuse_intensity", text="Diffuse Intensity")
			# row.operator("wm.anki_proxy_button", text="", icon='IMAGE_COL')
			diff_port = row.operator("anki.apply_texture_image", text = "", icon='IMAGE_COL')
			diff_port.mat_port_type = "diffuse"

			# if material.ANKI.diff_image:
			if diff_texture_slot:
				col = layout.column(align=True)
				row = col.row(align=True)

				# diff_port = row.operator("anki.apply_texture_image", text="Re-Apply New Diffuse Image")
				# diff_port.mat_port_type = "diffuse"


				# row = col.row(align=True)
				# row.label(text="") #spacer align to the right
				row.prop(material.ANKI, "diff_img_display", text="Toggle Properties",
						 icon=('VISIBLE_IPO_ON' if material.ANKI.diff_img_display
								else 'VISIBLE_IPO_OFF'), emboss=True)

				prop = row.operator("anki.clear_texture_slot" ,text="",icon = 'X', emboss=True)
				prop.port_type= "diffuse"

				if material.ANKI.diff_img_display:
					box = layout.box()
					box.prop_search(diff_texture_slot, "uv_layer", ob.data, "uv_textures", text="")
					box.label(text="UGLY BLENDER DESIGN---> CAN'T FIX")
					box.template_image(diff_texture_slot.texture, "image", diff_texture_slot.texture.image_user)
			# else:
			# 	diff_port = layout.operator("anki.apply_texture_image", text="Load Diffuse Image")
			# 	diff_port.mat_port_type = "diffuse"

			# ROUGHNESS PORT
			port_type = "roughness"
			split = layout.split(percentage=0.1)
			row = split.row(align=True)
			row.prop(material, "specular_color", text="")
			row = split.row(align=True)
			row.prop(material, "specular_hardness", text="Roughness Intensity")
			ruff_port = row.operator("anki.apply_texture_image", text = "", icon='IMAGE_COL')
			ruff_port.mat_port_type = port_type

			if ruff_texture_slot:
				col = layout.column(align=True)
				row = col.row(align=True)
				row.prop(material.ANKI, "ruff_img_display", text="Toggle Properties",
						 icon=('VISIBLE_IPO_ON' if material.ANKI.ruff_img_display
								else 'VISIBLE_IPO_OFF'), emboss=True)

				prop = row.operator("anki.clear_texture_slot" ,text="",icon = 'X', emboss=True)
				prop.port_type = port_type

				if material.ANKI.ruff_img_display:
					box = layout.box()
					box.prop(ruff_texture_slot, "hardness_factor", text="", slider=True)
					box.prop_search(ruff_texture_slot, "uv_layer", ob.data, "uv_textures", text="")
					# sub.prop(tex, factor, text=name, slider=True)
					box.label(text="UGLY BLENDER DESIGN---> CAN'T FIX")
					box.template_image(ruff_texture_slot.texture, "image", ruff_texture_slot.texture.image_user)

			# SPECULAR COLOR PORT
			# split = layout.split(percentage=0.1)
			# row = split.row(align=True)
			# row.prop(material, "specular_hardness", text="Roughness Intensity")
			# row.prop(material, "specular_hardness", text="Roughness Intensity")
			# row.operator("wm.anki_proxy_button", text="", icon='IMAGE_COL')

			# METALIC PORT
			split = layout.split(percentage=0.1)
			row = split.row(align=True)
			row.prop(material, "mirror_color", text="")
			row = split.row(align=True)
			row.prop(material.raytrace_mirror,"reflect_factor", text="Metallic Intensity")
			row.operator("wm.anki_proxy_button", text="", icon='IMAGE_COL')




			 # Diffuse, normal, roughness, metallic, emission, height

			# split = layout.split(percentage=0.1)
			# row = split.row(align=True)
			# row.prop(material, "diffuse_color", text="")

			row = layout.row(align=True)
			row.prop(material, "diffuse_intensity", text="Normal Intensity")
			row.operator("wm.anki_proxy_button", text="", icon='IMAGE_COL')


			# row.prop(material, "diffuse_color", text="")
			# row = split.row(align=True)
			# row.prop(material, "diffuse_intensity", text="Intensity")
			# row.operator("wm.anki_proxy_button", text="", icon='IMAGE_COL')

			layout.label(text=" Emission:")
			split = layout.split(percentage=0.1)
			row = split.row(align=True)
			row.prop(material, "diffuse_color", text="")
			row = split.row(align=True)
			row.prop(material, "diffuse_intensity", text="Intensity")
			row.operator("wm.anki_proxy_button", text="", icon='IMAGE_COL')

			layout.label(text=" Height:")
			split = layout.split(percentage=0.1)
			row = split.row(align=True)
			row.prop(material, "diffuse_color", text="")
			row = split.row(align=True)
			row.prop(material, "diffuse_intensity", text="Intensity")
			row.operator("wm.anki_proxy_button", text="", icon='IMAGE_COL')

		# # Create a simple row.

		# row = layout.row()
		# row.prop(scene, "frame_start")
		# row.prop(scene, "frame_end")

		# # Create an row where the buttons are aligned to each other.
		# layout.label(text=" Aligned Row:")

		# row = layout.row(align=True)
		# row.prop(scene, "frame_start")
		# row.prop(scene, "frame_end")

		# # Create two columns, by using a split layout.
		# split = layout.split()

		# # First column
		# col = split.column()
		# col.label(text="Column One:")
		# col.prop(scene, "frame_end")
		# col.prop(scene, "frame_start")

		# # Second column, aligned
		# col = split.column(align=True)
		# col.label(text="Column Two:")
		# col.prop(scene, "frame_start")
		# col.prop(scene, "frame_end")

		# # Big render button
		# layout.label(text="Big Button:")
		# row = layout.row()
		# row.scale_y = 3.0
		# row.operator("render.render")

		# # Different sizes in a row
		# layout.label(text="Different button sizes:")
		# row = layout.row(align=True)
		# row.operator("render.render")

		# sub = row.row()
		# sub.scale_x = 2.0
		# sub.operator("render.render")

		# row.operator("render.render")


def register():
	bpy.utils.register_class(ANKI_material)


def unregister():
	bpy.utils.unregister_class(ANKI_material)


if __name__ == "__main__":
	register()
