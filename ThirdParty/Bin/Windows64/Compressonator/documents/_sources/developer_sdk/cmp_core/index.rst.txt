CMP Core
=====================================
This library supports the following codecs BC1 to BC7, also known as ATI1N, ATI2N and DXTC.

The main API call for both compression and decompression is at the block level for each of these codecs.

GPU Shaders 
-----------
With OpenCL (OCL) and DirectX (DXC): These API are availble for use in user defined shaders by including BCn_Encode_Kernel.h and Common_Def.h

.. code-block:: c

    CGU_Vec2ui CompressBlockBC1_UNORM( CMP_IN CGU_Vec3f BlockRGB[16], CMP_IN CGU_FLOAT fquality,CGU_BOOL isSRGB)
    CGU_Vec4ui CompressBlockBC2_UNORM( CMP_IN CGU_Vec3f BlockRGB[16], CMP_IN CGU_FLOAT BlockA[16], CGU_FLOAT fquality, CGU_BOOL isSRGB)
    CGU_Vec4ui CompressBlockBC3_UNORM( CMP_IN CGU_Vec3f BlockRGB[16], CMP_IN CGU_FLOAT BlockA[16], CGU_FLOAT fquality,CGU_BOOL isSRGB)
    CGU_Vec2ui CompressBlockBC4_UNORM( CMP_IN CGU_FLOAT Block[16], CGU_FLOAT fquality)
    CGU_Vec4ui CompressBlockBC5_UNORM( CMP_IN CGU_FLOAT BlockU[16], CMP_IN CGU_FLOAT BlockV[16], CGU_FLOAT fquality)

For OCL addition API is provided under a generic API interface for all BCn encoders: CMP_STATIC CMP_KERNEL void CMP_GPUEncoder(...) see plugin CMP_GPU_OCL for details on how this is used.
For DXC CMP_Core contains sample HLSL shaders used by the plugin CMP_GPU_DXC

Error Codes
-----------
All Core API calls return a int success 0 (CMP_CORE_OK) or error value > 0 (CMP_CORE_ERR) for a more detailed and up to date list look at the file Common_Def.h 

.. code-block:: c

    CGU_CORE_OK = 0,                          // No errors, call was successfull
    CGU_CORE_ERR_UNKOWN,                      // An unknown error occurred
    CGU_CORE_ERR_NEWMEM,                      // New Memory Allocation Failed
    CGU_CORE_ERR_INVALIDPTR,                  // The pointer value used is invalid or null
    CGU_CORE_ERR_RANGERED,                    // values for Red   Channel is out of range (too high or too low)
    CGU_CORE_ERR_RANGEGREEN,                  // values for Green Channel is out of range (too high or too low)
    CGU_CORE_ERR_RANGEBLUE,                   // values for Blue  Channel is out of range (too high or too low)


Codec Quality Settings
----------------------

BC1, BC2, and BC3 have discrete quality settings, These settings are available in the following ranges (varying the q setting in these ranges will have no new effects, q is a discrete coarse setting)

.. code-block:: c

    q = 0.0   to 0.01 sets lowest quality and fast compression
    q = 0.101 to 0.6 sets mid-quality
    q = 0.601 to 1.0 set the best quality and low performance  
    BC4 and BC5 have no quality settings, no changes in quality will occur if set.
    BC6 & BC7 & ASTC have full q ranges from 0 to 1.0


Create and Destroy Options Pointers
-----------------------------------

Context based reference pointers are used instead of class or structure definitions for each codec, it provides a clear definition of what the setting options are for each codec and a means for easy expansion of the features for future releases.

BCn is used to describe a short form for all of the codecs BC1 to BC7. 

All BCn codecs will use default max quality settings when a null pointer is used for the CompressBlock calls, users can create multiple contexts to set preferred Quality, Mode Masks , Channel Mapping, Etc...  

A void pointer reference is used to relay the setting options to the respected setting and encoding functions. When a context pointer is created for a particular codecs like BC1 its should not be used for another codecs like BC2, it will create unpredictable results, runtime exceptions or errors.

.. code-block:: c

	int CMP_CDECL CreateOptionsBC1(void **optionsBC1);
	int CMP_CDECL CreateOptionsBC2(void **optionsBC2);
	int CMP_CDECL CreateOptionsBC3(void **optionsBC3);
	int CMP_CDECL CreateOptionsBC4(void **optionsBC4);
	int CMP_CDECL CreateOptionsBC5(void **optionsBC5);
	int CMP_CDECL CreateOptionsBC6(void **optionsBC6);
	int CMP_CDECL CreateOptionsBC7(void **optionsBC7);


