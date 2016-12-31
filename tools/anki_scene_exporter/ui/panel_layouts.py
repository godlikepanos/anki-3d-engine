# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep methods in alphabetical order


import bpy, os
from bpy import context
from bpy.app.translations import contexts as i18n_contexts
# from ..libs import material_utilities
# from ..libs import mesh_utilities
# from ..libs import gui_utilities

############################### CONSTANTS ###############################
# MAT = material_utilities.MaterialUtilities()
# GUI = gui_utilities.GuiUtilities()
# MESH = mesh_utilities.MeshUtilities()
# ico_path = "%s/../libs/ico" % os.path.dirname(os.path.realpath(__file__))
# ico = gui_utilities.get_icons(ico_path)
############################### CONSTANTS ###############################

def bsi_empty_panel(layout, context):
	pass

def VIEW3D_PP_meshproperties(layout, context):
	use_freestyle = bpy.app.build_options.freestyle
	mesh = context.active_object.data
	scene = context.scene

	if (context.active_object and (context.mode == 'EDIT_MESH')):
		box = layout.box()
		split = box.split()
		col = split.column()
		col.label(text="Overlays:")
		col.prop(mesh, "show_faces", text="Faces")
		col.prop(mesh, "show_edges", text="Edges")
		col.prop(mesh, "show_edge_crease", text="Creases")
		if use_freestyle:
			col.prop(mesh, "show_edge_seams", text="Seams")

		box.prop(mesh, "show_weight")

		col = split.column()
		col.label()
		if not use_freestyle:
			col.prop(mesh, "show_edge_seams", text="Seams")
		col.prop(mesh, "show_edge_sharp", text="Sharp", text_ctxt=i18n_contexts.plural)
		col.prop(mesh, "show_edge_bevel_weight", text="Bevel")
		if use_freestyle:
			col.prop(mesh, "show_freestyle_edge_marks", text="Edge Marks")
			col.prop(mesh, "show_freestyle_face_marks", text="Face Marks")

		box = layout.box()

		box.label(text="Normals:")
		row = box.row(align=True)

		row.prop(mesh, "show_normal_vertex", text="", icon='VERTEXSEL')
		row.prop(mesh, "show_normal_loop", text="", icon='LOOPSEL')
		row.prop(mesh, "show_normal_face", text="", icon='FACESEL')

		sub = row.row(align=True)
		sub.active = mesh.show_normal_vertex or mesh.show_normal_face or mesh.show_normal_loop
		sub.prop(scene.tool_settings, "normal_size", text="Size")

		box = layout.box()
		split = box.split()
		col = split.column()
		col.label(text="Edge Info:")
		col.prop(mesh, "show_extra_edge_length", text="Length")
		col.prop(mesh, "show_extra_edge_angle", text="Angle")
		col = split.column()
		col.label(text="Face Info:")
		col.prop(mesh, "show_extra_face_area", text="Area")
		col.prop(mesh, "show_extra_face_angle", text="Angle")
		if bpy.app.debug:
			layout.prop(mesh, "show_extra_indices")

		statvis = context.tool_settings.statvis
		box = layout.box()
		box.prop(mesh, "show_statvis", text="Mesh Analysis")
		if mesh.show_statvis:
			box.prop(statvis, "type")
			if statvis.type == 'OVERHANG':
				row = box.row(align=True)
				row.prop(statvis, "overhang_min", text="")
				row.prop(statvis, "overhang_max", text="")
				row = box.row(align=True)
				row.prop(statvis, "overhang_axis", expand=True)
			elif statvis.type == 'THICKNESS':
				row = box.row(align=True)
				row.prop(statvis, "thickness_min", text="")
				row.prop(statvis, "thickness_max", text="")
				row = box.row(align=True)
				rowt.prop(statvis, "thickness_samples")
			elif statvis_type == 'INTERSECT':
				pass
			elif statvis.type == 'DISTORT':
				row = box.row(align=True)
				row.prop(statvis, "distort_min", text="")
				row.prop(statvis, "distort_max", text="")
			elif statvis.type == 'SHARP':
				row = box.row(align=True)
				row.prop(statvis, "sharp_min", text="")
				row.prop(statvis, "sharp_max", text="")


def VIEW3D_PP_viewproperties(layout, context):
	v3d = GUI.get_screen_area(type = "VIEW_3D")
	col = layout.column()
	col.active = bool(v3d.region_3d.view_perspective != 'CAMERA' or v3d.region_quadviews)
	col.prop(v3d, "lens")
	col.label(text="Lock to Object:")
	col.prop(v3d, "lock_object", text="")
	lock_object = v3d.lock_object
	if lock_object:
		if lock_object.type == 'ARMATURE':
			col.prop_search(v3d, "lock_bone", lock_object.data,
							"edit_bones" if lock_object.mode == 'EDIT'
							else "bones",
							text="")
	else:
		col.prop(v3d, "lock_cursor", text="Lock to Cursor")

	col = layout.column()
	col.prop(v3d, "lock_camera")

	col = layout.column(align=True)
	col.label(text="Clip:")
	col.prop(v3d, "clip_start", text="Start")
	col.prop(v3d, "clip_end", text="End")

	subcol = col.column(align=True)
	subcol.enabled = not v3d.lock_camera_and_layers
	subcol.label(text="Local Camera:")
	subcol.prop(v3d, "camera", text="")

	col = layout.column(align=True)
	col.prop(view, "use_render_border")
	col.active = v3d.region_3d.view_perspective != 'CAMERA'


