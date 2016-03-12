
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

import os
import subprocess
import bpy
import mathutils
from bpy.types import Operator, AddonPreferences,Panel
from bpy.props import StringProperty, IntProperty, BoolProperty
from bpy.props import *
# Constants

ENVIRO = "ANKI_DATA_PATH"

bpy.types.Scene.UseViewport = BoolProperty(
    name = "View", 
    description = "True or False?",
    default= True)

bpy.types.Scene.ExportAllTextures = BoolProperty(
    name = "Export All Textures", 
    description = "True or False?",
    default= False)

bpy.types.Scene.UpdateTextures = BoolProperty(
    name = "Update Textures", 
    description = "True or False?",
    default= True)

# scn['UseViewport'] = True
# scn['ExportAllTextures'] = False 
# scn['UpdateTextures'] = True


def set_environment(anki_env_dct,tools_dct):
    environment_path = ':'.join(anki_env_dct.values())
    os.environ[ENVIRO]=environment_path
    environment_path = ':'.join(tools_dct.values())
    os.environ['ANKI_TOOLS']=environment_path

def get_environment(environment_root):
    environment_root = (os.path.abspath(environment_root)) #clean the path
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
    tool_etcpack_path     = "{0}/thirdparty/bin/etcpack".format(environment_root) 
    tools_path            = "{0}/tools".format(anki_build_path) 
    tools_scene_path      = "{0}/scene".format(tools_path) 
    tools_texture_path    = "{0}/texture".format(tools_path) 
    
    # Make the Export Paths
    for _path in [export_src_data, export_map_path, export_texture_path]:
        if not os.path.exists(_path):
            print ("Making directory:", _path)
            os.makedirs(_path)
    
    env_dct = {     
                    'environment_root':environment_root+'/',
                    'anki_build_path':anki_build_path,
                    'anki_engine_data_path':anki_engine_data_path,
                    'anki_shaders_path':anki_shaders_path,
                    }
    tools_dct = {     
                    'tool_etcpack_path':tool_etcpack_path,
                    'tools_path':tools_path,
                    'tools_scene_path':tools_scene_path,
                    'tools_texture_path':tools_texture_path,
                    }

    export_dct = {
                    'export_src_data':export_src_data,
                    'export_map_path':export_map_path,
                    'export_texture_path':export_texture_path,
                    'export_temp_dea':export_temp_dea,
                    }

    return env_dct, export_dct, tools_dct

def get_images():
    mats = bpy.data.materials
    for mat in mats:
        for slot in mat.texture_slots:
            if slot:
                if (slot.texture.image != None):
                    print(slot)
                    print(slot.texture.image.file_format)
                    path = slot.texture.image.filepath

