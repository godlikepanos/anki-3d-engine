Mesh Compression
================

As of v4.2 glTF draco mesh compression is no longer supported.
Reference CMP_Mesh lib for alternate implementations. 

Mesh compression/decompression provided in Compressonator by adding the following lines:

.. code-block:: c

    //========================================
    // Mesh Compression and Decompression
    //========================================
    std::string         src_file = "source.gltf";       //input source glTF file
    std::string         dst_file = "destination.gltf";  //output destination glTF file
    std::string         err;                            //error messages
    tinygltf2::Model    model;
    tinygltf2::TinyGLTF loader;
    tinygltf2::TinyGLTF saver;

    bool perform_mesh_compression = true;  //flag to turn on/off compression
    bool is_src_file_draco        = false; //flag to indicate source file is compressed or not- 
                                           //can be replaced with helper function provided below to check for glTF compressed file

    bool ret = loader.LoadASCIIFromFile(&model, &err, src_file, perform_mesh_compression);
    if (ret)
        printf("read success");
    else
        printf("read fail: %s", err);

    err.clear();

    CMP_CompressOptions CompressOptions;
    // it is recommended to use only default settings, other settings may result in corrupt in resource like texture.
    CompressOptions.iCmpLevel    = CMP_MESH_COMP_LEVEL;    //setting: compression level (range 0-10: higher mean more compressed) - default 7
    CompressOptions.iPosBits     = CMP_MESH_POS_BITS;      //setting: quantization bits for position - default 14
    CompressOptions.iTexCBits    = CMP_MESH_TEXC_BITS;     //setting: quantization bits for texture coordinates - default 12
    CompressOptions.iNormalBits  = CMP_MESH_NORMAL_BITS;   //setting: quantization bits for normal - default 10
    CompressOptions.iGenericBits = CMP_MESH_GENERIC_BITS;  //setting: quantization bits for generic - default 8

    ret = saver.WriteGltfSceneToFile(&model, &err, dst_file, CompressOptions, is_src_file_draco, perform_mesh_compression);

    if (ret)
        printf("write success");
    else
        printf("write fail: %s", err);
    //==========================================
    // end of Mesh Compression and Decompression
    //==========================================

Helper function
---------------

.. code-block:: c

    //Utility function to check for glTF draco compressed file
    bool isGLTFCompressedFile(std::string filename)
    {
        nlohmann::json j3;
        std::ifstream  f(filename);
        if (!f)
        {
            return false;
        }
    
        f >> j3;
    
        auto extrequired = j3["extensionsRequired"];
    
        for (int i = 0; i < extrequired.size(); i++)
        {
            std::string extname = extrequired[i].get<std::string>();
            if (extname.find("KHR_draco_mesh_compression") != string::npos)
            {
                return true;
            }
        }
    
        auto extused = j3["extensionsUsed"];
    
        for (int j = 0; j < extused.size(); j++)
        {
            std::string extnameused = extused[j].get<std::string>();
            if (extnameused.find("KHR_draco_mesh_compression") != string::npos)
            {
                return true;
            }
        }
    
        return false;
    }

	
.. toctree::
   :maxdepth: 8
   :name: toc-developer_sdk-mesh_compression
  
   


