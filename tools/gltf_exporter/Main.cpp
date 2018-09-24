// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"

using namespace anki;

static const char* USAGE = R"(Usage: %s in_file out_dir [options]
Options:
-rpath <string>     : Replace all absolute paths of assets with that path
-texrpath <string>  : Same as rpath but for textures
)";

static Error parseCommandLineArgs(int argc, char** argv, Exporter& exporter)
{
	Bool rpathFound = false;
	Bool texrpathFound = false;

	// Parse config
	if(argc < 3)
	{
		return Error::USER_DATA;
	}

	exporter.m_inputFilename.create(argv[1]);
	exporter.m_outputDirectory.sprintf("%s/", argv[2]);

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
					exporter.m_texrpath.sprintf("%s/", argv[i]);
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
					exporter.m_rpath.sprintf("%s/", argv[i]);
				}
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
		exporter.m_rpath = exporter.m_outputDirectory;
	}

	if(!texrpathFound)
	{
		exporter.m_texrpath = exporter.m_outputDirectory;
	}

	return Error::NONE;
}

int main(int argc, char** argv)
{
	Exporter exporter;
	if(parseCommandLineArgs(argc, argv, exporter))
	{
		ANKI_LOGE(USAGE, argv[0]);
		return 1;
	}

	return 0;
}