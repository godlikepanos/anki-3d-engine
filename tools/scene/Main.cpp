// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"

static void parseCommandLineArgs(int argc, char** argv, Exporter& exporter)
{
	static const char* usage = R"(Usage: %s in_file out_dir [options]
Options:
-rpath <string>    : Append a string to the meshes and materials
-texrpath <string> : Append a string to the textures paths
-flipyz            : Flip y with z (For blender exports)
)";

	// Parse config
	if(argc < 3)
	{
		goto error;
	}

	exporter.m_inputFilename = argv[1];
	exporter.m_outputDirectory = argv[2] + std::string("/");

	for(int i = 3; i < argc; i++)
	{
		if(strcmp(argv[i], "-texrpath") == 0)
		{
			++i;

			if(i < argc)
			{
				exporter.m_texrpath = argv[i] + std::string("/");
			}
			else
			{
				goto error;
			}
		}
		else if(strcmp(argv[i], "-rpath") == 0)
		{
			++i;

			if(i < argc)
			{
				exporter.m_rpath = argv[i] + std::string("/");
			}
			else
			{
				goto error;
			}
		}
		else if(strcmp(argv[i], "-flipyz") == 0)
		{
			exporter.m_flipyz = true;
		}
		else
		{
			goto error;
		}
	}

	if(exporter.m_rpath.empty())
	{
		exporter.m_rpath = exporter.m_outputDirectory;
	}

	if(exporter.m_texrpath.empty())
	{
		exporter.m_texrpath = exporter.m_outputDirectory;
	}

	return;

error:
	printf(usage, argv[0]);
	exit(1);
}

int main(int argc, char** argv)
{
	try
	{
		Exporter exporter;

		parseCommandLineArgs(argc, argv, exporter);

		// Load file
		exporter.load();

		// Export
		exporter.exportAll();
	}
	catch(std::exception& e)
	{
		ERROR("Exception: %s", &e.what()[0]);
	}
}