def VIEW3D_TP_addobject(layout, context):
	col = layout.column(align=True)

	col.label(text="Primitives:")
	row = col.row(align=True)
	row.operator("mesh.primitive_plane_add", text=" ", icon='MESH_PLANE')
	row.operator("mesh.primitive_cube_add", text=" ", icon='MESH_CUBE')
	row.operator("mesh.primitive_circle_add", text=" ", icon='MESH_CIRCLE')
	row.operator("mesh.primitive_uv_sphere_add", text=" ", icon='MESH_UVSPHERE')
	row.operator("mesh.primitive_ico_sphere_add", text=" ", icon='MESH_ICOSPHERE')
	row = col.row(align=True)
	row.operator("mesh.primitive_cylinder_add", text=" ", icon='MESH_CYLINDER')
	row.operator("mesh.primitive_cone_add", text=" ", icon='MESH_CONE')
	row.operator("mesh.primitive_torus_add", text=" ", icon='MESH_TORUS')
	row.operator("mesh.primitive_grid_add", text=" ", icon='MESH_GRID')
	row.operator("mesh.primitive_monkey_add", text=" ", icon='MESH_MONKEY')

	col = layout.column(align=True)
	col.label(text="Misc:")
	row = col.row(align=True)
	row.operator("object.text_add", text=" ", icon='OUTLINER_OB_FONT')
	row.operator("object.armature_add", text=" ", icon='OUTLINER_OB_ARMATURE')
	row.operator("object.add", text=" ", icon='OUTLINER_OB_LATTICE').type = 'LATTICE'
	row = col.row(align=True)
	row.operator("object.empty_add", text=" ", icon='OUTLINER_OB_EMPTY').type = 'PLAIN_AXES'
	row.operator("object.speaker_add", text=" ", icon='OUTLINER_OB_SPEAKER')
	row.operator("object.camera_add", text=" ", icon='OUTLINER_OB_CAMERA')

	col = layout.column(align=True)
	col.label(text="Bezier:")
	row = col.row(align=True)
	row.operator("curve.primitive_bezier_curve_add", text="Bezier", icon='CURVE_BEZCURVE')
	row.operator("curve.primitive_bezier_circle_add", text="Circle", icon='CURVE_BEZCIRCLE')


	col.label(text="Nurbs and Surfaces:")
	row = col.row(align=True)
	row.operator("curve.primitive_nurbs_curve_add", text=" ", icon='CURVE_NCURVE')
	row.operator("curve.primitive_nurbs_circle_add", text=" ", icon='CURVE_NCIRCLE')
	row.operator("curve.primitive_nurbs_path_add", text=" ", icon='CURVE_PATH')

	#col = layout.column(align=True)
	#col.label(text="Surface:  ")
	row = col.row(align=True)
	row.operator("surface.primitive_nurbs_surface_curve_add", text=" ", icon='SURFACE_NCURVE')
	row.operator("surface.primitive_nurbs_surface_circle_add", text=" ", icon='SURFACE_NCIRCLE')
	row.operator("surface.primitive_nurbs_surface_surface_add", text=" ", icon='SURFACE_NSURFACE')
	row = col.row(align=True)
	row.operator("surface.primitive_nurbs_surface_cylinder_add", text=" ", icon='SURFACE_NCYLINDER')
	row.operator("surface.primitive_nurbs_surface_sphere_add", text=" ", icon='SURFACE_NSPHERE')
	row.operator("surface.primitive_nurbs_surface_torus_add", text=" ", icon='SURFACE_NTORUS')

	col = layout.column(align=True)
	row = col.row(align=True)
	# row.operator_enum("object.metaball_add","type")
	row.operator_menu_enum("object.metaball_add", "type",
							text="Metaball",
							icon='OUTLINER_OB_META'
							)
	row.operator_menu_enum("object.lamp_add", "type",
							text="Lamp",
							icon='OUTLINER_OB_LAMP'
							)

def VIEW3D_TP_rigidbody(layout, context):
	col = layout.column(align=True)
	col.label(text="Add/Remove:")
	row = col.row(align=True)
	row.operator("rigidbody.objects_add", text="Add Active").type = 'ACTIVE'
	row.operator("rigidbody.objects_add", text="Add Passive").type = 'PASSIVE'
	row = col.row(align=True)
	row.operator("rigidbody.objects_remove", text="Remove")

	col = layout.column(align=True)
	col.label(text="Object Tools:")
	col.operator("rigidbody.shape_change", text="Change Shape")
	col.operator("rigidbody.mass_calculate", text="Calculate Mass")
	col.operator("rigidbody.object_settings_copy", text="Copy from Active")
	col.operator("object.visual_transform_apply", text="Apply Transformation")
	col.operator("rigidbody.bake_to_keyframes", text="Bake To Keyframes")
	col.label(text="Constraints:")
	col.operator("rigidbody.connect", text="Connect")

def VIEW3D_TP_relation(layout, context):
	col = layout.column(align=True)

	col.label(text="Group:")
	col.operator("group.create", text="New Group")
	col.operator("group.objects_add_active", text="Add to Active")
	col.operator("group.objects_remove", text="Remove from Group")

	col.separator()

	col.label(text="Parent:")
	row = col.row(align=True)
	row.operator("object.parent_set", text="Set")
	row.operator("object.parent_clear", text="Clear")

	col.separator()

	col.label(text="Object Data:")
	col.operator("object.make_links_data")
	col.operator("object.make_single_user")

	col.separator()

	col.label(text="Linked Objects:")
	col.operator("object.make_local")
	col.operator("object.proxy_make")

def draw_keyframing_tools(context, layout):
	col = layout.column(align=True)
	col.label(text="Keyframes:")
	row = col.row(align=True)
	row.operator("anim.keyframe_insert_menu", text="Insert")
	row.operator("anim.keyframe_delete_v3d", text="Remove")

def VIEW3D_TP_animation(layout, context):
	draw_keyframing_tools(context, layout)
	col = layout.column(align=True)
	col.label(text="Motion Paths:")
	row = col.row(align=True)
	row.operator("object.paths_calculate", text="Calculate")
	row.operator("object.paths_clear", text="Clear")

	col.separator()

	col.label(text="Action:")
	col.operator("nla.bake", text="Bake Action")

def gpencil_stroke_placement_settings(context, layout, gpd):
	col = layout.column(align=True)

	col.label(text="Stroke Placement:")

	row = col.row(align=True)
	row.prop_enum(gpd, "draw_mode", 'VIEW')
	row.prop_enum(gpd, "draw_mode", 'CURSOR')

	if context.space_data.type == 'VIEW_3D':
		row = col.row(align=True)
		row.prop_enum(gpd, "draw_mode", 'SURFACE')
		row.prop_enum(gpd, "draw_mode", 'STROKE')

		row = col.row(align=False)
		row.active = gpd.draw_mode in {'SURFACE', 'STROKE'}
		row.prop(gpd, "use_stroke_endpoints")

def VIEW3D_TP_greasepencil(layout, context):
	col = layout.column(align=True)

	col.label(text="Draw:")
	row = col.row(align=True)
	row.operator("gpencil.draw", text="Draw").mode = 'DRAW'
	row.operator("gpencil.draw", text="Erase").mode = 'ERASER'

	row = col.row(align=True)
	row.operator("gpencil.draw", text="Line").mode = 'DRAW_STRAIGHT'
	row.operator("gpencil.draw", text="Poly").mode = 'DRAW_POLY'

	row = col.row(align=True)
	row.prop(context.tool_settings, "use_grease_pencil_sessions", text="Continuous Drawing")

	if context.space_data.type in {'VIEW_3D', 'CLIP_EDITOR'}:
		col.separator()
		col.label("Data Source:")
		row = col.row(align=True)
		if context.space_data.type == 'VIEW_3D':
			row.prop(context.tool_settings, "grease_pencil_source", expand=True)
		elif context.space_data.type == 'CLIP_EDITOR':
			row.prop(context.space_data, "grease_pencil_source", expand=True)

	gpd = context.gpencil_data
	if gpd:
		col.separator()
		gpencil_stroke_placement_settings(context, col, gpd)

	if context.space_data.type == 'VIEW_3D':
		col.separator()
		col.separator()

		col.label(text="Tools:")
		col.operator("gpencil.convert", text="Convert...")
		col.operator("view3d.ruler")

	gpd = context.gpencil_data
	edit_ok = bool(context.editable_gpencil_strokes) and bool(gpd.use_stroke_edit_mode)

	col = layout.column(align=True)
	col.prop(gpd, "use_stroke_edit_mode", text="Enable Editing", icon='EDIT', toggle=True)

	col.separator()

	col.label(text="Select:")
	subcol = col.column(align=True)
	subcol.active = edit_ok
	subcol.operator("gpencil.select_all", text="Select All")
	subcol.operator("gpencil.select_border")
	subcol.operator("gpencil.select_circle")

	col.separator()

	subcol = col.column(align=True)
	subcol.active = edit_ok
	subcol.operator("gpencil.select_linked")
	subcol.operator("gpencil.select_more")
	subcol.operator("gpencil.select_less")

	col.separator()

	col.label(text="Edit:")
	row = col.row(align=True)
	row.active = edit_ok
	row.operator("gpencil.copy", text="Copy")
	row.operator("gpencil.paste", text="Paste")

	subcol = col.column(align=True)
	subcol.active = edit_ok
	subcol.operator("gpencil.delete", text="Delete")
	subcol.operator("gpencil.duplicate_move", text="Duplicate")
	subcol.operator("transform.mirror", text="Mirror").gpencil_strokes = True

	col.separator()

	subcol = col.column(align=True)
	subcol.active = edit_ok
	subcol.operator("transform.translate").gpencil_strokes = True   # icon='MAN_TRANS'
	subcol.operator("transform.rotate").gpencil_strokes = True      # icon='MAN_ROT'
	subcol.operator("transform.resize", text="Scale").gpencil_strokes = True      # icon='MAN_SCALE'

	col.separator()

	subcol = col.column(align=True)
	subcol.active = edit_ok
	subcol.operator("transform.bend", text="Bend").gpencil_strokes = True
	subcol.operator("transform.shear", text="Shear").gpencil_strokes = True
	subcol.operator("transform.tosphere", text="To Sphere").gpencil_strokes = True



