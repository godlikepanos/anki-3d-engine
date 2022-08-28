CMP Framework
==============

This library depends only on standard libaray.

CMP Error Codes
---------------

.. code-block:: c

	typedef enum {
        CMP_OK = 0,                            // Ok.
        CMP_ABORTED,                           // The conversion was aborted.
        CMP_ERR_INVALID_SOURCE_TEXTURE,        // The source texture is invalid.
        CMP_ERR_INVALID_DEST_TEXTURE,          // The destination texture is invalid.
        CMP_ERR_UNSUPPORTED_SOURCE_FORMAT,     // The source format is not a supported format.
        CMP_ERR_UNSUPPORTED_DEST_FORMAT,       // The destination format is not a supported format.
        CMP_ERR_UNSUPPORTED_GPU_ASTC_DECODE,   // The gpu hardware is not supported.
        CMP_ERR_UNSUPPORTED_GPU_BASIS_DECODE,  // The gpu hardware is not supported.
        CMP_ERR_SIZE_MISMATCH,                 // The source and destination texture sizes do not match.
        CMP_ERR_UNABLE_TO_INIT_CODEC,          // Compressonator was unable to initialize the codec needed for conversion.
        CMP_ERR_UNABLE_TO_INIT_DECOMPRESSLIB,  // GPU_Decode Lib was unable to initialize the codec needed for decompression .
        CMP_ERR_UNABLE_TO_INIT_COMPUTELIB,     // Compute Lib was unable to initialize the codec needed for compression.
        CMP_ERR_CMP_DESTINATION,               // Error in compressing destination texture
        CMP_ERR_MEM_ALLOC_FOR_MIPSET,          // Memory Error: allocating MIPSet compression level data buffer
        CMP_ERR_UNKNOWN_DESTINATION_FORMAT,    // The destination Codec Type is unknown! In SDK refer to GetCodecType()
        CMP_ERR_FAILED_HOST_SETUP,             // Failed to setup Host for processing
        CMP_ERR_PLUGIN_FILE_NOT_FOUND,         // The required plugin library was not found
        CMP_ERR_UNABLE_TO_LOAD_FILE,           // The requested file was not loaded
        CMP_ERR_UNABLE_TO_CREATE_ENCODER,      // Request to create an encoder failed
        CMP_ERR_UNABLE_TO_LOAD_ENCODER,        // Unable to load an encode library
        CMP_ERR_NOSHADER_CODE_DEFINED,         // No shader code is available for the requested framework
        CMP_ERR_GPU_DOESNOT_SUPPORT_COMPUTE,   // The GPU device selected does not support compute
        CMP_ERR_NOPERFSTATS,                   // No Performance Stats are available
        CMP_ERR_GPU_DOESNOT_SUPPORT_CMP_EXT,   // The GPU does not support the requested compression extension!
        CMP_ERR_GAMMA_OUTOFRANGE,              // Gamma value set for processing is out of range
        CMP_ERR_PLUGIN_SHAREDIO_NOT_SET,       // The plugin C_PluginSetSharedIO call was not set and is required for this plugin to operate
        CMP_ERR_UNABLE_TO_INIT_D3DX,           // Unable to initialize DirectX SDK or get a specific DX API
        CMP_ERR_GENERIC                        // An unknown error occurred.
	} CMP_ERROR;



