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

#include "tsOutputPager.h"
#include "tsSysUtils.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Default constructor.
//----------------------------------------------------------------------------

ts::OutputPager::OutputPager(const UString& envName) :
    ForkPipe(),
    _hasTerminal(false),
    _outputMode(KEEP_BOTH),
    _pagerCommand()
{
    // Check if we have a terminal.
    const bool outTerm = StdOutIsTerminal();
    const bool errTerm = StdErrIsTerminal();
    _hasTerminal = outTerm || errTerm;

    // Check if we should redirect one output.
    if (outTerm && !errTerm) {
        _outputMode = STDOUT_ONLY;
    }
    else if (!outTerm && errTerm) {
        _outputMode = STDERR_ONLY;
    }

    // Get the pager command.
    // First, check if the PAGER variable contains something.
    if (!envName.empty()) {
        _pagerCommand = ts::GetEnvironment(envName);
        _pagerCommand.trim();
    }

    // If there is no pager command, try to find one.
    if (_pagerCommand.empty()) {

        // Get the path search list.
        UStringList dirs;
        GetEnvironmentPath(dirs, TS_COMMAND_PATH);

        // Predefined list of commands.
        struct PredefinedPager {
            UString command;
            UString parameters;
        };
        const std::list<PredefinedPager> pagers({
            {u"less", u"-QFX"},
            {u"more", u""}
        });

        // Search the predefined pager commands in the path.
        for (std::list<PredefinedPager>::const_iterator itPager = pagers.begin(); itPager != pagers.end() && _pagerCommand.empty(); ++itPager) {
            for (UStringList::const_iterator itDir = dirs.begin(); itDir != dirs.end() && _pagerCommand.empty(); ++itDir) {
                // Full path of executable file.
                const UString exe(*itDir + PathSeparator + itPager->command + TS_EXECUTABLE_SUFFIX);
                if (FileExists(exe)) {
                    // The executable exists. Build the command line.
                    _pagerCommand = u'"' + exe + u"\" " + itPager->parameters;
                }
            }
        }
    }

    // On Windows, we can always use the embedded "more" command.
#if defined(TS_WINDOWS)
    if (_pagerCommand.empty()) {
        _pagerCommand = u"cmd /d /q /c more";
    }
#endif
}


//----------------------------------------------------------------------------
// Create the process, open the pipe.
//----------------------------------------------------------------------------

bool ts::OutputPager::open(bool synchronous, size_t buffer_size, Report& report)
{
    if (!_hasTerminal) {
        report.error(u"not a terminal, cannot page");
        return false;
    }
    else if (_pagerCommand.empty()) {
        report.error(u"no pager command found, cannot page");
        return false;
    }
    else {
        return ForkPipe::open(_pagerCommand, synchronous ? ForkPipe::SYNCHRONOUS : ForkPipe::ASYNCHRONOUS, buffer_size, report, _outputMode, STDIN_PIPE);
    }
}


//----------------------------------------------------------------------------
// Write data to the pipe (received at process' standard input).
//----------------------------------------------------------------------------

bool ts::OutputPager::write(const UString& text, Report& report)
{
    const std::string utf8Text(text.toUTF8());
    return ForkPipe::write(utf8Text.data(), utf8Text.size(), report);
}
