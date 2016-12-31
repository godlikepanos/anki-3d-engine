# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE
# keep in alphabetical order

import bpy
from bpy.types import Operator

class ANKI_TEXT_reload_run(Operator):
    """ANKI reload and run text"""
    bl_idname = "anki.reload_build"
    bl_label = "Reload Run Text"
    bl_options = {'REGISTER'}

    def execute(self, context):
        bpy.ops.text.reload()
        bpy.ops.text.run_script()
        return {'FINISHED'}