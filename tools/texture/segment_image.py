#!/usr/bin/python3

# Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import optparse
import sys
import struct

class Config:
	in_file = ""
	out_dir = ""

def printi(s):
	print("[I] %s" % s)

def parse_commandline():
	""" Parse the command line arguments """

	parser = optparse.OptionParser(usage = "usage: %prog [options]",
			description = "This program takes a single 2D image and spits a number of images. Input and output images "
			"should be TGA.")

	parser.add_option("-i", "--input", dest = "inp", type = "string", help = "specify the image to split.")

	parser.add_option("-o", "--output", dest = "out", type = "string", help = "the directory of the output images.")

	parser.add_option("-s", "--size", dest = "size", type = "string", metavar="WxH", help = "size of the splits.")

	# Add the default value on each option when printing help
	for option in parser.option_list:
		if option.default != ("NO", "DEFAULT"):
			option.help += (" " if option.help else "") + "[default: %default]"

	(options, args) = parser.parse_args()

	if not options.inp or not options.out or not options.size:
		parser.error("argument is missing")

	# Parse the --size
	size = [0, 0]
	try:
		size_strs = options.size.split("x")
		size[0] = int(size_strs[0])
		size[1] = int(size_strs[1])
	except:
		parser.error("incorrect --size: %s" % sys.exc_info()[0])

	config = Config()
	config.in_file = options.inp
	config.out_dir = options.out
	config.size = size

	return config

def split(filename, split_size, out_dir):
	""" Load an image """

	# Read and check the header
	uncompressed_tga_header = struct.pack("BBBBBBBBBBBB", 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0)

	in_file = open(filename, "rb")
	tga_header = in_file.read(12)

	if len(tga_header) != 12:
		raise Exception("Failed reading TGA header")

	if uncompressed_tga_header != tga_header:
		raise Exception("Incorrect TGA header")

	# Read the size and bpp
	header6_buff = in_file.read(6)

	if len(header6_buff) != 6:
		raise Exception("Failed reading TGA header #2")

	header6 = struct.unpack("BBBBBB", header6_buff)

	img_width = header6[1] * 256 + header6[0]
	img_height = header6[3] * 256 + header6[2]
	img_bpp = header6[4];

	if img_bpp != 24 and img_bpp != 32:
		raise Exception("Unexpected bpp")

	# Check split size against the image
	if (img_width % split_size[0]) != 0 or (img_height % split_size[1]) != 0:
		raise Exception("Sizes of the input image and the split are not compatible: %d %d vs %d %d"
				% (img_width, img_height, split_size[0], split_size[1]))

	# Dump the data to an array
	pixels = []
	for y in range(0, img_height):
		pixels.append([])
		for x in range(0, img_width):
			pixels[y].append([])

			if img_bpp == 24:
				pixel = in_file.read(3)

				pixels[y][x].append(pixel[0])
				pixels[y][x].append(pixel[1])
				pixels[y][x].append(pixel[2])
			else:
				pixel = in_file.read(4)

				pixels[y][x].append(pixel[0])
				pixels[y][x].append(pixel[1])
				pixels[y][x].append(pixel[2])
				pixels[y][x].append(pixel[3])

	# Iterate splits and write them
	split_count_x = int(img_width / split_size[0])
	split_count_y = int(img_height / split_size[1])

	count = 0
	for y in range(0, split_count_y):
		for x in range(0, split_count_x):

			# Open file and write header
			out_file = open("%s/out_%02d.tga" % (out_dir, count), "wb")

			out_file.write(uncompressed_tga_header)

			header2 = struct.pack("BBBBBB", split_size[0] % 256,
					int(split_size[0] / 256), split_size[1] % 256,
					int(split_size[1] / 256), img_bpp, 0)

			out_file.write(header2)

			# Iterate split pixels
			for sy in range(0, split_size[1]):
				for sx in range(0, split_size[0]):

					in_x = x * split_size[0] + sx
					in_y = y * split_size[1] + sy

					if(img_bpp == 24):
						pixels_s = struct.pack("BBB", pixels[in_y][in_x][0],
								pixels[in_y][in_x][1], pixels[in_y][in_x][2])
					else:
						pixels_s = struct.pack("BBBB", pixels[in_y][in_x][0],
								pixels[in_x][in_y][1], pixels[in_y][in_x][2],
								pixels[in_y][in_x][3])

					out_file.write(pixels_s)

			out_file.close()
			count += 1

def main():
	""" The main """

	config = parse_commandline();

	split(config.in_file, config.size, config.out_dir)

	printi("Done!")

if __name__ == "__main__":
	main()
