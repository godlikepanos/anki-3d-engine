#!/usr/bin/python

import optparse
import subprocess
import re
import os
import struct
import copy
import tempfile
import shutil

#
# AnKi texture
#

# Texture type
TT_NONE = 0
TT_2D = 1
TT_CUBE = 2
TT_3D = 3
TT_2D_ARRAY = 4

# Color format
CF_NONE = 0
CF_RGB8 = 1
CF_RGBA8 = 2

# Data compression
DC_NONE = 0
DC_RAW = 1 << 0
DC_ETC2 = 1 << 1
DC_S3TC = 1 << 2

#
# DDS
#

# dwFlags of DDSURFACEDESC2
DDSD_CAPS           = 0x00000001
DDSD_HEIGHT         = 0x00000002
DDSD_WIDTH          = 0x00000004
DDSD_PITCH          = 0x00000008
DDSD_PIXELFORMAT    = 0x00001000
DDSD_MIPMAPCOUNT    = 0x00020000
DDSD_LINEARSIZE     = 0x00080000
DDSD_DEPTH          = 0x00800000

# ddpfPixelFormat of DDSURFACEDESC2
DDPF_ALPHAPIXELS    = 0x00000001
DDPF_FOURCC         = 0x00000004
DDPF_RGB            = 0x00000040

# dwCaps1 of DDSCAPS2
DDSCAPS_COMPLEX     = 0x00000008
DDSCAPS_TEXTURE     = 0x00001000
DDSCAPS_MIPMAP      = 0x00400000

# dwCaps2 of DDSCAPS2
DDSCAPS2_CUBEMAP            = 0x00000200
DDSCAPS2_CUBEMAP_POSITIVEX  = 0x00000400
DDSCAPS2_CUBEMAP_NEGATIVEX  = 0x00000800
DDSCAPS2_CUBEMAP_POSITIVEY  = 0x00001000
DDSCAPS2_CUBEMAP_NEGATIVEY  = 0x00002000
DDSCAPS2_CUBEMAP_POSITIVEZ  = 0x00004000
DDSCAPS2_CUBEMAP_NEGATIVEZ  = 0x00008000
DDSCAPS2_VOLUME             = 0x00200000

class DdsHeader:
	""" The header of a dds file """

	_fields = [
		('dwMagic', '4s'),
		('dwSize', 'I'),
		('dwFlags', 'I'),
		('dwHeight', 'I'),
		('dwWidth', 'I'),
		('dwPitchOrLinearSize', 'I'),
		('dwDepth', 'I'),
		('dwMipMapCount', 'I'),
		('dwReserved1', '44s'),
	
		# Pixel format
		('dwSize', 'I'),
		('dwFlags', 'I'),
		('dwFourCC', '4s'),
		('dwRGBBitCount', 'I'),
		('dwRBitMask', 'I'),
		('dwGBitMask', 'I'),
		('dwBBitMask', 'I'),
		('dwRGBAlphaBitMask', 'I'),

		('dwCaps1', 'I'),
		('dwCaps2', 'I'),
		('dwCapsReserved', '8s'),
		('dwReserved2', 'I')]

	def __init__(self, buff):
		buff_format = self.get_format()
		items = struct.unpack(buff_format, buff)
		for field, value in map(None, self._fields, items):
			setattr(self, field[0], value)

	@classmethod
	def get_format(cls):
		return '<' + ''.join([f[1] for f in cls._fields])

	@classmethod
	def get_size(cls):
		return struct.calcsize(cls.get_format())

#
# ETC2
#
	
class PkmHeader:
	""" The header of a pkm file """
	
	_fields = [
		("magic", "6s"),
		("type", "H"),
		("width", "H"),
		("height", "H"),
		("origWidth", "H"),
		("origHeight", "H")]

	def __init__(self, buff):
		buff_format = self.get_format()
		items = struct.unpack(buff_format, buff)
		for field, value in map(None, self._fields, items):
			setattr(self, field[0], value)

	@classmethod
	def get_format(cls):
		return ">" + "".join([f[1] for f in cls._fields])

	@classmethod
	def get_size(cls):
		return struct.calcsize(cls.get_format())
	