class AnkiSceneExporter(AddonPreferences):
    # this must match the addon name, use '__package__'
    # when defining this in a submodule of a python package.
    bl_idname = __name__


    anki_environment = StringProperty(
            name="Anki Environment",
            default="ANKI_PATH:",
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
    path_list = os.getenv(ENVIRO)
    environment_root = None
    if path_list:
        # Environment variable exists get the common path.
        path_list = os.getenv(ENVIRO).split(":")
        environment_root = os.path.commonprefix(path_list)
        anki_project_path[1]['default'] = environment_root

        env_dct, export_dct, tools_dct = get_environment(environment_root)
        set_environment(env_dct, tools_dct)
        
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
        msg = "Preferences are driven by {} environment variable".format(ENVIRO)
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
    
        # layout.label(text=self.anki_environment)
        layout.operator("scene.anki_preference_set", text="Set Preferences")

class OBJECT_OT_anki_preference_set(Operator):
    """Set anki preferences"""
    bl_idname = "scene.anki_preference_set"
    bl_label = "Anki Preferences Setter"
    bl_options = {'REGISTER', 'UNDO'}
    # anki_path_str = "ANKI_PATH"
    def execute(self, context):
        user_preferences = context.user_preferences
        addon_prefs = user_preferences.addons[__name__].preferences
        
        # Evironment variables
        build = "{0}build".format(addon_prefs.anki_project_path)
        data = "{0}data".format(addon_prefs.anki_project_path)
        shaders = "{0}shaders".format(addon_prefs.anki_project_path)
        engine_data = "{0}engine_data".format(addon_prefs.anki_project_path)

        # Set project paths
        export_path ="{0}/src_data".format(data) 
        addon_prefs.export_path = export_path
        
        map_path ="{0}/map".format(data) 
        addon_prefs.map_path = map_path

        texture_path ="{0}/texture".format(data) 
        addon_prefs.texture_path = texture_path
        
        tools_path ="{0}/tools".format(build) 
        addon_prefs.tools_path = tools_path

        tools_scene_path ="{0}/scene".format(tools_path) 
        tools_texture_path ="{0}/texture".format(tools_path) 

        # Environment variable setter
        # environment_path = (":".join([build, data, shaders, engine_data, tools_path, tools_scene_path, tools_texture_path]))
        # os.environ[self.anki_path_str]=environment_path
        # # os.environ['ANKI_DATA_PATH']=environment_path
        # addon_prefs.anki_environment = "ANKI_PATH: {}".format(environment_path)

        # # Make the paths
        # for _path in [export_path,map_path, texture_path]:
        #     if not os.path.exists(_path):
        #         print ("Making directory:", _path)
        #         os.makedirs(_path)
        # addon_prefs.temp_dea = "{0}/tmp_scene.dae".format(export_path)
        # print (addon_prefs.temp_dea)

        return {'FINISHED'}

def get_camera():
    def getter():
        obs = bpy.data.objects
        for ob in obs:
            if ob.type == 'CAMERA':
                cam = ob
                return ob
    cam = getter()        
    if cam == None:
        bpy.ops.object.camera_add()
    return getter()

def get_region3d():
    for view in bpy.context.window.screen.areas:
        if view.type == 'VIEW_3D':
            return view.spaces[0].region_3d
    return None
            #print(dir(a))
            #cam.matrix_world = a.spaces[0].region_3d.view_matrix.inverted()

class OBJECT_OT_anki_export(Operator):
    """Export anki scene"""
    bl_idname = "scene.anki_export"
    bl_label = "Anki Export"
    bl_options = {'REGISTER'}

    def execute(self, context):
        scn = context.scene
        user_preferences = context.user_preferences
        addon_prefs = user_preferences.addons[__name__].preferences

        temp_dea =  addon_prefs.temp_dea
        
        view_mtx = None
        cam = None
        if scn['UseViewport']:
            # Get camera matrix to viewport projection matrix
            cam = get_camera()
            view_mtx = mathutils.Matrix(cam.matrix_world)
            r3d = get_region3d()
            cam.matrix_world = r3d.view_matrix.inverted()
            print ("Set camera to view projection matrix.")
        
        # Export the collada file to temp.dea    
        bpy.ops.wm.collada_export(filepath=temp_dea,
                          check_existing=True,
                          filter_blender=False,
                          filter_backup=False,
                          filter_image=False,
                          filter_movie=False,
                          filter_python=False,
                          filter_font=False,
                          filter_sound=False,
                          filter_text=False,
                          filter_btx=False,
                          filter_collada=True,
                          filter_folder=True,
                          filter_blenlib=False,
                          filemode=8,
                          display_type='DEFAULT',
                          sort_method='FILE_SORT_ALPHA',
                          apply_modifiers=False,
                          export_mesh_type=0,
                          export_mesh_type_selection='view',
                          selected=False,
                          include_children=False,
                          include_armatures=False,
                          include_shapekeys=True,
                          deform_bones_only=False,
                          active_uv_only=False,
                          include_uv_textures=False,
                          include_material_textures=False,
                          use_texture_copies=True,
                          triangulate=True,
                          use_object_instantiation=True,
                          sort_by_name=False,
                          export_transformation_type=0,
                          export_transformation_type_selection='matrix',
                          open_sim=False)
        print ("Exported to:", temp_dea)

        # Restore Camera
        if scn['UseViewport']:
            cam.matrix_world = view_mtx
            print ("Restored camera to original projection matrix.")
            

        relative_path = (addon_prefs.map_path.replace(addon_prefs.anki_project_path, ""))
        cmd  = "ankisceneimp {0} {1} -rpath {2}".format(temp_dea, 
                                                        addon_prefs.map_path, 
                                                        relative_path)

        # Run the level in the game
        subprocess.call(cmd, shell=True)
        test_app = ("{0}build/testapp/testapp".format(addon_prefs.anki_project_path))
        subprocess.call(test_app, shell=True)

        return {'FINISHED'}

class AnkiPanel:
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'


class VIEW3D_PT_prefs_anki(AnkiPanel, Panel):
    bl_category = "Anki"
    bl_label = "Preferences"

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
        col.prop(scn, 'UseViewport')
        col = split.column()
        col.operator("scene.anki_export", icon='PLAY')

        # layout.prop(scn, 'MyInt', icon='BLENDER', toggle=True)
        # col.prop(scn, 'MyFloat')
        # layout.prop(scn, 'MyEnum')
        # layout.prop(scn, 'MyString')
        # layout.operator("idname_must.be_all_lowercase_and_contain_one_dot")
        #col.prop(cam, "show_limits", text="Limits")

# Registration
def register():
    bpy.utils.register_class(OBJECT_OT_anki_preference_set)
    bpy.utils.register_class(OBJECT_OT_anki_export)
    bpy.utils.register_class(AnkiSceneExporter)
    bpy.utils.register_class(VIEW3D_PT_prefs_anki)
    bpy.utils.register_class(VIEW3D_PT_tools_anki)


def unregister():
    bpy.utils.unregister_class(OBJECT_OT_anki_preference_set)
    bpy.utils.unregister_class(OBJECT_OT_anki_export)
    bpy.utils.unregister_class(AnkiSceneExporter)
    bpy.utils.unregister_class(VIEW3D_PT_prefs_anki)
    bpy.utils.unregister_class(VIEW3D_PT_tools_anki)



if __name__ == "__main__":
    register()
# import bpy  
# import mathutils
# import json
    
# cam = bpy.context.scene.camera
# cam.rotation_mode = 'XYZ'


# rot = list()
# pos = list()
# print (list(pos))
# for area in bpy.context.screen.areas:
#     if area.type == 'VIEW_3D':
#         region = area
#         for space in area.spaces:
#             if space.type == 'VIEW_3D':
#                 rv3d = space.region_3d
#                 mat = rv3d.view_matrix
#                 pos = list(mat.to_translation())
#                 rot = list(mat.to_euler())

# rot = list(cam.rotation_euler)
# pos = list(cam.location)
# d = {"camera_pos":pos, "camera_rot":rot}
# with open('/home/akal/hacking/anki/data/map/camera.json', 'w') as fp:
#     json.dump(d, fp, indent=4)