Kernel Options and Extensions
-----------------------------
.. code-block:: c

	typedef enum CMPComputeExtensions {
	    CMP_COMPUTE_FP16 = 0x0001,       ///< Enable Packed Math Option for GPU
	    CMP_COMPUTE_MAX_ENUM = 0x7FFF
	} CMP_ComputeExtensions;

	/// An enum selecting the different GPU driver types.
	typedef enum {
	    CMP_CPU = 0,   //Use CPU Only, encoders defined CMP_CPUEncode or Compressonator lib will be used
	    CMP_HPC = 1,   //Use CPU High Performance Compute Encoders with SPMD support defined in CMP_CPUEncode)
	    CMP_GPU = 2,   //Use GPU Kernel Encoders to compress textures using Default GPU Framework auto set by the codecs been used
	    CMP_GPU_OCL = 3,   //Use GPU Kernel Encoders to compress textures using OpenCL Framework
	    CMP_GPU_DXC = 4,   //Use GPU Kernel Encoders to compress textures using DirectX Compute Framework
	    CMP_GPU_VLK = 5    //Use GPU Kernel Encoders to compress textures using Vulkan Compute Framework
	} CMP_Compute_type;


	struct KernalOptions {
	    CMP_ComputeExtensions   Extensions; // Compute extentions to use, set to 0 (default) if you are not using any extensions
	    CMP_DWORD  height;                  // Height of the encoded texture.
	    CMP_DWORD  width;                   // Width of the encoded texture.
	    CMP_FLOAT  fquality;                // Set the quality used for encoders 0.05 is the lowest and 1.0 for highest.
	    CMP_FORMAT format;                  // Encoder codec format to use for processing
	    CMP_Compute_type encodeWith;        // Host Type : default is HPC, options are [HPC or GPU (reserved for future use)]
	    CMP_INT    threads;                 // requested number of threads to use (1=single) max is 128 for HPC, 0 for Auto
	};

	typedef enum _CMP_ANALYSIS_MODES
	{
	    CMP_ANALYSIS_MSEPSNR = 0x00000000  // Enable Measurement of MSE and PSNR for 2 mipset image samples
	} CMP_ANALYSIS_MODES;
	
	typedef struct
	{
	    unsigned long analysisMode;     // Bit mapped setting to enable various forms of image anlaysis
	    unsigned int  channelBitMap;    // Bit setting for active channels to do analysis on and reserved features
	                                    // msb(....ABGR)lsb
	    double        mse;      // Mean Square Error for all active channels in a given CMP_FORMAT
	    double        mseR;     // Mean Square for Red Channel
	    double        mseG;     // Mean Square for Green
	    double        mseB;     // Mean Square for Blue 
	    double        mseA;     // Mean Square for Alpha
	    double        psnr;     // Peak Signal Ratio for all active channels in a given CMP_FORMAT
	    double        psnrR;    // Peak Signal Ratio for Red Chennel
	    double        psnrG;    // Peak Signal Ratio for Green
	    double        psnrB;    // Peak Signal Ratio for Blue
	    double        psnrA;    // Peak Signal Ratio for Alpha
	} CMP_AnalysisData;




Encoder Settings
----------------

.. code-block:: c

	// The structure describing block encoder level settings.
	typedef struct {
	    unsigned int  width;   // Width of the encoded texture.
	    unsigned int  height;  // Height of the encoded texture.
	    unsigned int  pitch;   // Distance to start of next line..
	    float         quality; // Set the quality used for encoders 0.05 is the lowest and 1.0 for highest.
	    unsigned int  format;  // Format of the encoder to use: this is a enum set see compressonator.h CMP_FORMAT
	} CMP_EncoderSetting;


Mip Map Interfaces
------------------

.. code-block:: c

	// MIP MAP Interfaces
	CMP_INT CMP_MaxFacesOrSlices(const CMP_MipSet* pMipSet, CMP_INT nMipLevel);
	CMP_INT CMP_API CMP_CalcMinMipSize(CMP_INT nHeight, CMP_INT nWidth, CMP_INT MipsLevel);
	
	CMP_VOID CMP_API CMP_FreeMipSet(CMP_MipSet *MipSetIn);
	CMP_VOID CMP_API CMP_GetMipLevel(CMP_MipLevel *data, const CMP_MipSet* pMipSet, CMP_INT nMipLevel, CMP_INT nFaceOrSlice);
	CMP_INT  CMP_API CMP_CalcMaxMipLevel(CMP_INT nHeight, CMP_INT nWidth, CMP_BOOL bForGPU);
	CMP_INT  CMP_API CMP_CalcMinMipSize(CMP_INT nHeight, CMP_INT nWidth, CMP_INT MipsLevel);
	
	CMP_INT  CMP_API CMP_GenerateMIPLevels(CMP_MipSet *pMipSet, CMP_INT nMinSize);
	CMP_INT  CMP_API CMP_GenerateMIPLevelsEx(CMP_MipSet* pMipSet, CMP_CFilterParams* pCFilterParams);
	
	CMP_ERROR CMP_API CMP_CreateCompressMipSet(CMP_MipSet* pMipSetCMP, CMP_MipSet* pMipSetSRC);

	// MIP Map Quality
	CMP_UINT  CMP_API CMP_getFormat_nChannels(CMP_FORMAT format);
	CMP_ERROR CMP_API CMP_MipSetAnlaysis(CMP_MipSet* src1, CMP_MipSet* src2, CMP_INT nMipLevel, CMP_INT nFaceOrSlice, CMP_AnalysisData* pAnalysisData);