def VIEW3D_PP_motiontracking(layout, context):
	v3d = GUI.get_screen_area(type = "VIEW_3D")
	layout.prop(v3d, "show_reconstruction", text="Motion Tracking")
	if v3d.show_reconstruction:
		col = layout.column()
		col.prop(v3d, "show_camera_path", text="Camera Path")
		col.prop(v3d, "show_bundle_names", text="3D Marker Names")
		col.label(text="Track Type and Size:")
		row = col.row(align=True)
		row.prop(v3d, "tracks_draw_type", text="")
		row.prop(v3d, "tracks_draw_size", text="")

def VIEW3D_TP_meshedit(layout, context):
	mode = MESH.get_selection_mode()
	if mode == "OBJECT":
		mesh_edit_object_mode(layout, context)
	else:
		mesh_edit_edit_mode(layout, context)
def mesh_edit_edit_mode(layout, context):
	r""" Taken from blender mesh edit layout"""
	col = layout.column(align=True)
	col.label(text="Deform:")
	row = col.row(align=True)
	row.operator("transform.edge_slide", text="Slide Edge")
	row.operator("transform.vert_slide", text="Vertex")
	col.operator("mesh.noise")
	col.operator("mesh.vertices_smooth")
	col.operator("transform.vertex_random")

	col = layout.column(align=True)
	col.label(text="Add:")

	col.menu("VIEW3D_MT_edit_mesh_extrude")
	col.operator("view3d.edit_mesh_extrude_move_normal", text="Extrude Region")
	col.operator("view3d.edit_mesh_extrude_individual_move", text="Extrude Individual")
	col.operator("mesh.inset", text="Inset Faces")
	col.operator("mesh.edge_face_add")
	col.operator("mesh.subdivide")
	col.operator("mesh.loopcut_slide")
	col.operator("mesh.offset_edge_loops_slide")
	col.operator("mesh.duplicate_move", text="Duplicate")
	row = col.row(align=True)
	row.operator("mesh.spin")
	row.operator("mesh.screw")

	row = col.row(align=True)
	props = row.operator("mesh.knife_tool", text="Knife")
	props.use_occlude_geometry = True
	props.only_selected = False
	props = row.operator("mesh.knife_tool", text="Select")
	props.use_occlude_geometry = False
	props.only_selected = True
	col.operator("mesh.knife_project")
	col.operator("mesh.bisect")

	col = layout.column(align=True)
	col.label(text="Remove:")
	col.menu("VIEW3D_MT_edit_mesh_delete")
	col.operator_menu_enum("mesh.merge", "type")
	col.operator("mesh.remove_doubles")

def mesh_edit_object_mode(layout, context):
	r""" Taken from blender mesh edit layout"""
	col = layout.column(align=True)
	col.operator("object.duplicate_move", text="Duplicate")
	col.operator("object.duplicate_move_linked", text="Duplicate Linked")

	col.operator("object.delete")

	obj = context.active_object
	if obj:
		obj_type = obj.type

		if obj_type in {'MESH', 'CURVE', 'SURFACE', 'ARMATURE'}:
			col = layout.column(align=True)
			col.operator("object.join")

		if obj_type in {'MESH', 'CURVE', 'SURFACE', 'ARMATURE', 'FONT', 'LATTICE'}:
			col = layout.column(align=True)
			col.operator_menu_enum("object.origin_set", "type", text="Set Origin")

		if obj_type in {'MESH', 'CURVE', 'SURFACE'}:
			col = layout.column(align=True)
			col.label(text="Shading:")
			row = col.row(align=True)
			row.operator("object.shade_smooth", text="Smooth")
			row.operator("object.shade_flat", text="Flat")

		if obj_type == 'MESH':
			col = layout.column(align=True)
			col.label(text="Data Transfer:")
			row = col.row(align=True)
			row.operator("object.data_transfer", text="Data")
			row.operator("object.datalayout_transfer", text="Data Layout")


def bsi_modeling_panel(layout, context):
	col = layout.column(align=True)
	col.operator("wm.bsi_decorator_button",text="Get")
	col.menu("INFO_MT_bsi_primitive", text="Primitive")
	col.menu("INFO_MT_bsi_material", text="Material")
	col.menu("INFO_MT_bsi_property", text="Property")

	col = layout.column(align=True)
	col.operator("wm.bsi_decorator_button",text="Create")
	col.menu("VIEW3D_MT_BSI_curve_create", text="Curve")
	col.menu("INFO_MT_add", text="Surf.Mesh")
	col.menu("INFO_MT_add", text="Poly.Mesh")
	col.menu("INFO_MT_add", text="Skeleton")
	col.menu("INFO_MT_add", text="Model")
	col.menu("INFO_MT_add", text="Text")

	layout = self.layout
	col = layout.column(align=True)
	col.operator("wm.bsi_decorator_button",text="Modify")
	col.menu("INFO_MT_add", text="Curve")
	col.menu("INFO_MT_add", text="Surf.Mesh")
	col.menu("INFO_MT_add", text="Poly.Mesh")
	col.menu("INFO_MT_add", text="Deform")
	col.menu("INFO_MT_add", text="Model")

def viewport_shading_panel(layout, context):
	v3d 	= GUI.get_screen_area("VIEW_3D")
	scene 	= context.scene

	box = layout.box()
	box.label(text= "Viewport Shading")
	col = box.column()
	col.prop(context.scene, "boundbox", text="Bounding Box")
	col.prop(context.scene, "wireframe", text="Wireframe")
	col.prop(context.scene, "constant", text="Constant")
	col.prop(context.scene, "solid", text="Shaded")
	col.prop(context.scene, "textured", text="Textured")
	col.prop(context.scene, "textured_decal", text="Textured Decal")

