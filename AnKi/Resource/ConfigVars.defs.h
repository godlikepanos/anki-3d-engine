// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_VAR_GROUP(RSRC)

ANKI_CONFIG_VAR_U32(RsrcMaxImageSize, 1024u * 1024u, 4u, kMaxU32, "Max image size to load")
ANKI_CONFIG_VAR_STRING(
	RsrcDataPaths, ".",
	"The engine loads assets only in from these paths. Separate them with : (it's smart enough to identify drive "
	"letters in Windows)")
ANKI_CONFIG_VAR_STRING(RsrcDataPathExcludedStrings, "AndroidProject",
					   "A list of string separated by : that will be used to exclude paths from rsrc_dataPaths")
ANKI_CONFIG_VAR_PTR_SIZE(RsrcTransferScratchMemorySize, 256_MB, 1_MB, 4_GB,
						 "Memory that is used fot texture and buffer uploads")
ANKI_CONFIG_VAR_BOOL(RsrcForceFullFpPrecision, false, "Force full floating point precision")
