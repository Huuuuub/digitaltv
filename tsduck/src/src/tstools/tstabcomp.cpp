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
//
//  PSI/SI tables compiler.
//
//----------------------------------------------------------------------------

#include "tsMain.h"
#include "tsSysUtils.h"
#include "tsBinaryTable.h"
#include "tsSectionFile.h"
#include "tsDVBCharset.h"
#include "tsxmlTweaksArgs.h"
#include "tsReportWithPrefix.h"
#include "tsInputRedirector.h"
#include "tsOutputRedirector.h"
TSDUCK_SOURCE;

// With static link, enforce a reference to MPEG/DVB structures.
#if defined(TSDUCK_STATIC_LIBRARY)
#include "tsStaticReferencesDVB.h"
const ts::StaticReferencesDVB dependenciesForStaticLib;
#endif


//----------------------------------------------------------------------------
//  Command line options
//----------------------------------------------------------------------------

struct Options: public ts::Args
{
    Options(int argc, char *argv[]);

    ts::UStringVector     infiles;         // Input file names.
    ts::UString           outfile;         // Output file path.
    bool                  outdir;          // Output name is a directory.
    bool                  compile;         // Explicit compilation.
    bool                  decompile;       // Explicit decompilation.
    bool                  xmlModel;        // Display XML model instead of compilation.
    ts::xml::TweaksArgs   xmlTweaks;       // XML formatting options.
    const ts::DVBCharset* defaultCharset;  // Default DVB character set to interpret strings.

private:
    // Inaccessible operations.
    Options(const Options&) = delete;
    Options& operator=(const Options&) = delete;
};

Options::Options(int argc, char *argv[]) :
    Args(u"PSI/SI tables compiler", u"[options] filename ..."),
    infiles(),
    outfile(),
    outdir(false),
    compile(false),
    decompile(false),
    xmlModel(false),
    xmlTweaks(),
    defaultCharset(nullptr)
{
    xmlTweaks.defineOptions(*this);

    option(u"", 0, STRING);
    help(u"",
         u"XML source files to compile or binary table files to decompile. By default, "
         u"files ending in .xml are compiled and files ending in .bin are decompiled. "
         u"For other files, explicitly specify --compile or --decompile.");

    option(u"compile", 'c');
    help(u"compile",
         u"Compile all files as XML source files into binary files. This is the "
         u"default for .xml files.");

    option(u"decompile", 'd');
    help(u"decompile",
         u"Decompile all files as binary files into XML files. This is the default "
         u"for .bin files.");

    option(u"default-charset", 0, STRING);
    help(u"default-charset", u"name",
         u"Default DVB character set to use. The available table names are " +
         ts::UString::Join(ts::DVBCharset::GetAllNames()) + u".\n\n"
         u"With --compile, this character set is used to encode strings. If a "
         u"given string cannot be encoded with this character set or if this option "
         u"is not specified, an appropriate character set is automatically selected.\n\n"
         u"With --decompile, this character set is used to interpret DVB strings "
         u"without explicit character table code. According to DVB standard ETSI EN "
         u"300 468, the default DVB character set is ISO-6937. However, some bogus "
         u"signalization may assume that the default character set is different, "
         u"typically the usual local character table for the region. This option "
         u"forces a non-standard character table.");

    option(u"output", 'o', STRING);
    help(u"output", u"filepath",
         u"Specify the output file name. By default, the output file has the same "
         u"name as the input and extension .bin (compile) or .xml (decompile). If "
         u"the specified path is a directory, the output file is built from this "
         u"directory and default file name. If more than one input file is specified, "
         u"the output path, if present, must be a directory name.");

    option(u"xml-model", 'x');
    help(u"xml-model",
         u"Display the XML model of the table files. This model is not a full "
         u"XML-Schema, this is an informal template file which describes the "
         u"expected syntax of TSDuck XML files. If --output is specified, save "
         u"the model here. Do not specify input files.");

    analyze(argc, argv);

    getValues(infiles, u"");
    getValue(outfile, u"output");
    compile = present(u"compile");
    decompile = present(u"decompile");
    xmlModel = present(u"xml-model");
    outdir = !outfile.empty() && ts::IsDirectory(outfile);
    xmlTweaks.load(*this);

    if (!infiles.empty() && xmlModel) {
        error(u"do not specify input files with --xml-model");
    }
    if (infiles.size() > 1 && !outfile.empty() && !outdir) {
        error(u"with more than one input file, --output must be a directory");
    }
    if (compile && decompile) {
        error(u"specify either --compile or --decompile but not both");
    }

    // Get default character set.
    const ts::UString csName(value(u"default-charset"));
    if (!csName.empty() && (defaultCharset = ts::DVBCharset::GetCharset(csName)) == nullptr) {
        error(u"invalid character set name '%s", {csName});
    }

    exitOnError();
}