def viewport_fx_panel(layout, context):
	v3d 			= GUI.get_screen_area("VIEW_3D")
	scene 			= context.scene
	game_settings 	= scene.game_settings
	box = layout.box()
	box.label(text= "FX Shading")
	col = box.column()
	if not scene.render.use_shading_nodes:
		col.prop(game_settings, "material_mode", text="")

	if v3d.viewport_shade == 'SOLID':
		col.prop(v3d, "use_matcap")
		if v3d.use_matcap:
			col.template_icon_view(v3d, "matcap_icon")
		if not v3d.use_matcap:
			col.prop(v3d, "show_textured_solid")

	fx_settings = v3d.fx_settings

	if v3d.viewport_shade not in {'BOUNDBOX', 'WIREFRAME'}:
		sub = col.column()
		sub.active = v3d.region_3d.view_perspective == 'CAMERA'
		sub.prop(fx_settings, "use_dof")
		col.prop(fx_settings, "use_ssao", text="Ambient Occlusion")
		if fx_settings.use_ssao:
			ssao_settings = fx_settings.ssao
			subcol = col.column(align=True)
			subcol.prop(ssao_settings, "factor")
			subcol.prop(ssao_settings, "distance_max")
			subcol.prop(ssao_settings, "attenuation")
			subcol.prop(ssao_settings, "samples")
			subcol.prop(ssao_settings, "color")

	col.prop(v3d, "show_only_render")
	col.prop(v3d, "show_world")

def viewport_objects_panel(layout, context):
	v3d 	= GUI.get_screen_area("VIEW_3D")
	scene 	= context.scene
	obj = context.object
	display_all = v3d.show_only_render
	box = layout.box()
	col = box.column()
	col.label(text="Object Shading")
	if display_all:
		col.label(text="Hidden because of \"Only Render\"")
	if not display_all:
		col.prop(context.scene, "show_wire", text="Wireframe On Shaded")
		if v3d.viewport_shade not in {'BOUNDBOX', 'WIREFRAME'}:
			if obj and obj.mode == 'EDIT':
				col.prop(context.scene, "occlude_wire", text="Occlude Wire")
				col.prop(v3d, "show_occlude_wire")
		col.prop(context.scene, "show_x_ray", text="X Ray <--LAME")
		col.prop(context.scene, "show_all_edges", text="All Edges <--WTF is this")
		col.prop(context.scene, "show_axis", text="Axis")
		col.prop(context.scene, "show_bounds", text="Bounding Boxes")
		col.prop(context.scene, "show_name", text="Names")
		col.prop(context.scene, "show_only_shape_key", text="Shapes")
		col.prop(v3d, "show_backface_culling")

		if v3d.show_backface_culling:
			col.prop(context.scene, "show_transparent", text="Hide Outline")
		col.prop(context.scene, "show_texture_space", text="Texture Space")
		col.prop(v3d, "show_outline_selected")
		col.prop(v3d, "show_all_objects_origin", text = "Object Origins")
		col.prop(v3d, "show_relationship_lines", text = "Relationships")

def viewport_widgets_panel(layout, context):
	v3d 			= GUI.get_screen_area("VIEW_3D")
	scene 			= context.scene

	box = layout.box()
	col = box.column()
	col.label(text="Viewport Widgets")
	display_all = v3d.show_only_render
	if display_all:
		col.label(text="Hidden because of \"Only Render\"")
	if not display_all:
		split = col.split(percentage=0.55)
		split.prop(v3d, "show_floor", text="Grid Floor")

		row = split.row(align=True)
		row.prop(v3d, "show_axis_x", text="X", toggle=True)
		row.prop(v3d, "show_axis_y", text="Y", toggle=True)
		row.prop(v3d, "show_axis_z", text="Z", toggle=True)

		sub = col.column(align=True)
		sub.prop(v3d, "grid_lines", text="Lines")
		sub.prop(v3d, "grid_scale", text="Scale")
		col.prop(v3d ,"cursor_location")
		subsub = sub.column(align=True)
		subsub.active = scene.unit_settings.system == 'NONE'
		subsub.prop(v3d, "grid_subdivisions", text="Subdivisions")

		box.operator("screen.region_quadview", text="Toggle Quad View")

		if v3d.region_quadviews:
			region = v3d.region_quadviews[2]
			col = box.column()
			col.prop(region, "lock_rotation")
			row = col.row()
			row.enabled = region.lock_rotation
			row.prop(region, "show_sync_view")
			row = col.row()
			row.enabled = region.lock_rotation and region.show_sync_view
			row.prop(region, "use_box_clip")

def viewport_properties_panel(layout, context):
	viewport_shading_panel(layout, context)
	viewport_fx_panel(layout, context)
	viewport_objects_panel(layout, context)
	viewport_widgets_panel(layout, context)

##########################################################
# dynamic prop panel methods
##########################################################


def ANKI_PP_home(layout, context):
	scene = context.scene

	# Create a simple row.
	layout.label(text=" Simple Row:")

	row = layout.row()
	row.prop(scene, "frame_start")
	row.prop(scene, "frame_end")
	row.prop(scene, "frame_end")
	row.prop(scene, "frame_end")

	# Create an row where the buttons are aligned to each other.
	layout.label(text=" Aligned Row:")

	row = layout.row(align=True)
	row.prop(scene, "frame_start")
	row.prop(scene, "frame_end")

	# Create two columns, by using a split layout.
	split = layout.split()

	# First column
	col = split.column()
	col.label(text="Column One:")
	col.prop(scene, "frame_end")
	col.prop(scene, "frame_start")

	# Second column, aligned
	col = split.column(align=True)
	col.label(text="Column Two:")
	col.prop(scene, "frame_start")
	col.prop(scene, "frame_end")

	# Big render button
	layout.label(text="Big Button:")
	row = layout.row()
	row.scale_y = 3.0
	row.operator("render.render")

	# Different sizes in a row
	layout.label(text="Different button sizes:")
	row = layout.row(align=True)
	row.operator("render.render")

	sub = row.row()
	sub.scale_x = 2.0
	sub.operator("render.render")

	row.operator("render.render")

