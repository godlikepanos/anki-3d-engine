Texture Compression and Decompression
=====================================
For more details see Bibliography Reference (1)


BC1 Block (S3TC/DXT1)
---------------------
BC1 block consists of two base colors c0 and c1 and an index table (bitmap).

The index table, however, has a two-bit entry, since BC1 allows for 2 additional colors, c2 and c3 obtained by blending of the base colors. 
All together c0, c1, c2 and c3 could be treated as a local palette for a compressed block. 
The base colors are stored in RGB565 format, i.e. 5 bits for red and blue channels and 6 bit for green channel, resulting in 4bpp compression level.

There are two types of BC1 blocks: the first one that does not support transparency and the second one, that does.



BC2 Block (DXT2/DXT3)
---------------------
The BC1 format can manage 24-bit RGB textures, but is unsuitable for 32-bit RGBA8888 textures.
The BC2 block occupies 128 bit, twice the  BC1 size. Therefore, compression level is 8bpp.
One half of the BC2 is reserved for alpha values with 4-bit precision, the other one is just a BC1 for storing RGB data



BC3 Block (DXT4/DXT5)
---------------------
The BC3 block, likewise BC2, consists of two 64-bit parts: one for the alpha data and one for the color data. 
Color part repeats the BC1 layout as well, but the alpha part is stored in the compressed form.
Alpha compression is very similar to the DXT1 except for the number of the channels; there are two endpoints with 8-bit precision and the table of 3-bit indexes allowing to choose one of the eight values of a local palette.


BC4 Block (ATI1/3Dc+)
---------------------
The BC4 block (Figure 9) is just an alpha part of the BC3 block. It is used for 1-channel textures, for example a height map or a specular map. Decoded values are associated with the red channel.



BC5 Block (ATI2/3Dc)
--------------------
The 3Dc format was originally developed by ATI specially for the normal map compression, as the DXT1 format did not provide the required quality for such data. 
Normal map contains information about the direction of normal vector for every texel, which allows one to compute lighting with high level of detail and without increasing the geometry complexity.



BC6H
----
The BC6H format is designed to compress textures with high dynamic range (HDR). Only RGB images without alpha are supported.
The format uses 128-bit blocks, resulting in  8bpp compression level.
Depending on the block type, a compressed block has a different set of fields and a different size of each field. This allows choosing the best encoding on the per block basis.
This flexibility greatly reduces compression artifacts, but strongly complicates the compression procedure. 
The number of block types has increased to 14 for BC6H and to 8 for BC7. Unlike BC1, block type is set explicitly in the first bits of compressed block. Block type is also referred to as the block mode.

BC7
---
Improves quality be adding new formats that improve the endpoint precision and storing up to three pairs of endpoints. 
The format uses 128-bit blocks, resulting in  8bpp compression level.



.. toctree::
   :maxdepth: 8
   :name: toc-developer_sdk-codecs
   