These calls removes the context memory used by the CreateOptions.

.. code-block:: c

	int CMP_CDECL DestroyOptionsBC1(void *optionsBC1);
	int CMP_CDECL DestroyOptionsBC2(void *optionsBC2);
	int CMP_CDECL DestroyOptionsBC3(void *optionsBC3);
	int CMP_CDECL DestroyOptionsBC4(void *optionsBC4);
	int CMP_CDECL DestroyOptionsBC5(void *optionsBC5);
	int CMP_CDECL DestroyOptionsBC6(void *optionsBC6);
	int CMP_CDECL DestroyOptionsBC7(void *optionsBC7);


Channel Weights
---------------

Setting channel Weights : Applies to BC1, BC2 and BC3 valid ranges are [0..1.0f] Default is {1.0f, 1.0f, 1.0f}

With swizzle formats the weighting applies to the data within the specified channel not the channel itself

.. code-block:: c

	int CMP_CDECL SetChannelWeightsBC1(void *options, float WeightRed, float WeightGreen, float WeightBlue);
	int CMP_CDECL SetChannelWeightsBC2(void *options, float WeightRed, float WeightGreen, float WeightBlue);
	int CMP_CDECL SetChannelWeightsBC3(void *options, float WeightRed, float WeightGreen, float WeightBlue);

Quality Settings
----------------

All values are clamped in the range 0.0 to 1.0, the encoding performance is much slower when quality is set high.

.. code-block:: c

	int CMP_CDECL SetQualityBC1(void *options, float fquality);
	int CMP_CDECL SetQualityBC2(void *options, float fquality);
	int CMP_CDECL SetQualityBC3(void *options, float fquality);
	int CMP_CDECL SetQualityBC4(void *options, float fquality);
	int CMP_CDECL SetQualityBC5(void *options, float fquality);
	int CMP_CDECL SetQualityBC6(void *options, float fquality);
	int CMP_CDECL SetQualityBC7(void *options, float fquality);

Alpha Threshold
----------------

BC1 uses 1 bit alpha for encoding. This bit is set when pixel values are >= alphaThreshold, default is 128

.. code-block:: c

	int CMP_CDECL SetAlphaThresholdBC1(void *options, unsigned char alphaThreshold);


Mode Masks
----------
This mask can be used to enable any or all encoding modes of the codec. If all are enabled the resulting quality of the image is increased, reducing the modes enabled lowers the quality and increases performance.

**mask :** BC6 uses 14 compression modes, default is all enabled (0x3FFF)

**mask :** BC7 uses 8  compression modes, default is all enabled (0xFF)

The current BC6H codec always defaults all modes enabled. Next release will enable this feature setting! 

BC7 Mask values are mapped by bit position with mode 0 been at the lowest significant bit (LSB) value in the mask. 

+--------------+--+--+--+--+--+--+--+--+
|              MSB                  LSB|
+==============+==+==+==+==+==+==+==+==+
| mask bits    |1 |1 |1 |1 |1 |1 |1 |1 |
+--------------+--+--+--+--+--+--+--+--+
| BC7 mode     |7 |6 |5 |4 |3 |2 |1 |0 |
+--------------+--+--+--+--+--+--+--+--+

Example: To enable modes 6 & 1 only the mask value is set to hex 0x42 or binary 01000010
if mask is set to <= 0 then default will be 0xCF which is what prior versions of Compressonator used.

.. code-block:: c

	int CMP_CDECL SetMaskBC6(void *options, unsigned int  mask);
	int CMP_CDECL SetMaskBC7(void *options, unsigned char mask);


Decoder Channel Mapping
-----------------------
The channel mapping can be set for BC1, BC2 and BC3 decoders to decode channels Red(0) ,Green(1), Blue(2) and Alpha(3) as RGBA channels [0,1,2,3] (default True) else BGRA maps to [0,1,2,3]
In this release bool is used as a swizzle setting (Red, Blue channel swap). Future release will allow different channel mapping!

.. code-block:: c

	int CMP_CDECL SetDecodeChannelMapping(void *options, bool mapRGBA);


BC7 Alpha Options
------------------

**imageNeedsAlpha :** Reserved for future use (default is false) 