User Processing Callback
------------------------

.. code-block:: c

	// CMP_MIPFeedback_Proc
	// Feedback function for conversion.
	// \param[in] fProgress The percentage progress of the texture compression.
	// \param[in] mipProgress The current MIP level been processed, value of fProgress = mipProgress
	// \return non-NULL(true) value to abort conversion
	typedef bool(CMP_API* CMP_MIPFeedback_Proc)(CMP_MIPPROGRESSPARAM mipProgress);


Texture Load and Save
------------------------
Has complete support for dds file format and limited versions of jpeg, png, bmp, hdr, psd, tga, gif, pic, psd, pgm and ppm file formats as used in std_image.h

.. code-block:: c

    //--------------------------------------------
    // CMP_Compute Lib: Texture Encoder Interfaces
    //--------------------------------------------
    CMP_ERROR  CMP_API CMP_LoadTexture(const char *sourceFile, CMP_MipSet *pMipSet);
    CMP_ERROR  CMP_API CMP_SaveTexture(const char *destFile,   CMP_MipSet *pMipSet);


Texture Processing 
------------------

Two types of texture processing API are provided. 

CMP_ProcessTexture is a higher level call that sets up a complete framework and supports textures generated with mip level processing. this api uses the alternate framework API's and CMP_ProcessTexture.

CMP_CompressTexture is a lower level access API that users can use to setup processing textures. By default processing uses the Compressonator Core codecs with a CPU framework.
 
.. code-block:: c

    CMP_ERROR  CMP_API CMP_ProcessTexture(CMP_MipSet* srcMipSet, CMP_MipSet* dstMipSet, KernalOptions kernelOptions,  CMP_Feedback_Proc pFeedbackProc);
    CMP_ERROR  CMP_API CMP_CompressTexture(KernalOptions *options,CMP_MipSet srcMipSet,CMP_MipSet dstMipSet,CMP_Feedback_Proc pFeedback);


Using Alternate Frameworks 
--------------------------

These options provides user options to set setup the CMP_CompressTexture interface pipeline for CPU, HPC or GPU based processing, with the "encodeWith" option in KernelOptions.

.. code-block:: c

    //--------------------------------------------
    // CMP_Compute Lib: Host level interface
    //--------------------------------------------
    CMP_ERROR CMP_API CMP_CreateComputeLibrary(CMP_MipSet *srcTexture, KernelOptions  *kernelOptions, void *Reserved);
    CMP_ERROR CMP_API CMP_DestroyComputeLibrary(CMP_BOOL forceClose);
    CMP_ERROR CMP_API CMP_SetComputeOptions(ComputeOptions *options);


Block level Access 
------------------

Provides users with options to process any image block by providing a pointer to the source textures and destination buffers to be processed.
The source pointer can be any location on the original texture as long as it is bounded within a valid 4x4 image block.
The destination buffer must also be sufficiently large enough to hold the compressed buffer generated by the target format.