def uv_tiling_layout(layout, texture_node):
	r""" UV tiling properties"""
	box =layout.box()
	box.prop(texture_node, "extension", text= "")
	col = box.column(align=True)
	row = col.row(align=True)

	if texture_node.extension == 'REPEAT':
		row.label(text="Mirror and Repeat:")
		col = box.column(align=True)
		row = col.row()
		row.prop(texture_node, "use_mirror_x", text="",
					icon_value=(ico["toggle_on"].icon_id
					if texture_node.use_mirror_x
					else ico["toggle_off"].icon_id ),
					emboss=False)
		row.prop(texture_node, "repeat_x", text="Repeat X")
		row.active = (texture_node.repeat_x > 1)

		row = col.row()
		row.prop(texture_node, "use_mirror_y", text="",
			icon_value=(ico["toggle_on"].icon_id
			if texture_node.use_mirror_y
			else ico["toggle_off"].icon_id ),
			emboss=False)
		row.prop(texture_node, "repeat_y", text="Repeat Y")
		row.active = (texture_node.repeat_y > 1)

	elif texture_node.extension == 'CHECKER':
		row = col.row(align=True)
		row.prop(texture_node, "use_checker_even", text="Even",
			icon_value=(ico["toggle_on"].icon_id
			if texture_node.use_checker_even
			else ico["toggle_off"].icon_id ),
			emboss=False)
		row.prop(texture_node, "use_checker_odd", text="Odd",
			icon_value=(ico["toggle_on"].icon_id
			if texture_node.use_checker_odd
			else ico["toggle_off"].icon_id ),
			emboss=False)

		col = box.column(align=True)
		col.prop(texture_node, "checker_distance", text="Distance")

	col.label(text="Crop:")
	row = col.row(align=True)
	row.prop(texture_node, "crop_min_y", text="Min Y")
	row.prop(texture_node, "crop_max_x", text="Max X")
	row = col.row(align=True)
	row.prop(texture_node, "crop_min_x", text="Min Y")
	row.prop(texture_node, "crop_max_y", text="Max Y")

def common_texture_filter(tex, col):
	row = col.row(align=True)
	row.label(text=":: Filter Properties" )
	row.label(text="" )

	row = col.row(align=True)
	row.prop(tex, "filter_type", text="")
	if tex.use_mipmap and tex.filter_type in {'AREA', 'EWA', 'FELINE'}:
		if tex.filter_type == 'FELINE':
			row.prop(tex, "filter_probes", text="Probes")
		else:
			row.prop(tex, "filter_eccentricity", text="Eccentricity")

	row = col.row(align=True)
	row.prop(tex, "use_filter_size_min", text="Min Size")
	row.prop(tex, "filter_size")

def uv_projection_layout(layout, texture_slot, object_node):
	r"""UV layout and project types """
	box =layout.box()
	col = box.column(align=True)
	row = col.row(align=True)
	row.prop(texture_slot, "texture_coords", text="")
	row.prop(texture_slot, "mapping", text="")
	if texture_slot.texture_coords == 'UV':
		row.prop_search(texture_slot, "uv_layer", object_node.data, "uv_textures", text="")
	elif texture_slot.texture_coords == 'OBJECT':
		row.prop(texture_slot, "object", text="")
	row.operator("mesh.uv_texture_add", text="", icon='ZOOMIN')

	col = box.column(align=True)
	row = col.row(align=True)
	if texture_slot.texture_coords in {'ORCO', 'UV'}:
		row.prop(texture_slot, "use_from_dupli", text="",
				  icon_value=(ico["toggle_on"].icon_id
								if texture_slot.use_from_dupli
								else ico["toggle_off"].icon_id ),
				  emboss=False)


	row.prop(texture_slot, "mapping_x", text="")
	row.prop(texture_slot, "mapping_y", text="")
	row.prop(texture_slot, "mapping_z", text="")

	col = box.column(align=True)
	row = box.row(align=True)
	row = col.row(align=True)

	row.column().prop(texture_slot, "offset")
	row.column().prop(texture_slot, "scale")

def texture_sampling_layout(layout, texture_slot):
	slot = texture_slot #getattr(context, "texture_slot", None)
	tex = slot.texture #context.texture
	box =layout.box()
	col = box.column(align=True)

	row = col.row(align=True)
	row.label(text=":: Alpha Properties" )
	row.label(text="" )

	row = col.row(align=True)
	row.prop(tex, "use_preview_alpha")

	row = col.row(align=True)
	row.prop(tex, "use_alpha", text="Use")
	row.prop(tex, "use_calculate_alpha", text="Calculate")

	row = col.row(align=True)
	row.prop(tex, "invert_alpha", text="Invert")
	row.prop(tex, "use_flip_axis", text="Flip X/Y Axis")

	col.separator()
	row = col.row(align=True)
	row.label(text=":: Normal Properties" )
	row.label(text="" )

	row = col.row(align=True)
	row.prop(tex, "use_normal_map")
	row.active = tex.use_normal_map
	row.prop(slot, "normal_map_space", text="")
	row.active = tex.use_normal_map

	row = col.row(align=True)
	row.prop(tex, "use_mipmap")
	row.prop(tex, "use_derivative_map")
	row.active = tex.use_mipmap

	row = col.row(align=True)
	row.prop(tex, "use_mipmap_gauss", text ="MIP Blur")
	row.prop(tex, "use_interpolation")

	col.separator()
	common_texture_filter(tex, col)

def texture_influence_layout(layout, texture_slot, port_type="diffuse"):
	tex = texture_slot
	box =layout.box()
	col = box.column()
	if port_type == "diffuse":
		GUI.button_slider(col, texture_slot, "use_map_diffuse", "diffuse_factor", "Intensity")
		GUI.button_slider(col, texture_slot, "use_map_color_diffuse", "diffuse_color_factor", "Color")
		GUI.button_slider(col, texture_slot, "use_map_alpha", "alpha_factor", "Alpha")
		GUI.button_slider(col, texture_slot, "use_map_translucency", "translucency_factor", "Translucency")
	if port_type == "specular":
		GUI.button_slider(col, texture_slot,"use_map_specular", "specular_factor", "Intensity")
		GUI.button_slider(col, texture_slot,"use_map_hardness", "hardness_factor", "Hardness")
		GUI.button_slider(col, texture_slot,"use_map_color_spec", "specular_color_factor", "Color")
	col = box.column(align=False)
	row = col.row(align =True)
	row.prop(texture_slot, "blend_type", text="")
	row = col.row(align =True)
	row.prop(texture_slot, "invert", text="Negative")
	row.prop(texture_slot, "use_stencil")
	# color is used on gray-scale textures even when use_rgb_to_intensity is disabled.
	row = col.row(align =True)
	row.prop(texture_slot, "use_rgb_to_intensity")
	if texture_slot.use_rgb_to_intensity:
		row.prop(texture_slot, "color", text="")

	row = col.row(align =True)
	row.prop(texture_slot, "default_value", text="DVar", slider=True)