**colourRestrict :** Sets block to avoid using Combined Alpha modes. value of true enables else false disables the setting (default is false)

**alphaRestrict :** Avoid blocking issues with punch-through or threshold alpha encoding. value of true enables else false disables the setting (default is false)

Each pixel in the tile is checked for alpha values, if any alpha value in the input block is set below 255 (blockNeedsAlpha = TRUE) or is a value of {0 or 255} (blockAlphaZeroOne = TRUE), respective modes will be either be kept or discarded using the ModeMask as a guide and the following conditions for each block the Mode cycles from 0 to 7.

If the block needs alpha and this mode doesn't support alpha then indicate that this is not a valid mode for the block:
if (blockNeedsAlpha == TRUE) and (Mode has NO ALPHA) then that Mode is disabled from been used for processing

Optional user restriction for color only blocks so that they do not use modes that have combined color and alpha : this avoids the possibility that the encoder might choose an alpha other than 255 (due to parity) and cause something to become accidentally slightly transparent it's possible that when encoding 3-component texture applications will assume that the 4th component can safely be assumed to be 255 all the time


if (blockNeedsAlpha == TRUE) and (Mode has COMBINED ALPHA) and (ColourRestrict == TRUE) then that Mode is excluded for processing


Optional restriction for blocks with alpha to avoid issues with punch-through or threshold alpha encoding:

if (blockNeedsAlpha == TRUE) and (Mode has COMBINED ALPHA) and (AlphaRestrict == TRUE) and (blockAlphaZeroOne == TRUE) then that Mode is excluded for processing


.. code-block:: c

	int CMP_CDECL SetAlphaOptionsBC7(void *options, bool imageNeedsAlpha, bool colourRestrict, bool alphaRestrict);


BC7 Error Threshold
-------------------

The **minThreshold** (default 5.0f) and **maxThreshold** (default 80.0f) is used to map the quality setting and partition ranges used during quantization.
These values should be set prior to using the SetQualityBC7 call else it will have no effect and the defaults will be used.

To increase performance, adjust maxThreshold higher, to reduce performance and increase quality adjust maxThreshold lower.

minThreshold is used to clamp a minimum error value that is added to maxThreshold when quality is set to max 1.0;

The upper range values are not checked, values are clampped to 0.0 if negative!

.. code-block:: c

	int CMP_CDECL SetErrorThresholdBC7(void *options, float minThreshold, float maxThreshold);


Compressing Blocks
------------------

**srcBlock :** Buffer pointer reference to a source block to use for compression, the source buffer is expected to be in RGBA:8888 format for BC1..5,BC7 codecs. For BC6H the buffer should be in Half float format.

**srcStrideInBytes :** Is the number of bytes required to access the next row of the 4x4 block reference from the original srcBlock pointer.

**srcStrideInShorts :** Is the number of short int values required to access the next row of the 4x4 block reference from the original srcBlock pointer.

**cmpBlock :**  Pointer reference to a destination block that is typically in the range of 8 to 16 bytes for BCn codecs.

**options :** Is the encoder reference created by CreateOptions. This can be a null pointer, which uses the codecs default settings for high quality encoding.


BC4 codec uses a srcBlock array of 16 bytes. This array typically represents the Red Channel of a RGBA_8888 4x4 image block.

BC5 codec uses two srcBlocks each are a array of 16 bytes. These two arrays typically represents the Red and Green channels of a RGBA_8888 4x4 image block.

.. code-block:: c

	int CMP_CDECL CompressBlockBC1(unsigned char *srcBlock, unsigned int  srcStrideInBytes, unsigned char cmpBlock[8 ], void *options CMP_DEFAULTNULL);
	int CMP_CDECL CompressBlockBC2(unsigned char *srcBlock, unsigned int  srcStrideInBytes, unsigned char cmpBlock[16], void *options CMP_DEFAULTNULL);
	int CMP_CDECL CompressBlockBC3(unsigned char *srcBlock, unsigned int  srcStrideInBytes, unsigned char cmpBlock[16], void *options CMP_DEFAULTNULL);
	int CMP_CDECL CompressBlockBC4(unsigned char *srcBlock, unsigned int  srcStrideInBytes, unsigned char cmpBlock[8], void *options  CMP_DEFAULTNULL);
	int CMP_CDECL CompressBlockBC5(unsigned char *srcBlock1, unsigned int srcStrideInBytes1,
        	                       unsigned char *srcBlock2, unsigned int srcStrideInBytes2,
                	               unsigned char cmpBlock[16], void *options  CMP_DEFAULTNULL);
	int CMP_CDECL CompressBlockBC6(unsigned short *srcBlock, unsigned int srcStrideInShorts, unsigned char cmpBlock[16], void *options CMP_DEFAULTNULL);
	int CMP_CDECL CompressBlockBC7(unsigned char *srcBlock, unsigned int  srcStrideInBytes, unsigned char cmpBlock[16], void *options CMP_DEFAULTNULL);