.. code-block:: c

    //---------------------------------------------------------
    // Generic API to access the core using CMP_EncoderSetting
    //----------------------------------------------------------
    CMP_ERROR CMP_API CMP_CreateBlockEncoder(void **blockEncoder, CMP_EncoderSetting encodeSettings);
    void      CMP_API CMP_DestroyBlockEncoder(void  **blockEncoder);

    CMP_ERROR CMP_API CMP_CompressBlock(void  **blockEncoder,void *srcBlock, unsigned int sourceStride, void *dstBlock, unsigned int dstStride);
    CMP_ERROR CMP_API CMP_CompressBlockXY(void  **blockEncoder,unsigned int blockx, unsigned int blocky, void *imgSrc, unsigned int sourceStride, void *cmpDst, unsigned int dstStride);



Format and Processor Utils 
--------------------------

.. code-block:: c

    CMP_VOID   CMP_API CMP_Format2FourCC(CMP_FORMAT format,   CMP_MipSet *pMipSet);
    CMP_FORMAT CMP_API CMP_ParseFormat(char* pFormat);
    CMP_INT    CMP_API CMP_NumberOfProcessors();


Framework Example: Mip Level Processing
---------------------------------------

You will need to include a header file and a lib file: **CMP_Framework** and **CMP_Framework_MD.lib**

.. code-block:: c

    const char*     pszSourceFile = argv[1];
    const char*     pszDestFile   = argv[2];
    CMP_FORMAT      destFormat    = CMP_ParseFormat(argv[3]);
    CMP_ERROR       cmp_status;
    CMP_FLOAT       fQuality;

    try {
      fQuality = std::stof(argv[4]);
      if (fQuality < 0.0f) {
        fQuality = 0.0f;
        std::printf("Warning: Quality setting is out of range using 0.0\n");
      }
      if (fQuality > 1.0f) {
        fQuality = 1.0f;
        std::printf("Warning: Quality setting is out of range using 1.0\n");
      }
    } catch (...) {
      std::printf("Error: Unable to process quality setting\n");
      return -1;
    }

    if (destFormat == CMP_FORMAT_Unknown) {
        std::printf("Error: Unsupported destination format\n");
        return 0;
    }

    //---------------
    // Load the image
    //---------------
    CMP_MipSet MipSetIn;
    memset(&MipSetIn, 0, sizeof(CMP_MipSet));
    cmp_status = CMP_LoadTexture(pszSourceFile, &MipSetIn);
    if (cmp_status != CMP_OK) {
        std::printf("Error %d: Loading source file!\n",cmp_status);
        return -1;
    }

    //----------------------------------------------------------------------
    // generate mipmap level for the source image, if not already generated
    //----------------------------------------------------------------------

    if (MipSetIn.m_nMipLevels <= 1)
    {
        CMP_INT requestLevel = 10; // Request 10 miplevels for the source image

        //------------------------------------------------------------------------
        // Checks what the minimum image size will be for the requested mip levels
        // if the request is too large, a adjusted minimum size will be returns
        //------------------------------------------------------------------------
        CMP_INT nMinSize = CMP_CalcMinMipSize(MipSetIn.m_nHeight, MipSetIn.m_nWidth, 10);

        //--------------------------------------------------------------
        // now that the minimum size is known, generate the miplevels
        // users can set any requested minumum size to use. The correct
        // miplevels will be set acordingly.
        //--------------------------------------------------------------
        CMP_GenerateMIPLevels(&MipSetIn, nMinSize);
    }

    //==========================
    // Set Compression Options
    //==========================
    KernalOptions   kernel_options;
    memset(&kernel_options, 0, sizeof(KernalOptions));

    kernel_options.format   = destFormat;   // Set the format to process
    kernel_options.fquality = fQuality;     // Set the quality of the result
    kernel_options.threads  = 0;            // Auto setting

    //--------------------------------------------------------------
    // Setup a results buffer for the processed file,
    // the content will be set after the source texture is processed
    // in the call to CMP_ConvertMipTexture()
    //--------------------------------------------------------------
    CMP_MipSet MipSetCmp;
    memset(&MipSetCmp, 0, sizeof(CMP_MipSet));

    //===============================================
    // Compress the texture using Compressonator Lib
    //===============================================
    cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, CompressionCallback);
    if (cmp_status != CMP_OK) {
        CMP_FreeMipSet(&MipSetIn);
        std::printf("Compression returned an error %d\n", cmp_status);
        return cmp_status;
    }

    //----------------------------------------------------------------
    // Save the result into a DDS file
    //----------------------------------------------------------------
    cmp_status = CMP_SaveTexture(pszDestFile, &MipSetCmp);
    CMP_FreeMipSet(&MipSetIn);
    CMP_FreeMipSet(&MipSetCmp);