#
# Functions
# 

def is_power2(num):
	""" Returns true if a number is a power of two """
	return num != 0 and ((num & (num - 1)) == 0)

def get_base_fname(path):
	""" From path/to/a/file.ext return the "file" """
	return os.path.splitext(os.path.basename(path))[0]

def parse_commandline():
	""" Parse the command line arguments """

	parser = optparse.OptionParser("usage: %prog [options]")

	parser.add_option("-i", "--input", dest = "inp",
			type = "string", help = "specify the image(s) to convert. " \
			"Seperate with :")

	parser.add_option("-o", "--output", dest = "out",
			type = "string", help = "specify new image. ")

	parser.add_option("-t", "--type", dest = "type",
			type = "string", default = "2D", 
			help = "type of the image (2D or cube or 3D or 2DArray)")

	parser.add_option("-f", "--fast", dest = "fast",
			action = "store_true", default = False, 
			help = "run the fast version of the converters")

	parser.add_option("-n", "--normal", dest = "normal",
			action = "store_true", default = False, 
			help = "assume the texture is normal")

	(options, args) = parser.parse_args()

	if not options.inp or not options.out:
		parser.error("argument is missing")

	if options.type == "2D":
		typ = TT_2D
	elif options.type == "cube":
		typ = TT_CUBE
	elif options.type == "3D":
		typ = TT_3D
	elif options.type == "2DArray":
		typ = TT_2D_ARRAY
	else:
		parser.error("Unrecognized type: " + options.type)

	return (options.inp.split(":"), options.out, options.fast,
			typ, options.normal)

def get_internal_format_and_size(in_file):
	""" Return the size of the input image and the internal format """

	internal_format = CF_NONE
	width = 0
	height = 0

	proc = subprocess.Popen(["identify", "-verbose" , in_file],
			stdout=subprocess.PIPE)

	stdout_str = proc.stdout.read()

	reg = re.search(r"Colorspace: (.*)", stdout_str)
	if not reg:
		raise Exception("Cannot extract colorspace")

	print("-- Colorspace: %s" % reg.group(1))
	if reg.group(1) == "RGB":
		internal_format = CF_RGB8
	elif reg.group(1) == "RGBA":
		internal_format = CF_RGBA8
	else:
		raise Exception("Unrecognized color format")

	reg = re.search(r"Geometry: ([0-9]*)x([0-9]*)\+", stdout_str)

	if not reg:
		raise Exception("Cannot extract size")

	print("-- width: %s, height: %s" % (reg.group(1), reg.group(2)))

	return (internal_format, int(reg.group(1)), int(reg.group(2)))

def create_mipmaps(in_file, tmp_dir, orig_size, internal_format):
	""" Create a number of images for all mipmaps """

	print("-- Generate mipmaps")

	size = orig_size

	mips_fnames = []

	while size >= 4:
		size_str = "%dx%d" % (size, size)
		out_file_str = os.path.join(tmp_dir, get_base_fname(in_file)) \
				+ "." + str(size)

		print("  -- %s.(tga & ppm)" % out_file_str)

		mips_fnames.append(out_file_str)

		args = ["convert", in_file, "-resize", size_str, "-alpha"]

		if internal_format == CF_RGB8:
			args.append("deactivate")
		else:
			args.append("activate")

		# ppm
		args_ppm = copy.deepcopy(args)
		args_ppm.append(out_file_str + ".ppm")
		subprocess.check_call(args_ppm)

		# png
		args_tga = copy.deepcopy(args)
		args_tga.append(out_file_str + ".tga")
		subprocess.check_call(args_tga)

		size = size / 2

	return mips_fnames

