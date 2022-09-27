// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/MaliOfflineCompiler.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/File.h>
#include <regex>

namespace anki {

static MaliOfflineCompilerHwUnit strToHwUnit(CString str)
{
	MaliOfflineCompilerHwUnit out = MaliOfflineCompilerHwUnit::NONE;

	if(str.find("FMA") == 0)
	{
		out = MaliOfflineCompilerHwUnit::FMA;
	}
	else if(str.find("CVT") == 0)
	{
		out = MaliOfflineCompilerHwUnit::CVT;
	}
	else if(str.find("SFU") == 0)
	{
		out = MaliOfflineCompilerHwUnit::SFU;
	}
	else if(str.find("LS") == 0)
	{
		out = MaliOfflineCompilerHwUnit::LOAD_STORE;
	}
	else if(str.find("V") == 0)
	{
		out = MaliOfflineCompilerHwUnit::VARYING;
	}
	else if(str.find("T") == 0)
	{
		out = MaliOfflineCompilerHwUnit::TEXTURE;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return out;
}

static CString hwUnitToStr(MaliOfflineCompilerHwUnit u)
{
	CString out;
	switch(u)
	{
	case MaliOfflineCompilerHwUnit::FMA:
		out = "FMA";
		break;
	case MaliOfflineCompilerHwUnit::CVT:
		out = "CVT";
		break;
	case MaliOfflineCompilerHwUnit::SFU:
		out = "SFU";
		break;
	case MaliOfflineCompilerHwUnit::LOAD_STORE:
		out = "LS";
		break;
	case MaliOfflineCompilerHwUnit::VARYING:
		out = "VAR";
		break;
	case MaliOfflineCompilerHwUnit::TEXTURE:
		out = "TEX";
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

void MaliOfflineCompilerOut::toString(StringRaii& str) const
{
	str.destroy();
	str.sprintf("Regs %u Spilling %u "
				"FMA %f CVT %f SFU %f LS %f VAR %f TEX %f Bound %s "
				"FP16 %f%%",
				m_workRegisters, m_spilling, m_fma, m_cvt, m_sfu, m_loadStore, m_varying, m_texture,
				hwUnitToStr(m_boundUnit).cstr(), m_fp16ArithmeticPercentage);
}

static Error runMaliOfflineCompilerInternal(CString maliocExecutable, CString spirvFilename, ShaderType shaderType,
											BaseMemoryPool& tmpPool, MaliOfflineCompilerOut& out)
{
	out = {};

	// Set the arguments
	Array<CString, 3> args;

	switch(shaderType)
	{
	case ShaderType::kVertex:
		args[0] = "-v";
		break;
	case ShaderType::kFragment:
		args[0] = "-f";
		break;
	case ShaderType::kCompute:
		args[0] = "-C";
		break;
	default:
		ANKI_ASSERT(0 && "Unhandled case");
	}

	args[1] = "--vulkan";
	args[2] = spirvFilename;

	// Execute
	Process proc;
	ANKI_CHECK(proc.start(maliocExecutable, args, {}));
	ProcessStatus status;
	I32 exitCode;
	ANKI_CHECK(proc.wait(-1.0, &status, &exitCode));
	if(exitCode != 0)
	{
		StringRaii stderre(&tmpPool);
		const Error err = proc.readFromStderr(stderre);
		ANKI_SHADER_COMPILER_LOGE("Mali offline compiler failed with exit code %d. Stderr: %s", exitCode,
								  (err || stderre.isEmpty()) ? "<no text>" : stderre.cstr());
		return Error::kFunctionFailed;
	}

	// Get stdout
	StringRaii stdouts(&tmpPool);
	ANKI_CHECK(proc.readFromStdout(stdouts));
	const std::string stdoutstl(stdouts.cstr());

	// Work registers
	std::smatch match;
	if(std::regex_search(stdoutstl, match, std::regex("Work registers: ([0-9]+)")))
	{
		ANKI_CHECK(CString(match[1].str().c_str()).toNumber(out.m_workRegisters));
	}
	else
	{
		ANKI_SHADER_COMPILER_LOGE("Error parsing work registers");
		return Error::kFunctionFailed;
	}

#define ANKI_FLOAT_REGEX "([0-9]+[.]?[0-9]*)"

	// Instructions
	if(shaderType == ShaderType::kVertex)
	{
		// Add the instructions in position and varying variants

		std::string stdoutstl2(stdouts.cstr());

		out.m_fma = 0.0;
		out.m_cvt = 0.0;
		out.m_sfu = 0.0;
		out.m_loadStore = 0.0;
		out.m_texture = 0.0;

		U32 count = 0;
		while(std::regex_search(stdoutstl2, match,
								std::regex("Total instruction cycles:\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX
										   "\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX
										   "\\s*([A-Z]+)")))
		{
			ANKI_ASSERT(match.size() == 7);
			Array<F32, 5> floats;
			for(U32 i = 0; i < floats.getSize(); ++i)
			{
				ANKI_CHECK(CString(match[i + 1].str().c_str()).toNumber(floats[i]));
			}

			out.m_fma += floats[0];
			out.m_cvt += floats[1];
			out.m_sfu += floats[2];
			out.m_loadStore += floats[3];
			out.m_texture += floats[4];

			out.m_boundUnit = strToHwUnit(match[6].str().c_str());

			// Advance
			++count;
			stdoutstl2 = match.suffix();
		}

		if(count == 0)
		{
			ANKI_SHADER_COMPILER_LOGE("Error parsing instruction cycles");
			return Error::kFunctionFailed;
		}
	}
	else if(shaderType == ShaderType::kFragment)
	{
		if(std::regex_search(stdoutstl, match,
							 std::regex("Total instruction cycles:\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX
										"\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX
										"\\s*" ANKI_FLOAT_REGEX "\\s*([A-Z]+)")))
		{
			ANKI_ASSERT(match.size() == 8);

			U32 count = 1;
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_fma));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_cvt));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_sfu));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_loadStore));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_varying));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_texture));

			out.m_boundUnit = strToHwUnit(match[count++].str().c_str());
		}
		else
		{
			ANKI_SHADER_COMPILER_LOGE("Error parsing instruction cycles");
			return Error::kFunctionFailed;
		}
	}
	else
	{
		ANKI_ASSERT(shaderType == ShaderType::kCompute);

		if(std::regex_search(stdoutstl, match,
							 std::regex("Total instruction cycles:\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX
										"\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX "\\s*" ANKI_FLOAT_REGEX
										"\\s*([A-Z]+)")))
		{
			ANKI_ASSERT(match.size() == 7);

			U32 count = 1;
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_fma));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_cvt));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_sfu));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_loadStore));
			ANKI_CHECK(CString(match[count++].str().c_str()).toNumber(out.m_texture));

			out.m_boundUnit = strToHwUnit(match[count++].str().c_str());
		}
		else
		{
			ANKI_SHADER_COMPILER_LOGE("Error parsing instruction cycles");
			return Error::kFunctionFailed;
		}
	}

