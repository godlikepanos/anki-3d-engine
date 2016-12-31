#!/usr/bin/python3

# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import argparse
from PIL import Image, ImageDraw
from math import *
import os

class SubImage:
	image = None
	image_name = ""
	width = 0
	height = 0

	mwidth = 0
	mheight = 0

	atlas_x = 0xFFFFFFFF
	atlas_y = 0xFFFFFFFF

class Frame:
	x = 0
	y = 0
	w = 0
	h = 0

	@classmethod
	def diagonal(self):
		return sqrt(self.w * self.w + self.h * self.h)
	
	@classmethod
	def area(self):
		return self.w * self.h

class Context:
	in_files = []
	out_file = ""
	margin = 0
	bg_color = 0
	rpath = ""

	sub_images = []
	atlas_width = 0
	atlas_height = 0
	mode = None

def next_power_of_two(x):
	return pow(2.0, ceil(log(x) / log(2)))

def printi(msg):
	print("[I] %s" % msg)

def parse_commandline():
	""" Parse the command line arguments """

	parser = argparse.ArgumentParser(description = "This program creates a texture atlas",
			formatter_class = argparse.ArgumentDefaultsHelpFormatter)

	parser.add_argument("-i", "--input", nargs = "+", required = True,
			help = "specify the image(s) to convert. Seperate with space")

	parser.add_argument("-o", "--output", default = "atlas.png", help = "specify output PNG image.")

	parser.add_argument("-m", "--margin", type = int, default = 0, help = "specify the margin.")

	parser.add_argument("-b", "--background-color", help = "specify background of empty areas", default = "ff00ff00")

	parser.add_argument("-r", "--rpath", help = "Path to append to the .ankiatex", default = "")

	args = parser.parse_args()

	ctx = Context()
	ctx.in_files = args.input
	ctx.out_file = args.output
	ctx.margin = args.margin
	ctx.bg_color = int(args.background_color, 16)
	ctx.rpath = args.rpath

	if len(ctx.in_files) < 2:
		parser.error("Not enough images")

	return ctx

def load_images(ctx):
	""" Load the images """

	for i in ctx.in_files:
		img = SubImage()
		img.image = Image.open(i)
		img.image_name = i

		if ctx.mode == None:
			ctx.mode = img.image.mode
		else:
			if ctx.mode != img.image.mode:
				raise Exception("Image \"%s\" has a different mode: \"%s\"" % (i, img.image.mode))

		img.width = img.image.size[0]
		img.height = img.image.size[1]

		img.mwidth = img.width + ctx.margin
		img.mheight = img.height + ctx.margin

		printi("Image \"%s\" loaded. Mode \"%s\"" % (i, img.image.mode))

		for simage in ctx.sub_images:
			if os.path.basename(simage.image_name) == os.path.basename(i):
				raise Exception("Cannot have images with the same base %s" % i)

		ctx.sub_images.append(img)

def compute_atlas_rough_size(ctx):
	for i in ctx.sub_images:
		ctx.atlas_width += i.mwidth
		ctx.atlas_height += i.mheight

	ctx.atlas_width = next_power_of_two(ctx.atlas_width)
	ctx.atlas_height = next_power_of_two(ctx.atlas_height)

def sort_image_key_diagonal(img):
	return img.width * img.width + img.height * img.height

def sort_image_key_biggest_side(img):
	return max(img.width, img.height)

def best_fit(img, crnt_frame, frame):
	if img.mwidth > frame.w or img.mheight > frame.h:
		return False

	if frame.area() < crnt_frame.area():
		return True
	else:
		return False

def worst_fit(img, crnt_frame, new_frame):
	if img.mwidth > new_frame.w or img.mheight > new_frame.h:
		return False

	if new_frame.area() > crnt_frame.area():
		return True
	else:
		return False

def closer_to_00(img, crnt_frame, new_frame):
	if img.mwidth > new_frame.w or img.mheight > new_frame.h:
		return False

	new_dist = new_frame.x * new_frame.x + new_frame.y * new_frame.y
	crnt_dist = crnt_frame.x * crnt_frame.x + crnt_frame.y * crnt_frame.y

	if new_dist < crnt_dist:
		return True
	else:
		return False

