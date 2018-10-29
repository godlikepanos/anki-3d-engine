#!/usr/bin/python3

# Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import argparse
from PIL import Image, ImageDraw
from math import *
import os

class Context:
	input = ""
	out_prefix = ""
	out_count = 0
	out_format = ""

	img = None
	color_delta = 0.0

ctx = Context()

def parse_commandline():
	""" Parse the command line arguments """

	parser = argparse.ArgumentParser(description = "This program takes a blue noise texture and creates an array of " \
			"noise textures",
			formatter_class = argparse.ArgumentDefaultsHelpFormatter)

	parser.add_argument("-i", "--input", required = True, help = "the input noise texture")

	parser.add_argument("-o", "--output-prefix", required = True, help = "prefix of the output images")

	parser.add_argument("-c", "--output-img-count", required = True, help = "number of output images")

	parser.add_argument("-f", "--output-format", help = "output format", default = "")

	args = parser.parse_args()

	ctx.input = args.input
	ctx.out_prefix = args.output_prefix
	ctx.out_count = int(args.output_img_count)
	ctx.out_format = args.output_format

def init():
	# Open image
	ctx.img = Image.open(ctx.input)

	# Color fmt
	if ctx.img.mode != "RGB" and ctx.img.mode != "RGBA":
		raise Exception("Unknown mode %s" % ctx.img.mode)

	if ctx.out_format == "":
		ctx.out_format = ctx.img.mode

	# Color delta
	ctx.color_delta = int(0xFF / (ctx.out_count + 1.0))

def create_image(idx):
	out_img = Image.new(ctx.img.mode, (ctx.img.width, ctx.img.height))

	delta = idx * ctx.color_delta

	for x in range(0, ctx.img.width):
		for y in range(0, ctx.img.height):
			pixel = ctx.img.getpixel((x, y))

			r = int(pixel[0] + delta) % 0xFF
			g = int(pixel[1] + delta) % 0xFF
			b = int(pixel[2] + delta) % 0xFF

			if ctx.img.mode == "RGBA":
				a = int(pixel[3] + delta) % 0xFF

			if ctx.out_format == "RGB":
				out_img.putpixel((x, y), (r, g, b))
			else:
				out_img.putpixel((x, y), (r, g, b, a))

	out_img.save("%s_%02d.png" % (ctx.out_prefix, idx))

def main():
	""" The main """

	parse_commandline()
	init()
	for i in range(ctx.out_count):
		create_image(i + 1)

if __name__ == "__main__":
	main()