//----------------------------------------------------------------------------
//  Display the XML model.
//----------------------------------------------------------------------------

bool DisplayModel(Options& opt)
{
    // Locate the model file.
    const ts::UString inName(ts::SearchConfigurationFile(u"tsduck.xml"));
    if (inName.empty()) {
        opt.error(u"XML model file not found");
        return false;
    }
    opt.verbose(u"original model file is %s", {inName});

    // Save to a file. Default to stdout.
    ts::UString outName(opt.outfile);
    if (opt.outdir) {
        // Specified output is a directory, add default name.
        outName.push_back(ts::PathSeparator);
        outName.append(u"tsduck.xml");
    }
    if (!outName.empty()) {
        opt.verbose(u"saving model file to %s", {outName});
    }

    // Redirect input and output, exit in case of error.
    ts::InputRedirector in(inName, opt);
    ts::OutputRedirector out(outName, opt);

    // Display / copy the XML model.
    std::cout << std::cin.rdbuf();
    return true;
}


//----------------------------------------------------------------------------
//  Process one file. Return true on success, false on error.
//----------------------------------------------------------------------------

bool ProcessFile(Options& opt, const ts::UString& infile)
{
    const ts::SectionFile::FileType inType = ts::SectionFile::GetFileType(infile);
    const bool compile = opt.compile || inType == ts::SectionFile::XML;
    const bool decompile = opt.decompile || inType == ts::SectionFile::BINARY;
    const ts::SectionFile::FileType outType = compile ? ts::SectionFile::BINARY : ts::SectionFile::XML;

    // Compute output file name with default file type.
    ts::UString outname(opt.outfile);
    if (outname.empty()) {
        outname = ts::SectionFile::BuildFileName(infile, outType);
    }
    else if (opt.outdir) {
        outname += ts::PathSeparator + ts::SectionFile::BuildFileName(ts::BaseName(infile), outType);
    }

    ts::SectionFile file;
    file.setTweaks(opt.xmlTweaks.tweaks());

    ts::ReportWithPrefix report(opt, ts::BaseName(infile) + u": ");

    // Process the input file, starting with error cases.
    if (!compile && !decompile) {
        opt.error(u"don't know what to do with file %s, unknown file type, specify --compile or --decompile", {infile});
        return false;
    }
    else if (compile && inType == ts::SectionFile::BINARY) {
        opt.error(u"cannot compile binary file %s", {infile});
        return false;
    }
    else if (decompile && inType == ts::SectionFile::XML) {
        opt.error(u"cannot decompile XML file %s", {infile});
        return false;
    }
    else if (compile) {
        // Load XML file and save binary sections.
        opt.verbose(u"Compiling %s to %s", {infile, outname});
        return file.loadXML(infile, report, opt.defaultCharset) && file.saveBinary(outname, report);
    }
    else {
        // Load binary sections and save XML file.
        opt.verbose(u"Decompiling %s to %s", {infile, outname});
        return file.loadBinary(infile, report, ts::CRC32::CHECK) && file.saveXML(outname, report, opt.defaultCharset);
    }
}


//----------------------------------------------------------------------------
//  Program entry point
//----------------------------------------------------------------------------

int MainCode(int argc, char *argv[])
{
    Options opt(argc, argv);
    bool ok = true;
    if (opt.xmlModel) {
        ok = DisplayModel(opt);
    }
    else {
        for (size_t i = 0; i < opt.infiles.size(); ++i) {
            if (!opt.infiles[i].empty()) {
                ok = ProcessFile(opt, opt.infiles[i]) && ok;
            }
        }
    }
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

TS_MAIN(MainCode)