def shader_property_layout(context, layout, material):
	col = layout.column(align=True)
	row = col.row(align=True)
	row.operator("wm.bsi_decorator_button", text="", emboss=False) # pushes to the right top buttons

	row.prop( material.BSI, "shader_stand", text="",
			  icon_value=(ico["shader_strand_on"].icon_id
							if material.BSI.shader_stand
							else ico["shader_strand_off"].icon_id ),
			  emboss=False)

	row.prop( material.BSI, "indirect_illum", text="",
			  icon_value=(ico["shader_shading_on"].icon_id
							if material.BSI.indirect_illum
							else ico["shader_shading_off"].icon_id ),
			  emboss=False)

	row.prop( material.BSI, "shader_rendering", text="",
			  icon=('RESTRICT_RENDER_OFF'
							if material.BSI.shader_rendering
							else 'RESTRICT_RENDER_ON' ),
			  emboss=False)

	row.prop( material.BSI, "shader_shadow", text="",
			  icon_value=(ico["shader_shadow_on"].icon_id
							if material.BSI.shader_shadow
							else ico["shader_shadow_off"].icon_id ),
			  emboss=False)

	row.prop( material.BSI, "shader_sss", text="",
			  icon_value=(ico["shader_sss_on"].icon_id
							if material.BSI.shader_sss
							else ico["shader_sss_off"].icon_id ),
			  emboss=False)
	row.prop( material.BSI, "shader_transparency", text="",
			  icon_value=(ico["shader_transparency_on"].icon_id
							if material.BSI.shader_transparency
							else ico["shader_transparency_off"].icon_id ),
			  emboss=False)
	row.prop( material.BSI, "shader_reflection", text="",
			  icon_value=(ico["shader_reflect_on"].icon_id
							if material.BSI.shader_reflection
							else ico["shader_reflect_off"].icon_id ),
			  emboss=False)

	if material.BSI.shader_stand:
		shader_strand_layout(context, layout, material)

	if material.BSI.indirect_illum:
		shader_indirect_illum_layout(layout, material)

	if material.BSI.shader_rendering:
		shader_rendering_layout(layout, material)

	if material.BSI.shader_shadow:
		shader_shadow_layout(layout, material)

	if material.BSI.shader_sss:
		shader_sss(layout, material)

	if material.BSI.shader_transparency:
		shader_transparency(layout, material)

	if material.BSI.shader_reflection:
		shader_reflection(layout, material)

def shader_ramp_layout(layout, material):
	box = layout.box()
	box.prop(material, "use_diffuse_ramp", text="Use Ramp Shader")
	if material.use_diffuse_ramp:
		col = box.column()
		col.active = (not material.use_shadeless)
		col.separator()
		col.template_color_ramp(material, "diffuse_ramp", expand=True)
		col.separator()

		row = col.row()
		row.prop(material, "diffuse_ramp_input", text="Input")
		row.prop(material, "diffuse_ramp_blend", text="Blend")

		col.prop(material, "diffuse_ramp_factor", text="Factor")

def shader_strand_layout(context, layout, material):
	box = layout.box()
	box.label(text="Strand Properties")
	tan = material.strand

	col = box.column(align=True)
	col.prop(tan, "use_blender_units")
	col.prop(tan, "root_size", text="Size Root")
	col.prop(tan, "tip_size", text="Size Tip")
	col.prop(tan, "size_min", text="Size Min")

	col = box.column(align=True)
	col.active = (not material.use_shadeless)
	col.prop(tan, "use_tangent_shading")
	col.prop(tan, "shape", text="Tangent Shape")

	col = box.column(align=True)
	row = col.row(align=True)
	row.prop(tan, "width_fade", text="Shading")
	if context.object and context.object.type == 'MESH':
		row.prop_search(tan, "uv_layer", context.object.data, "uv_textures", text="")
	else:
		row.prop(tan, "uv_layer", text="")
	col.label("Surface diffuse:")
	col.prop(tan, "blend_distance", text="Distance")

def shader_indirect_illum_layout(layout, material):
	box = layout.box()
	box.label(text="Indirect Illumination Properties")
	if material.type in {'SURFACE', 'WIRE'}:
		col = box.column(align=True)
		row = col.row(align=True)
		row.prop(material, "use_shadeless", text="Constant")
		col.active = not material.use_shadeless
		row.prop(material, "emit")
		row = col.row(align=True)
		row.prop(material, "use_tangent_shading", text="Tangent")
		row.prop(material, "ambient")
		row = col.row(align=True)
		row.prop(material, "use_cubic", text="Cubic ")
		row.prop(material, "translucency")

def shader_rendering_layout(layout, material):
	box = layout.box()
	col = box.column(align=True)
	row = col.row(align=True)
	row.label(text=":: Rendering")
	row.label(text="")

	row = col.row(align=True)
	if not material.use_nodes:
		row.prop(material, "use_raytrace")
		row.prop(material, "use_sky")
		row = col.row(align=True)
		row.prop(material, "use_full_oversampling", text="Oversampling")
	row.prop(material, "use_mist")

	col = box.column(align=True)
	row = col.row(align=True)
	if not material.use_nodes:
		row.prop(material, "invert_z")
		sub = row.row()
		sub.active = material.use_transparency and material.transparency_method == 'Z_TRANSPARENCY'
		sub.prop(material, "offset_z")

	col.separator()
	row = col.row(align=True)
	row.label(text=":: Shading")
	row.label(text="")

	row = col.row(align=True)
	row.prop(material, "use_face_texture")
	sub = row.column()
	sub.active = material.use_face_texture
	sub.prop(material, "use_face_texture_alpha", text="Use Alpha")

	row = col.row(align=True)
	row.prop(material, "use_vertex_color_paint")
	row.prop(material, "use_vertex_color_light")

	row = col.row(align=True)
	row.prop(material, "use_object_color")
	row.prop(material, "use_uv_project")

	row = col.row(align=True)
	row.label(text=":: Lighting")
	row.label(text="")

	col.separator()
	row = col.row()
	row.prop(material, "light_group", text="Light Group:")
	if not material.use_nodes:
		row.prop(material, "pass_index")

	row = col.row(align=True)
	row.active = bool(material.light_group)
	row.prop(material, "use_light_group_exclusive", text="Exclusive")
	row.prop(material, "use_light_group_local", text="Local")

def shader_shadow_layout(layout, material):
	box = layout.box()
	box.label(text="Shadow Properties")
	if not material.use_nodes:
		col = box.column()
		row = col.row(align=True)
		row.prop(material, "use_shadows", text="Receive")
		row.prop(material, "use_cast_shadows", text="Cast")
		row = col.row(align=True)
		row.prop(material, "use_transparent_shadows", text="Receive Transparent")
		row.prop(material, "use_cast_shadows_only", text="Cast Only")
		row = col.row(align=True)
		row.prop(material, "use_cast_approximate")
		row = col.row(align=True)
		row.prop(material, "use_only_shadow", text="Shadows Only")
		row.active = material.use_only_shadow
		row.prop(material, "shadow_only_type", text="")
		row = col.row(align=True)
		row.prop(material, "use_cast_buffer_shadows")
		row.active = material.use_cast_buffer_shadows
		row.prop(material, "shadow_buffer_bias", text="Buffer Bias")
		row = col.row(align=True)
		row.label(text="")
		row.active = material.use_cast_buffer_shadows
		row.prop(material, "shadow_cast_alpha", text="Casting Alpha")
	row = col.row(align=True)
	row.prop(material, "use_ray_shadow_bias", text="Auto Ray Bias")
	row.active = (not material.use_ray_shadow_bias)
	row.prop(material, "shadow_ray_bias", text="Ray Bias")

