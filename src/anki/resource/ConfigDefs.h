// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_OPTION(rsrc_maxTextureSize, 1024u * 1024u, 4u, MAX_U32)
ANKI_CONFIG_OPTION(rsrc_dumpShaderSources, 0, 0, 1)
ANKI_CONFIG_OPTION(
	rsrc_dataPaths, ".",
	"The engine loads assets only in from these paths. Separate them with : (it's smart enough to identify drive "
	"letters in Windows)")
ANKI_CONFIG_OPTION(rsrc_transferScratchMemorySize, 256_MB, 1_MB, 4_GB)