Decompressing Blocks
--------------------

.. code-block:: c

	int CMP_CDECL DecompressBlockBC1(unsigned char cmpBlock[8 ], unsigned char srcBlock[64] , void *options CMP_DEFAULTNULL);
	int CMP_CDECL DecompressBlockBC2(unsigned char cmpBlock[16], unsigned char srcBlock[64] , void *options CMP_DEFAULTNULL);
	int CMP_CDECL DecompressBlockBC3(unsigned char cmpBlock[16], unsigned char srcBlock[64] , void *options CMP_DEFAULTNULL);
	int CMP_CDECL DecompressBlockBC4(unsigned char cmpBlock[8 ], unsigned char srcBlock[16] , void *options  CMP_DEFAULTNULL);
	int CMP_CDECL DecompressBlockBC5(unsigned char cmpBlock[16], unsigned char srcBlock1[16], unsigned char srcBlock2[16], void *options  CMP_DEFAULTNULL);
	int CMP_CDECL DecompressBlockBC6(unsigned char cmpBlock[16], unsigned short srcBlock[48], void *options CMP_DEFAULTNULL);
	int CMP_CDECL DecompressBlockBC7(unsigned char cmpBlock[16], unsigned char srcBlock[64] , void *options CMP_DEFAULTNULL);

Example Usage of Core API
-------------------------

Sample application to process a 4x4 image block using Compress and Decompress API's
SDK files required for application to build:

CMP_Core.h
CMP_Core_xx.lib  For static libs xx is either MD, MT or MDd or MTd, when using DLL's make sure the  CMP_Core_xx_DLL.dll is in exe path

Example usage is shown as below, error checking on the function returns has been omitted for clarity:

.. code-block:: c

        // BC1 options context
        void *BC1Options;

	// Create an options context reference using this call, it returns a pointer to use for BC1 codec settings
	// All functions used in CMP Core return error codes to check if calls were successful
        CreateOptionsBC1(&BC1Options);
    
        // Check if pointer is allocated, can also use the call function return CGU_CORE_OK
        if (BC1Options == NULL) {
            printf("Failed to create BC1 Options Context!");
            return (-1);
        }

        // Setting Channel Weights {Red,Green,Blue}
        SetChannelWeightsBC1(BC1Options, 0.3086f, 0.6094f, 0.0820f);

	// Compress a sample image shape0 which is a 4x4 RGBA_8888 block, its stride is 16 bytes to the next row of the 4x4 image block.
	// Users can use a pointer to any sized image buffers to reference a 4x4 block by supplying a stride offset for the next row.

	unsigned char shape0_RGBA[64] = { filled with image source data as RGBA ...};

	// cmpBuffer is a byte array of 8 byte to hold the compressed results.
	unsigned char cmpBuffer[8]   = { 0 };

	// Compress the source into cmpBuffer
        CompressBlockBC1(shape0_RGBA, 16, cmpBuffer, BC1Options);

	// Example to decompress comBuffer back to a RGBA_8888 4x4 image block
	unsigned char imgBuffer[64] = { 0 };
        DecompressBlockBC1(cmpBuffer,imgBuffer,BC1Options);

	// Can compare the original image (shape0_RGBA) vs the decompressed image (imgBuffer) for quality

	// Remove the Options Setting Memory
        DestroyOptionsBC1(BC1Options);


A full example project is provided `here <https://github.com/GPUOpen-Tools/Compressonator/tree/master/examples/>`_ 

* core_example demonstrates compression and decompression of all of the available codecs with various quality and performance settings. 

The example is also distributed through CompressonatorCore installer in the `release <https://github.com/GPUOpen-Tools/Compressonator/releases>`_ page.

.. toctree::
   :maxdepth: 22
   :name: toc-developer_sdk-cmp_core
   
