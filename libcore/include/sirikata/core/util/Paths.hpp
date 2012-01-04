// Copyright (c) 2011 Sirikata Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef _SIRIKATA_LIBCORE_UTIL_PATHS_HPP_
#define _SIRIKATA_LIBCORE_UTIL_PATHS_HPP_

#include <sirikata/core/util/Platform.hpp>

namespace Sirikata {

/** Utilities for getting information about paths, such as the executable file,
 *  executable directory, current directory, etc.
 */
namespace Path {

enum Key {
    PATH_START = 0,

    // Full path to executable file
    FILE_EXE,
    // Full path to executable file's directory
    DIR_EXE,
    // Full path to executable file's bundle. On most platform's this is
    // equivalent to DIR_EXE. On OS X, it gives the directory of the .app
    // containing the binary when it is located in one.
    DIR_EXE_BUNDLE,
    // Full path to current directory
    DIR_CURRENT,
    // Full path to a user-specific directory, e.g. /home/username
    DIR_USER,
    // Full path to a hidden directory in a user-specific location,
    // e.g. /home/username/.sirikata
    DIR_USER_HIDDEN,
    // Full path to temporary directory, e.g. under /tmp
    DIR_TEMP,

    PATH_END
};

SIRIKATA_FUNCTION_EXPORT String Get(Key key);
// Get a path from an offset based on key, e.g. use Get(DIR_TEMP, "foo.log") to
// get the equivalent of "/tmp/sirikata/foo.log".
SIRIKATA_FUNCTION_EXPORT String Get(Key key, const String& relative_path);
SIRIKATA_FUNCTION_EXPORT bool Set(Key key, const String& path);

} // namespace Path
} // namespace Sirikata

#endif //_SIRIKATA_LIBCORE_UTIL_PATHS_HPP_