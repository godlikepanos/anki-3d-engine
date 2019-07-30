// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Importer.h"

using namespace anki;

static const char* USAGE = R"(Usage: %s in_file out_dir [options]
Options:
-rpath <string>        : Replace all absolute paths of assets with that path
-texrpath <string>     : Same as rpath but for textures
-optimize-meshes <0|1> : Optimize meshes. Default is 1
)";

class CmdLineArgs
{
public:
	HeapAllocator<U8> m_alloc{allocAligned, nullptr};
	StringAuto m_inputFname = {m_alloc};
	StringAuto m_outDir = {m_alloc};
	StringAuto m_rpath = {m_alloc};
	StringAuto m_texRpath = {m_alloc};
	Bool m_optimizeMeshes = true;
};

static Error parseCommandLineArgs(int argc, char** argv, CmdLineArgs& info)
{
	Bool rpathFound = false;
	Bool texrpathFound = false;

	// Parse config
	if(argc < 3)
	{
		return Error::USER_DATA;
	}

	info.m_inputFname.create(argv[1]);
	info.m_outDir.sprintf("%s/", argv[2]);

	for(I i = 3; i < argc; i++)
	{
		if(strcmp(argv[i], "-texrpath") == 0)
		{
			texrpathFound = true;
			++i;

			if(i < argc)
			{
				if(std::strlen(argv[i]) > 0)
				{
					info.m_texRpath.sprintf("%s/", argv[i]);
				}
				else
				{
					info.m_texRpath.sprintf("");
				}
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else if(strcmp(argv[i], "-rpath") == 0)
		{
			rpathFound = true;
			++i;

			if(i < argc)
			{
				if(std::strlen(argv[i]) > 0)
				{
					info.m_rpath.sprintf("%s/", argv[i]);
				}
				else
				{
					info.m_rpath.sprintf("");
				}
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else if(strcmp(argv[i], "-optimize-meshes") == 0)
		{
			++i;

			if(i < argc)
			{
				I optimize = 1;
				ANKI_CHECK(CString(argv[i]).toNumber(optimize));
				info.m_optimizeMeshes = optimize != 0;
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else
		{
			return Error::USER_DATA;
		}
	}

	if(!rpathFound)
	{
		info.m_rpath = info.m_outDir;
	}

	if(!texrpathFound)
	{
		info.m_texRpath = info.m_outDir;
	}

	return Error::NONE;
}

int main(int argc, char** argv)
{
	CmdLineArgs info;
	if(parseCommandLineArgs(argc, argv, info))
	{
		ANKI_GLTF_LOGE(USAGE, argv[0]);
		return 1;
	}

	Importer importer;
	if(importer.init(info.m_inputFname.toCString(),
		   info.m_outDir.toCString(),
		   info.m_rpath.toCString(),
		   info.m_texRpath.toCString(),
		   info.m_optimizeMeshes))
	{
		return 1;
	}

	if(importer.writeAll())
	{
		return 1;
	}

	return 0;
}