def create_etc_images(mips_fnames, tmp_dir, fast, internal_format):
	""" Create the etc files """

	print("-- Creating ETC images")

	for fname in mips_fnames:
		fname = fname + ".ppm"

		print("  -- %s" % fname)

		args = ["etcpack", fname, tmp_dir, "-c", "etc2"]

		if fast:
			args.append("-s")
			args.append("fast")

		args.append("-f")
		if internal_format == CF_RGB8:
			args.append("RGB")
		else:
			args.append("RGBA")

		subprocess.check_call(args, stdout = subprocess.PIPE)

def create_dds_images(mips_fnames, tmp_dir, fast, internal_format, \
		normal):
	""" Create the dds files """

	print("-- Creating DDS images")

	for fname in mips_fnames:
		# Unfortunately we need to flip the image. Use convert again
		in_fname = fname + ".tga"
		flipped_fname = fname + "_flip.tga"
		args = ["convert", in_fname, "-flip", flipped_fname]
		subprocess.check_call(args)
		in_fname = flipped_fname

		# Continue
		out_fname = os.path.join(tmp_dir, os.path.basename(fname) + ".dds")

		print("  -- %s" % out_fname)

		args = ["nvcompress", "-silent", "-nomips"]

		if fast:
			args.append("-fast")

		if internal_format == CF_RGB8:
			if not normal:
				args.append("-bc1")
			else:
				args.append("-bc1n")
		elif internal_format == CF_RGBA8:
			if not normal:
				args.append("-bc3")
			else:
				args.append("-bc3n")

		args.append(in_fname)
		args.append(out_fname)

		subprocess.check_call(args, stdout = subprocess.PIPE)

def write_raw(tex_file, fname, width, height, internal_format):
	""" Append raw data to the AnKi texture file """

	# Read and check the header
	uncompressed_tga_header = struct.pack("BBBBBBBBBBBB", \
			0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0)

	in_file = open(fname, "rb")
	tga_header = in_file.read(12)
	
	if uncompressed_tga_header != tga_header:
		raise Exception("Incorrect TGA header")

	# Read the size and bpp
	header6_buff = in_file.read(6)
	header6 = struct.unpack("BBBBBB", header6_buff)

	img_width = header6[1] * 256 + header6[0]
	img_height = header6[3] * 256 + header6[2]
	img_bpp = header6[4];

	if (internal_format != CF_RGB8 or img_bpp != 24) \
			and (internal_format != CF_RGBA8 or img_bpp != 32):
		raise Exception("Unexpected bpp")
		
	if img_width != width or img_height != height:
		raise Exception("Unexpected width or height")

	# Read the data
	data_size = width * height
	if internal_format == CF_RGB8:
		data_size *= 3
	else:
		data_size *= 4

	data = bytearray(in_file.read(data_size))

	if len(data) != data_size:
		raise Exception("Failed to read all data")

	# Swap colors
	bpp = img_bpp / 8
	for i in xrange(0, data_size, bpp):
		temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;

	# Write data to tex_file
	tex_file.write(data)

def write_s3tc(out_file, fname, width, height, internal_format):
	""" Append s3tc data to the AnKi texture file """

	# Read header
	print("  -- Appending %s" % fname)
	in_file = open(fname, "rb")

	dds_header = DdsHeader(in_file.read(DdsHeader.get_size()))

	if dds_header.dwWidth != width or dds_header.dwHeight != height:
		raise Exception("Incorrect width")

	if internal_format == CF_RGB8 \
			and dds_header.dwFourCC != "DXT1":
		raise Exception("Incorrect format. Expecting DXT1")

	if internal_format == CF_RGBA8 \
			and dds_header.dwFourCC != "DXT5":
		raise Exception("Incorrect format. Expecting DXT5")

	# Read and write the data
	if internal_format == CF_RGB8:
		block_size = 8
	else:
		block_size = 16

	data_size = ((width + 3) / 4) * ((height + 3) / 4) * block_size

	data = in_file.read(data_size)
	out_file.write(data)

