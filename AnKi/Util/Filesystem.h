// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/String.h>
#include <AnKi/Util/Function.h>

namespace anki {

/// @addtogroup util_file
/// @{

/// Return true if a file exists
Bool fileExists(const CString& filename);

/// Get path extension.
void getFilepathExtension(const CString& filename, String& out);

/// Get path filename.
/// On path/to/file.ext return file.ext
void getFilepathFilename(const CString& filename, String& out);

/// Get base path.
/// On path/to/file.ext return path/to
void getParentFilepath(const CString& filename, String& out);

/// Return true if directory exists?
Bool directoryExists(const CString& dir);

/// @internal
Error walkDirectoryTreeInternal(const CString& dir, const Function<Error(const CString&, Bool)>& callback);

/// Walk a directory tree.
/// @param dir The dir to walk.
/// @param func A lambda. See code example on how to use it.
/// Example:
/// @code
/// walkDirectoryTree("./path/to", [&, this](CString path, Bool isDir) {
/// 	...
/// 	return Error::kNone;
/// });
/// @endcode
template<typename TFunc>
Error walkDirectoryTree(const CString& dir, TFunc func)
{
	Function<Error(const CString&, Bool)> f(func);
	const Error err = walkDirectoryTreeInternal(dir, f);
	return err;
}

/// Equivalent to: rm -rf dir
/// @param dir The directory to remove.
Error removeDirectory(const CString& dir);

/// Remove a file.
Error removeFile(const CString& filename);

/// Equivalent to: mkdir dir
Error createDirectory(const CString& dir);

/// Get the home directory.
Error getHomeDirectory(String& out);

/// Get the temp directory.
Error getTempDirectory(String& out);

/// Get the time the file was last modified.
Error getFileModificationTime(CString filename, U32& year, U32& month, U32& day, U32& hour, U32& min, U32& second);

/// Get the path+filename of the currently running executable.
Error getApplicationPath(String& path);

/// A convenience class to delete a file when it goes out of scope. It tries multiple times because of Windows
/// antivirus sometimes keeping a lock to the file.
class CleanupFile
{
public:
	String m_fileToDelete;
	U32 m_tries = 3 * 1000; ///< Number of times to try delete the file.
	Second m_seepTimeBeforeNextTry = 1.0_ms; ///< Time before the next try.

	CleanupFile(CString filename)
		: m_fileToDelete(filename)
	{
	}

	/// Deletes the file.
	~CleanupFile();
};
/// @}

} // end namespace anki