def shader_sss(layout, material):
	sss = material.subsurface_scattering
	box = layout.box()
	box.prop(sss, "use", text="Subsurface Scattering")

	if sss.use:
		row = box.row().split()
		sub = row.row(align=True).split(align=True, percentage=0.75)
		sub.menu("MATERIAL_MT_sss_presets", text=bpy.types.MATERIAL_MT_sss_presets.bl_label)
		sub.operator("material.sss_preset_add", text="", icon='ZOOMIN')
		sub.operator("material.sss_preset_add", text="", icon='ZOOMOUT').remove_active = True

		split = box.split()

		col = split.column()
		col.prop(sss, "ior")
		col.prop(sss, "scale")
		col.prop(sss, "color", text="")
		col.prop(sss, "radius", text="RGB Radius", expand=True)

		col = split.column()
		sub = col.column(align=True)
		sub.label(text="Blend:")
		sub.prop(sss, "color_factor", text="Color")
		sub.prop(sss, "texture_factor", text="Texture")
		sub.label(text="Scattering Weight:")
		sub.prop(sss, "front")
		sub.prop(sss, "back")
		col.separator()
		col.prop(sss, "error_threshold", text="Error")

def shader_transparency(layout, material):
	box = layout.box()
	box.prop(material, "use_transparency", text="Transparency")
	if material.use_transparency:
		rt_transparency = material.raytrace_transparency
		row = box.row()
		row.active = material.use_transparency
		row.prop(material, "transparency_method", expand=True)

		split = box.split()
		split.active = material.use_transparency

		col = split.column()
		col.prop(material, "alpha")
		row = col.row()
		row.active = (material.transparency_method != 'MASK') and (not material.use_shadeless)
		row.prop(material, "specular_alpha", text="Specular")

		col = split.column()
		col.active = (not material.use_shadeless)
		col.prop(rt_transparency, "fresnel")
		sub = col.column()
		sub.active = (rt_transparency.fresnel > 0.0)
		sub.prop(rt_transparency, "fresnel_factor", text="Blend")

		if material.transparency_method == 'RAYTRACE':
			box.separator()
			split = box.split()
			split.active = material.use_transparency

			col = split.column()
			col.prop(rt_transparency, "ior")
			col.prop(rt_transparency, "filter")
			col.prop(rt_transparency, "falloff")
			col.prop(rt_transparency, "depth_max")
			col.prop(rt_transparency, "depth")

			col = split.column()
			col.label(text="Gloss:")
			col.prop(rt_transparency, "gloss_factor", text="Amount")
			sub = col.column()
			sub.active = rt_transparency.gloss_factor < 1.0
			sub.prop(rt_transparency, "gloss_threshold", text="Threshold")
			sub.prop(rt_transparency, "gloss_samples", text="Samples")

def shader_reflection(layout, material):
	refl_mat = material.raytrace_mirror
	box = layout.box()
	box.prop(refl_mat, "use", text="Reflection")

	if refl_mat.use:
		box.active = refl_mat.use

		split = box.split()

		col = split.column()
		col.prop(refl_mat, "reflect_factor")
		col.prop(material, "mirror_color", text="")

		col = split.column()
		col.prop(refl_mat, "fresnel")
		sub = col.column()
		sub.active = (refl_mat.fresnel > 0.0)
		sub.prop(refl_mat, "fresnel_factor", text="Blend")

		split = box.split()

		col = split.column()
		col.separator()
		col.prop(refl_mat, "depth")
		col.prop(refl_mat, "distance", text="Max Dist")
		col.separator()
		sub = col.split(percentage=0.4)
		sub.active = (refl_mat.distance > 0.0)
		sub.label(text="Fade To:")
		sub.prop(refl_mat, "fade_to", text="")

		col = split.column()
		col.label(text="Gloss:")
		col.prop(refl_mat, "gloss_factor", text="Amount")
		sub = col.column()
		sub.active = (refl_mat.gloss_factor < 1.0)
		sub.prop(refl_mat, "gloss_threshold", text="Threshold")
		sub.prop(refl_mat, "gloss_samples", text="Samples")
		sub.prop(refl_mat, "gloss_anisotropic", text="Anisotropic")