def place_sub_images(ctx):
	""" Place the sub images in the atlas """

	# Sort the images
	ctx.sub_images.sort(key = sort_image_key_diagonal, reverse = True)

	frame = Frame()
	frame.w = ctx.atlas_width
	frame.h = ctx.atlas_height
	frames = []
	frames.append(frame)

	unplaced_imgs = []
	for i in range(0, len(ctx.sub_images)):
		unplaced_imgs.append(i)

	while len(unplaced_imgs) > 0:
		sub_image = ctx.sub_images[unplaced_imgs[0]]
		unplaced_imgs.pop(0)

		printi("Will try to place image \"%s\" of size %ux%d" % 
				(sub_image.image_name, sub_image.width, sub_image.height))

		# Find best frame
		best_frame = None
		best_frame_idx = 0
		idx = 0
		for frame in frames:
			if not best_frame or closer_to_00(sub_image, best_frame, frame):
				best_frame = frame
				best_frame_idx = idx
			idx += 1

		assert best_frame != None, "See file"

		# Update the sub_image
		sub_image.atlas_x = best_frame.x + ctx.margin
		sub_image.atlas_y =	best_frame.y + ctx.margin
		printi("Image placed in %dx%d" % (sub_image.atlas_x, sub_image.atlas_y))

		# Split frame
		frame_top = Frame()
		frame_top.x = best_frame.x + sub_image.mwidth
		frame_top.y = best_frame.y
		frame_top.w = best_frame.w - sub_image.mwidth
		frame_top.h = sub_image.mheight

		frame_bottom = Frame()
		frame_bottom.x = best_frame.x
		frame_bottom.y = best_frame.y + sub_image.mheight
		frame_bottom.w = best_frame.w
		frame_bottom.h = best_frame.h - sub_image.mheight

		frames.pop(best_frame_idx)
		frames.append(frame_top)
		frames.append(frame_bottom)

def shrink_atlas(ctx):
	""" Compute the new atlas size """

	width = 0
	height = 0
	for sub_image in ctx.sub_images:
		width = max(width, sub_image.atlas_x + sub_image.width + ctx.margin)
		height = max(height, sub_image.atlas_y + sub_image.height + ctx.margin)

	ctx.atlas_width = next_power_of_two(width)
	ctx.atlas_height = next_power_of_two(height)

def create_atlas(ctx):
	""" Create and populate the atlas """

	# Change the color to something PIL can understand
	bg_color = (ctx.bg_color >> 24)
	bg_color |= (ctx.bg_color >> 8) & 0xFF00
	bg_color |= (ctx.bg_color << 8) & 0xFF0000
	bg_color |= (ctx.bg_color << 24) & 0xFF000000

	mode = "RGB"
	if ctx.mode == "RGB":
		color_space = (255, 255, 255)
	else:
		mode = "RGBA"
		color_space = (255, 255, 255, 255)

	atlas_img = Image.new(mode, \
			(int(ctx.atlas_width), int(ctx.atlas_height)), color_space)

	draw = ImageDraw.Draw(atlas_img)
	draw.rectangle((0, 0, ctx.atlas_width, ctx.atlas_height), bg_color)

	for sub_image in ctx.sub_images:
		assert sub_image.atlas_x != 0xFFFFFFFF and sub_image.atlas_y != 0xFFFFFFFF, "See file"

		atlas_img.paste(sub_image.image, (int(sub_image.atlas_x), int(sub_image.atlas_y)))

	printi("Saving atlas \"%s\"" % ctx.out_file)
	atlas_img.save(ctx.out_file)

def write_xml(ctx):
	""" Write the schema """

	fname = os.path.splitext(ctx.out_file)[0] + ".ankiatex"
	printi("Writing XML \"%s\"" % fname)
	f = open(fname, "w")
	f.write("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n")
	f.write("<textureAtlas>\n")
	out_filename = ctx.rpath + "/" + os.path.splitext(os.path.basename(ctx.out_file))[0] + ".ankitex"
	f.write("\t<texture>%s</texture>\n" % out_filename)
	f.write("\t<subTextureMargin>%u</subTextureMargin>\n" % ctx.margin)
	f.write("\t<subTextures>\n")

	for sub_image in ctx.sub_images:
		f.write("\t\t<subTexture>\n")
		f.write("\t\t\t<name>%s</name>\n" % os.path.basename(sub_image.image_name))

		# Now change coordinate system
		left = sub_image.atlas_x / ctx.atlas_width
		right = left + (sub_image.width / ctx.atlas_width)
		top = (ctx.atlas_height - sub_image.atlas_y) / ctx.atlas_height
		bottom = top - (sub_image.height / ctx.atlas_height)

		f.write("\t\t\t<uv>%f %f %f %f</uv>\n" % (left, bottom, right, top))
		f.write("\t\t</subTexture>\n")

	f.write("\t</subTextures>\n")
	f.write("</textureAtlas>\n")

def main():
	""" The main """

	ctx = parse_commandline();
	load_images(ctx)
	compute_atlas_rough_size(ctx)
	place_sub_images(ctx)
	shrink_atlas(ctx)
	create_atlas(ctx)
	write_xml(ctx)

if __name__ == "__main__":
	main()

