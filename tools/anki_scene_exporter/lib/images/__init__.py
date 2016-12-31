# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

#local imports
import os

root = (os.path.dirname(os.path.realpath(__file__)))
files = os.listdir(root)
d_lib_images = dict([(ff[:-4], os.sep.join([root, ff])) for ff in files if ff.endswith(".png")])