def material_panel(layout, context):
	material = context.active_object.active_material

	ob = context.active_object
	is_sortable = (len(ob.material_slots) > 1)
	rows = 1
	if is_sortable:
		rows = 4
	layout.template_list("MATERIAL_UL_matslots", "", ob, "material_slots", ob, "active_material_index", rows=rows)
	layout.template_ID(ob, "active_material", new="material.new")

	if material!= None:
		scene = context.scene
		ob = context.active_object

		diff_texture_slot = material.texture_slots[0]
		spec_texture_slot = material.texture_slots[1]
		nrml_texture_slot = material.texture_slots[2]
		refl_texture_slot = material.texture_slots[3]
		trns_texture_slot = material.texture_slots[4]

		shader_property_layout(context, layout, material)

		##########################################################
		# DIFFUSE PORT
		##########################################################
		split = layout.split(percentage=0.1)
		row = split.row(align=True)
		row.prop(material, "diffuse_color", text="")
		row = split.row(align=True)
		row.prop(material, "diffuse_shader", text="")
		row.prop(material, "diffuse_intensity", text="Intensity")
		col = layout.column(align=True)
		row = col.row(align=True)
		row.operator("wm.bsi_decorator_button", text="DIFFUSE PORT", emboss=False)

		row.prop( material.BSI, "diff_image", text="",
				  icon=('TEXTURE'
								if material.BSI.diff_image
								else 'IMASEL' ),
				  emboss=False)

		row.prop( material.BSI, "diff_uvs", text="",
				  icon_value=(ico["uv_conn"].icon_id
								if material.BSI.diff_uvs
								else ico["uv"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "diff_sampling", text="",
				  icon_value=(ico["sampling_on"].icon_id
								if material.BSI.diff_sampling
								else ico["sampling_off"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "diff_influence", text="",
				  icon_value=(ico["influences_on"].icon_id
								if material.BSI.diff_influence
								else ico["influences_off"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "diff_ramp", text="",
				  icon_value=(ico["ramp_on"].icon_id
								if material.BSI.diff_ramp
								else ico["ramp_off"].icon_id ),
				  emboss=False)

		if material.BSI.diff_image:
			if diff_texture_slot  != None:
				col = layout.column(align=True)
				row = col.row(align=True)

				diff_port = row.operator("bsi.apply_texture_image", text="Re-Apply New Diffuse Image")
				diff_port.mat_port_type = "diffuse"

				prop = row.operator("bsi.clear_texture_slot" ,text="",icon = 'X', emboss=True)
				prop.port_type= "diffuse"

				row = col.row(align=True)
				row.label(text="") #spacer align to the right
				row.prop(material.BSI, "diff_img_display", text="",
						  icon=('VISIBLE_IPO_ON'
										if material.BSI.diff_img_display
										else 'VISIBLE_IPO_OFF'),
						  emboss=False)
				if material.BSI.diff_img_display:
					box = layout.box()
					box.label(text="UGLY BLENDER DESIGN---> CAN'T FIX")
					box.template_image(diff_texture_slot.texture, "image", diff_texture_slot.texture.image_user)
			else:
				diff_port = layout.operator("bsi.apply_texture_image", text="Load Diffuse Image")
				diff_port.mat_port_type = "diffuse"

		if material.BSI.diff_uvs:
			if diff_texture_slot  != None:
				uv_projection_layout(layout, diff_texture_slot, ob)
				uv_tiling_layout(layout, diff_texture_slot.texture)

			else:
				layout.label(text="No Texture Connection")

		if material.BSI.diff_sampling:
			if diff_texture_slot  != None:
				texture_sampling_layout(layout, diff_texture_slot)
			else:
				layout.label(text="No Texture Connection")

		if material.BSI.diff_influence:
			if diff_texture_slot  != None:
				texture_influence_layout(layout, diff_texture_slot, port_type="diffuse")
			else:
				layout.label(text="No Texture Connection")

		if material.BSI.diff_ramp:
			shader_ramp_layout(layout, material)
		##########################################################
		# SPECULAR PORT
		##########################################################
		split = layout.split(percentage=0.1)
		row = split.row(align=True)
		row.prop(material, "specular_color", text="")
		row = split.row(align=True)
		row.prop(material, "specular_shader", text="")
		row.prop(material, "specular_intensity", text="Intensity")

		col = layout.column(align=True)
		row = col.row(align=True)
		row.operator("wm.bsi_decorator_button", text="SPECULAR PORT", emboss=False)

		row.prop( material.BSI, "spec_image", text="",
				  icon=('TEXTURE'
								if material.BSI.spec_image
								else 'IMASEL' ),
				  emboss=False)
		row.prop( material.BSI, "spec_uvs", text="",
				  icon_value=(ico["uv_conn"].icon_id
								if material.BSI.spec_uvs
								else ico["uv"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "spec_sampling", text="",
				  icon_value=(ico["sampling_on"].icon_id
								if material.BSI.spec_sampling
								else ico["sampling_off"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "spec_influence", text="",
				  icon_value=(ico["influences_on"].icon_id
								if material.BSI.spec_influence
								else ico["influences_off"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "spec_ramp", text="",
				  icon_value=(ico["ramp_on"].icon_id
								if material.BSI.spec_ramp
								else ico["ramp_off"].icon_id ),
				  emboss=False)

		if material.BSI.spec_image:
			if spec_texture_slot  != None:
				col = layout.column(align=True)
				row = col.row(align=True)

				spec_port = row.operator("bsi.apply_texture_image", text="Re-Apply New Specular Image")
				spec_port.mat_port_type = "specular"

				prop = row.operator("bsi.clear_texture_slot" ,text="",icon = 'X', emboss=True)
				prop.port_type= "specular"

				row = col.row(align=True)
				row.label(text="") #spacer align to the right
				row.prop(material.BSI, "spec_img_display", text="",
						  icon=('VISIBLE_IPO_ON'
										if material.BSI.spec_img_display
										else 'VISIBLE_IPO_OFF'),
						  emboss=False)
				if material.BSI.spec_img_display:
					box = layout.box()
					box.label(text="UGLY BLENDER DESIGN---> CAN'T FIX")
					box.template_image(spec_texture_slot.texture, "image", spec_texture_slot.texture.image_user)
			else:
				spec_port = layout.operator("bsi.apply_texture_image", text="Load Specular Image")
				spec_port.mat_port_type = "specular"

		if material.BSI.spec_uvs:
			if spec_texture_slot  != None:
				uv_projection_layout(layout, spec_texture_slot, ob)
				uv_tiling_layout(layout, spec_texture_slot.texture)

			else:
				layout.label(text="No Texture Connection")

		if material.BSI.spec_sampling:
			if spec_texture_slot  != None:
				texture_sampling_layout(layout, spec_texture_slot)
			else:
				layout.label(text="No Texture Connection")

		if material.BSI.spec_influence:
			if spec_texture_slot  != None:
				texture_influence_layout(layout, spec_texture_slot, port_type="specular")
			else:
				layout.label(text="No Texture Connection")

		if material.BSI.spec_ramp:
			shader_ramp_layout(layout, material)
		##########################################################
		# NORMAL - BUMP PORT
		##########################################################
		split = layout.split(percentage=0.1)
		row = split.row(align=True)
		row.prop(material, "diffuse_color", text="")
		row = split.row(align=True)
		row.prop(material, "diffuse_shader", text="")
		row.prop(material, "diffuse_intensity", text="Intensity")
		col = layout.column(align=True)
		row = col.row(align=True)
		row.operator("wm.bsi_decorator_button", text="BUMP PORT", emboss=False)

		row.prop( material.BSI, "spec_image", text="",
				  icon=('TEXTURE'
								if material.BSI.spec_image
								else 'IMASEL' ),
				  emboss=False)
		row.prop( material.BSI, "spec_uvs", text="",
				  icon_value=(ico["uv_conn"].icon_id
								if material.BSI.spec_uvs
								else ico["uv"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "spec_sampling", text="",
				  icon_value=(ico["sampling_on"].icon_id
								if material.BSI.spec_sampling
								else ico["sampling_off"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "spec_influence", text="",
				  icon_value=(ico["influences_on"].icon_id
								if material.BSI.spec_influence
								else ico["influences_off"].icon_id ),
				  emboss=False)

		row.prop( material.BSI, "spec_ramp", text="",
				  icon_value=(ico["ramp_on"].icon_id
								if material.BSI.spec_ramp
								else ico["ramp_off"].icon_id ),
				  emboss=False)

# def bl_shading_panel(layout):
# 	context = bpy.context
# 	view = context.space_data
# 	scene = context.scene
# 	gs = scene.game_settings
# 	obj = context.object

# 	col = layout.column()

# 	if not scene.render.use_shading_nodes:
# 		col.prop(gs, "material_mode", text="")

# 	if view.viewport_shade == 'SOLID':
# 		col.prop(view, "show_textured_solid")
# 		col.prop(view, "use_matcap")
# 		if view.use_matcap:
# 			col.template_icon_view(view, "matcap_icon")
# 	elif view.viewport_shade == 'TEXTURED':
# 		if scene.render.use_shading_nodes or gs.material_mode != 'GLSL':
# 			col.prop(view, "show_textured_shadeless")

# 	col.prop(view, "show_backface_culling")

# 	if view.viewport_shade not in {'BOUNDBOX', 'WIREFRAME'}:
# 		if obj and obj.mode == 'EDIT':
# 			col.prop(view, "show_occlude_wire")

# 	fx_settings = view.fx_settings

# 	if view.viewport_shade not in {'BOUNDBOX', 'WIREFRAME'}:
# 		sub = col.column()
# 		sub.active = view.region_3d.view_perspective == 'CAMERA'
# 		sub.prop(fx_settings, "use_dof")
# 		col.prop(fx_settings, "use_ssao", text="Ambient Occlusion")
# 		if fx_settings.use_ssao:
# 			ssao_settings = fx_settings.ssao
# 			subcol = col.column(align=True)
# 			subcol.prop(ssao_settings, "factor")
# 			subcol.prop(ssao_settings, "distance_max")
# 			subcol.prop(ssao_settings, "attenuation")
# 			subcol.prop(ssao_settings, "samples")
# 			subcol.prop(ssao_settings, "color")

