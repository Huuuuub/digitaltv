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

#include "tsConfigFile.h"
#include "tsSysUtils.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::ConfigFile::ConfigFile(const UString& filename, Report& report) :
    _filename(filename),
    _sections(),
    _empty()
{
    if (!filename.empty()) {
        load(filename, report);
    }
}

ts::ConfigFile::ConfigFile(std::istream& strm) :
    _filename(),
    _sections(),
    _empty()
{
    merge(strm);
}


//----------------------------------------------------------------------------
// Default configuration file name (executable file name with ".ini" extension)
//----------------------------------------------------------------------------

ts::UString ts::ConfigFile::DefaultFileName(FileStyle style, const UString& name)
{
    if (style == LOCAL_SYSTEM) {
#if defined(TS_WINDOWS)
        style = WINDOWS_STYLE;
#else
        style = UNIX_STYLE;
#endif
    }

    UString fileName(name);
    if (fileName.empty()) {
        fileName = PathPrefix(BaseName(ExecutableFile()));
    }

    if (style == WINDOWS_STYLE) {
        return DirectoryName(ExecutableFile()) + PathSeparator + fileName + u".ini";
    }
    else {
        return (UserHomeDirectory() + PathSeparator) + u'.' + fileName;
    }
}


//----------------------------------------------------------------------------
// Reset content of the configuration
//----------------------------------------------------------------------------

void ts::ConfigFile::reset()
{
    _sections.clear();
}


//----------------------------------------------------------------------------
// Reload configuration from a file
//----------------------------------------------------------------------------

bool ts::ConfigFile::load(const UString& filename, Report& report)
{
    reset();
    return merge(filename, report);
}


//----------------------------------------------------------------------------
// Merge configuration from a file.
//----------------------------------------------------------------------------

bool ts::ConfigFile::merge(const UString& filename, Report& report)
{
    // Save file name for further save
    _filename = filename;

    // Open the file
    std::ifstream file(_filename.toUTF8().c_str());

    // Non-existent file means empty configuration.
    // Report a warning.
    if (!file) {
        report.error(u"Cannot open configuration file %s", {_filename});
        return false;
    }

    // Parse the content.
    merge(file);
    return true;
}

void ts::ConfigFile::merge(std::istream& strm)
{
    // Initial section is ""
    UString section;
    UString line;
    UString cont;
    size_t pos = 0;

    // Loop on all lines.
    while (line.getLine(strm)) {

        // Rebuild multi-line.
        while (line.endWith(u"\\")) {
            line.erase(line.size() - 1);
            if (!cont.getLine(strm)) {
                break;
            }
            line.append(cont);
        }

        // Remove leading blanks.
        line.trim(true, false);

        if (line.startWith(u"#")) {
            // Ignore comment lines.
        }
        else if (line.startWith(u"[")) {
            // Handle section name
            line.erase(0, 1);
            if ((pos = line.find(u']')) != NPOS) {
                line.erase(pos);
            }
            line.trim();
            section = line;
            // Implicitely creates the section
            _sections[section];
        }
        else if ((pos = line.find(u'=')) != NPOS) {
            // Handle entry definition
            UString name(line, 0, pos);
            UString val(line, pos + 1, NPOS);
            name.trim();
            val.trim();
            _sections[section].append(name, val);
        }
    }
}


//----------------------------------------------------------------------------
// Save a configuration file.
// If no file name is specified, use name from constructor or load()
//----------------------------------------------------------------------------

bool ts::ConfigFile::save(const UString& filename, Report& report) const
{
    // Get file name
    if (!filename.empty()) {
        _filename = filename;
    }
    if (_filename.empty()) {
        report.error(u"no file name specified to save configuration");
        return false;
    }

    // Create the file
    std::ofstream file(_filename.toUTF8().c_str());

    if (!file) {
        report.error(u"error creating configuration file %s", {_filename});
        return false;
    }

    // Save the content
    return save(file).good();
}


//----------------------------------------------------------------------------
// Save a configuration file in a stream
//----------------------------------------------------------------------------

std::ostream& ts::ConfigFile::save(std::ostream& strm) const
{
    // First, save content of section "" (out of any section)
    SectionMap::const_iterator sec(_sections.find(UString()));
    if (sec != _sections.end()) {
        sec->second.save(strm);
    }

    // Then, save all sections, skipping section ""
    for (sec = _sections.begin(); strm && sec != _sections.end(); ++sec) {
        if (!sec->first.empty()) {
            strm << std::endl << "[" << sec->first << "]" << std::endl;
            sec->second.save(strm);
        }
    }

    return strm;
}


//----------------------------------------------------------------------------
// Get the names of all sections
//----------------------------------------------------------------------------

void ts::ConfigFile::getSectionNames(UStringVector& names) const
{
    names.clear();
    for (auto sec = _sections.begin(); sec != _sections.end(); ++sec) {
        names.push_back(sec->first);
    }
}


//----------------------------------------------------------------------------
// Get a reference to a section.
// Return a reference to an empty section if does not exist.
//----------------------------------------------------------------------------

const ts::ConfigSection& ts::ConfigFile::section(const UString& name) const
{
    SectionMap::const_iterator sec = _sections.find(name);
    return sec == _sections.end() ? _empty : sec->second;
}