#undef ANKI_FLOAT_REGEX

	// Spilling
	{
		std::string stdoutstl2(stdouts.cstr());

		while(std::regex_search(stdoutstl2, match, std::regex("Stack spilling:\\s([0-9]+)")))
		{
			U32 spill;
			ANKI_CHECK(CString(match[1].str().c_str()).toNumber(spill));
			out.m_spilling += spill;

			// Advance
			stdoutstl2 = match.suffix();
		}
	}

	// FP16
	{
		std::string stdoutstl2(stdouts.cstr());

		U32 count = 0;
		while(std::regex_search(stdoutstl2, match, std::regex("16-bit arithmetic:\\s(?:([0-9]+|N\\/A))")))
		{
			if(CString(match[1].str().c_str()) == "N/A")
			{
				// Do nothing
			}
			else
			{
				U32 percentage;
				ANKI_CHECK(CString(match[1].str().c_str()).toNumber(percentage));
				out.m_fp16ArithmeticPercentage += F32(percentage);
			}

			// Advance
			stdoutstl2 = match.suffix();
			++count;
		}

		if(count == 0)
		{
			ANKI_SHADER_COMPILER_LOGE("Error parsing 16-bit arithmetic");
			return Error::kFunctionFailed;
		}
	}

	// Debug
	if(false)
	{
		printf("%s\n", stdouts.cstr());
		StringRaii str(&tmpPool);
		out.toString(str);
		printf("%s\n", str.cstr());
	}

	return Error::kNone;
}

Error runMaliOfflineCompiler(CString maliocExecutable, ConstWeakArray<U8> spirv, ShaderType shaderType,
							 BaseMemoryPool& tmpPool, MaliOfflineCompilerOut& out)
{
	ANKI_ASSERT(spirv.getSize() > 0);

	// Create temp file to dump the spirv
	StringRaii tmpDir(&tmpPool);
	ANKI_CHECK(getTempDirectory(tmpDir));
	StringRaii spirvFilename(&tmpPool);
	spirvFilename.sprintf("%s/AnKiMaliocTmpSpirv_%" PRIu64 ".spv", tmpDir.cstr(), getRandom());

	File spirvFile;
	ANKI_CHECK(spirvFile.open(spirvFilename, FileOpenFlag::kWrite | FileOpenFlag::kBinary));
	Error err = spirvFile.write(spirv.getBegin(), spirv.getSizeInBytes());
	spirvFile.close();

	// Call malioc
	if(!err)
	{
		err = runMaliOfflineCompilerInternal(maliocExecutable, spirvFilename, shaderType, tmpPool, out);
	}

	// Cleanup
	const Error err2 = removeFile(spirvFilename);

	return (err) ? err : err2;
}

} // end namespace anki