def write_etc(out_file, fname, width, height, internal_format):
	""" Append etc2 data to the AnKi texture file """
	
	# Read header
	in_file = open(fname, "rb")

	pkm_header = PkmHeader(in_file.read(PkmHeader.get_size()))

	if pkm_header.magic != "PKM 20":
		raise Exception("Incorrect PKM header")

	if width != pkm_header.width or height != pkm_header.height:
		raise Exception("Incorrect PKM width or height")

	# Read and write the data
	data_size = (pkm_header.width / 4) * (pkm_header.height / 4) * 8

	data = in_file.read(data_size)
	out_file.write(data)

def convert(in_files, out, fast, typ, normal, tmp_dir):
	""" This is the function that does all the work """

	# Invoke app named "identify" to get internal format and width and height
	(internal_format, width, height) = get_internal_format_and_size(in_files[0])

	if width != height:
		raise Exception("Only square images are accepted")

	if not is_power2(width):
		raise Exception("Image should power of 2")

	if internal_format == CF_RGBA8 and normal:
		raise Exception("RGBA image and normal does not make much sense")

	for i in range(1, len(in_files)):
		(internal_format_2, width_2, height_2) = \
				get_internal_format_and_size(in_files[i])

		if width != width_2 or height != height_2 \
				or internal_format != internal_format_2:
			raise Exception("Images are not same size and color space")

	# Create images
	for in_file in in_files:
		mips_fnames = create_mipmaps(in_file, tmp_dir, width, internal_format)

		# Create etc images
		create_etc_images(mips_fnames, tmp_dir, fast, internal_format)

		# Create dds images
		create_dds_images(mips_fnames, tmp_dir, fast, internal_format, normal)

	# Open file
	fname = out
	print("-- Writing %s" % fname)
	tex_file = open(fname, "wb")

	# Write header
	ak_format = "8sIIIIIIII"

	buff = struct.pack(ak_format, 
			b"ANKITEX1",
			width,
			height,
			len(in_files),
			typ,
			internal_format,
			DC_RAW | DC_S3TC | DC_ETC2,
			normal,
			len(mips_fnames))

	tex_file.write(buff)

	# Write header padding
	header_padding_size = 128 - struct.calcsize(ak_format)

	if header_padding_size != 88:
		raise Exception("Check the header")

	for i in range(0, header_padding_size):
		tex_file.write('\0')

	# For each compression
	for compression in range(0, 3):
		# For each image
		for in_file in in_files:
			size = width

			# For each level
			while size >= 4:
				in_base_fname = os.path.join(tmp_dir, get_base_fname(in_file)) \
					+ "." + str(size)

				# Write RAW
				if compression == 0:
					write_raw(tex_file, in_base_fname + ".tga", size, size, \
							internal_format)

				# Write S3TC
				if compression == 1:
					write_s3tc(tex_file, in_base_fname + ".dds", size, size, \
							internal_format)

				# Write ETC
				if compression == 2:
					write_etc(tex_file, in_base_fname + ".pkm", size, size, \
							internal_format)
				
				size = size / 2

def main():
	""" The main """

	# Parse cmd line args
	(in_files, out, fast, typ, normal) = parse_commandline();

	if typ == TT_CUBE and len(in_files) != 6:
		raise Exception("Not enough images for cube generation")

	if (typ == TT_3D or typ == TT_2D_ARRAY) and len(in_files) < 2:
		raise Exception("Not enough images for 2DArray/3D texture")

	if typ == TT_2D and len(in_files) != 1:
		raise Exception("Only one image for 2D textures needed")

	# Setup the temp dir
	tmp_dir = tempfile.mkdtemp("_ankitex")

	# Do the work
	try:
		convert(in_files, out, fast, typ, normal, tmp_dir)
	finally:
		shutil.rmtree(tmp_dir)
		
	# Done
	print("-- Done!")

if __name__ == "__main__":
	main()