Example projects have been provided `here <https://github.com/GPUOpen-Tools/Compressonator/tree/master/examples/>`_ with:

* framework_example1 demonstrates simple SDK API usage by generating mipmap levels as shown above. 
* framework_example2 demonstrates how to use the SDK API compression format, using a quality setting and a HPC pipeline framework.
* framework_exmaple3 demonstrates how to use the block level encoding SDK API.

These examples are also distributed through Compressonator Framework installer in the `release <https://github.com/GPUOpen-Tools/Compressonator/releases>`_ page.


Using the Pipeline API Interfaces
---------------------------------
These interfaces are designed to setup a specific data processing pipeline for:

- CPU generated code (1) using standard compilers such Visual Studio, GCC, Clang, ...
- Vectorized CPU generated code (2) using SPMD (Single Process Multi Data) compilers such as ISPC compiler and libs.
- GPU Kernels using OpenCL, DirectX or Vulkan compilers.

To distinguish code used for (1) & (2) Compressonator uses the notation CPU & HPC respectively.
The GPU setting is reserved for future release.

Compressonator framework sets up the path using Encoder Settings options for processing source data at a block level using the Compressonator Core. if the pipeline fails to setup then the data processing will default to the CPU (1) process.

To use the block level encoders, you must first create a specific encoder to access its block functions, this is done by calling CMP_CreateBlockEncoder (), by passing in a void reference pointer for the codec you want to create.  This API requires you to specify which codec format type you are creating using a CMP_EncoderSetting structure format setting.  An optional parameter is provided for setting the quality of the encoded blocks. 

.. code-block:: c

	void *BC7encoder;
	BC7encoder = NULL;
	CMP_EncoderSetting encodeSettings;

	encodeSettings.format = CMP_FORMAT_BC7;
	encodeSettings.quality= 1.0f;

	CMP_ERROR status = CMP_CreateBlockEncoder(&BC7block_encoder, encodeSettings);

If the create is successful the call will return CMP_OK else it will return a CMP_ERROR value.
Once you have the reference pointer you can call the block encode CMP_CompressBlock () passing in the reference pointer and two block level buffers, one for the source and one for the compressed output.

.. code-block:: c

	status = CMP_CompressBlock(&BC7encoder, (void*)sourceBlock,(void*)compressBlock);


For users who want to use a fixed source buffer and destination buffer and have the codec process any specified block offset to them, a call to CMP_CompressBlockXY () is provided. In order for the call to process the correct buffer offset, the original source buffer sizes must be provided during the codec create by specifying the size of the image using the EncoderSetting width and height parameters for the CMP_CreateBlockEncoder call.

.. code-block:: c

	encodeSettings.width  = SourceBufferWidth;
	encodeSettings.height = SourceBufferHeight;

Once this is set the user can call CMP_CompressBlockXY which has a reference to the codec pointer and block locations x for column, y for the height and the fixed source and destination buffer pointers.

.. code-block:: c

	CMP_CompressBlockXY(&BC7encoder, x, y, (void*) sourceBufferData, (void*) compBufferData);


Both the source and destination buffers must be a correctly sized buffers for the encoders to use. 

Once the processing is done the codec reference pointers can be removes from memory by calling CMP_DestroyBlockEncoder passing in the codec reference pointer.


.. code-block:: c

	CMP_DestroyBlockEncoder(&BC7encoder);

.. toctree::
   :maxdepth: 22
   :name: toc-developer_sdk--cmp_framework
   
