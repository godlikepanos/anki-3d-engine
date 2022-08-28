Compressonator SDK
==================
Compressonator SDK's supported codecs includes BC1-BC7/DXTC, ETC1, ETC2, ASTC, ATC, ATI1N, ATI2N.


Error Codes
-----------

All Compressonator API calls return a int success 0 (CMP_OK) or error value > 0 (CMP_ERR) for a more detailed and up to date list look at the file Compressonator.h enum CMP_ERROR values.

.. code-block:: c

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
    CMP_ERR_GENERIC                        // An unknown error occurred.

Convert Texture
---------------

The main API call for both compression and decompression as well as texture conversion:

.. code-block:: c

    /// Converts the source texture to the destination texture
    /// This can be compression, decompression or converting between two uncompressed formats.
    /// \param[in] pSourceTexture A pointer to the source texture.
    /// \param[in] pDestTexture A pointer to the destination texture.
    /// \param[in] pOptions A pointer to the compression options - can be NULL.
    /// \param[in] pFeedbackProc A pointer to the feedback function - can be NULL.
    /// \param[in] pUser1 User data to pass to the feedback function.
    /// \param[in] pUser2 User data to pass to the feedback function.
    /// \return    CMP_OK if successful, otherwise the error code.
    CMP_ERROR CMP_API CMP_ConvertTexture(CMP_Texture* pSourceTexture, CMP_Texture* pDestTexture, const CMP_CompressOptions* pOptions,
                                         CMP_Feedback_Proc pFeedbackProc, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2);

Example Usage of Compressonator API
-----------------------------------

You will need to include a header file and a lib file: **Compressonator.h** and **Compressonator_MD.lib**

and a simple usage is shown as below:

.. code-block:: c

    //==========================
    // Load Source Texture
    //==========================
    CMP_Texture srcTexture;
    // note that LoadDDSFile function is a utils function to initialize the source CMP_Texture
    // you can also initialize the source CMP_Texture the same way as initialize destination CMP_Texture 
    if (!LoadDDSFile(pszSourceFile, srcTexture))
    {
        std::printf("Error loading source file!\n");
        return 0;
    }

    //===================================
    // Initialize Compressed Destination 
    //===================================
    CMP_Texture destTexture;
    destTexture.dwSize     = sizeof(destTexture);
    destTexture.dwWidth    = srcTexture.dwWidth;
    destTexture.dwHeight   = srcTexture.dwHeight;
    destTexture.dwPitch    = 0;
    destTexture.format     = destFormat;
    destTexture.dwDataSize = CMP_CalculateBufferSize(&destTexture);
    destTexture.pData = (CMP_BYTE*)malloc(destTexture.dwDataSize);

    //==========================
    // Set Compression Options
    //==========================
    CMP_CompressOptions options = {0};
    options.dwSize       = sizeof(options);
    options.fquality     = fQuality;
    options.dwnumThreads = 8;

    //==========================
    // Compress Texture
    //==========================
    CMP_ERROR   cmp_status;
    cmp_status = CMP_ConvertTexture(&srcTexture, &destTexture, &options, &CompressionCallback, NULL, NULL);
    if (cmp_status != CMP_OK)
    {
        free(srcTexture.pData);
        free(destTexture.pData);
        std::printf("Compression returned an error %d\n", cmp_status);
        return cmp_status;
    }

    //==========================
    // Save Compressed Testure
    //==========================
    if (cmp_status == CMP_OK)
        SaveDDSFile(pszDestFile, destTexture);

    free(srcTexture.pData);
    free(destTexture.pData);

Example projects have been provided `here <https://github.com/GPUOpen-Tools/Compressonator/tree/master/examples/>`_ with:

* sdk_example1 demonstrates simple SDK API usage as shown above. 
* sdk_example2 demonstrates how to use the SDK API in multihreaded environment.
* sdk_exmaple3 demonstrates how to use the block level SDK API.

These examples are also distributed through Compressonator SDK installer in the `release <https://github.com/GPUOpen-Tools/Compressonator/releases>`_ page.

.. toctree::
   :maxdepth: 8
   :name: toc-developer_sdk-cmp_compressonator
   
