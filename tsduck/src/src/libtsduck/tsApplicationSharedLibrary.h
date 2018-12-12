//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2018, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------
//!
//!  @file
//!  Application shared libraries
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsSharedLibrary.h"
#include "tsNullReport.h"

namespace ts {
    //!
    //! Representation of an application shared library.
    //! @ingroup system
    //!
    class TSDUCKDLL ApplicationSharedLibrary: public SharedLibrary
    {
    public:
        //!
        //! Constructor.
        //! @param [in] filename Shared library file name. Directory and suffix are optional.
        //! If @a filename contains a directory, the specified file is used directly, with
        //! and without suffix (.so, .dll). If @a filename is just a name without directory,
        //! a search algorithm is used. All directories in @a library_path are searched.
        //! Then the same directory as the executable is searched. In each directory, a
        //! file with @a prefix is searched. Then, if not found, without prefix.
        //! Finally, when everything failed, @a filename is searched with the default
        //! system lookup mechanism.
        //! @param [in] prefix Prefix to add to @a filename if the file is not found.
        //! @param [in] library_path Name of an environment variable, an optional list of directories to search,
        //! similar to @c LD_LIBARY_PATH.
        //! @param [in] permanent If false (the default), the shared library is unloaded from the current process
        //! when this object is destroyed. If true, the shared library remains active.
        //! @param [in,out] report Where to report errors.
        //!
        explicit ApplicationSharedLibrary(const UString& filename,
                                          const UString& prefix = UString(),
                                          const UString& library_path = UString(),
                                          bool permanent = false,
                                          Report& report = NULLREP);

        //!
        //! The module name is derived from the file name without the prefix.
        //! @return The module name.
        //!
        UString moduleName() const;

        //!
        //! Get the prefix.
        //! @return The file name prefix.
        //!
        UString prefix() const {return _prefix;}

        //!
        //! Get a list of plugins.
        //! @param [out] files List of shared library files.
        //! @param [in] prefix Prefix for plugin names.
        //! @param [in] library_path Name of an environment variable, an optional list of directories to search, similar to @c LD_LIBARY_PATH.
        //!
        static void GetPluginList(UStringVector& files, const UString& prefix, const UString& library_path = UString());

    private:
        UString _prefix;

        // Unreachable operations.
        ApplicationSharedLibrary() = delete;
        ApplicationSharedLibrary(const ApplicationSharedLibrary&) = delete;
        ApplicationSharedLibrary& operator=(const ApplicationSharedLibrary&) = delete;
    };
}
