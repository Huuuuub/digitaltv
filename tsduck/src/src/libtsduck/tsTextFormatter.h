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
//!  Format and print a text document.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsNullReport.h"
#include "tsAlgorithm.h"

namespace ts {
    //!
    //! Format and print a text document using various output types and indentation.
    //! @ingroup cpp
    //!
    //! This class is used to format XML documents or other types of structured text output.
    //! If is a subclass of <code>std::ostream</code> and can be used as any output stream.
    //! It also defines additional I/O manipulators to handle indentation.
    //!
    class TSDUCKDLL TextFormatter:
        public std::basic_ostream<char>,     // Public base
        private std::basic_streambuf<char>   // Internally use a streambuf
    {
    public:
        //!
        //! Explicit reference to the public superclass.
        //!
        typedef std::basic_ostream<char> SuperClass;

        // These types are declared by std::basic_ostream and are inherited.
        // But the same names are also declared by the private base class basic_streambuf.
        // Because of this conflict, they are hidden. We restore here the visibility
        // of the names which are inherited by the public base class.
#if !defined(DOXYGEN)
        typedef SuperClass::char_type char_type;
        typedef SuperClass::traits_type traits_type;
        typedef SuperClass::int_type int_type;
        typedef SuperClass::pos_type pos_type;
        typedef SuperClass::off_type off_type;
#endif

        //!
        //! Constructor.
        //! @param [in,out] report Where to report errors.
        //!
        explicit TextFormatter(Report& report = NULLREP);

        //!
        //! Destructor.
        //!
        virtual ~TextFormatter() override;

        //!
        //! Get the margin size for outer-most elements.
        //! @return The margin size for outer-most elements.
        //!
        size_t marginSize() const { return _margin; }

        //!
        //! Set the margin size for outer-most elements.
        //! @param [in] margin The margin size for outer-most elements.
        //! @return A reference to this object.
        //! @see I/O manipulator ts::margin(size_t)
        //!
        TextFormatter& setMarginSize(size_t margin);

        //!
        //! Get the indent size for inner elements.
        //! @return The indent size for inner elements.
        //!
        size_t indentSize() const { return _indent; }

        //!
        //! Set the indent size for inner elements.
        //! @param [in] indent The indent size for inner elements.
        //! @return A reference to this object.
        //!
        TextFormatter& setIndentSize(size_t indent) { _indent = indent; return *this; }

        //!
        //! Set output to an open text stream.
        //! @param [in,out] strm The output text stream.
        //! The referenced stream object must remain valid as long as this object.
        //! @return A reference to this object.
        //!
        TextFormatter& setStream(std::ostream& strm);

        //!
        //! Set output to a text file.
        //! @param [in] fileName Output file name.
        //! @return True on success, false on error.
        //!
        bool setFile(const UString& fileName);

        //!
        //! Set output to an internal string buffer.
        //! @return A reference to this object.
        //! @see getString()
        //!
        TextFormatter& setString();

        //!
        //! Retrieve the current content of the internal string buffer.
        //! Must be called after setString() and before close().
        //! @param [out] str Returned string containing the formatted document.
        //! @return True on success, false if there is no internal string buffer.
        //! @see setString()
        //!
        bool getString(UString& str);

        //!
        //! Return the current content of the internal string buffer.
        //! Must be called after setString() and before close().
        //! @return The string containing the formatted document.
        //! @see getString()
        //!
        UString toString();

        //!
        //! Check if the Output is open to some output.
        //! @return True if the Output is open.
        //!
        bool isOpen() const;

        //!
        //! Close the current output.
        //! Depending on the output mode:
        //! - The external stream is no longer referenced.
        //! - The external file is closed.
        //! - The internal string buffer is emptied.
        //!
        void close();

        //!
        //! Insert all necessary new-lines and spaces to move to the current margin.
        //! @return A reference to this object.
        //! @see I/O manipulator ts::margin()
        //!
        TextFormatter& margin();

        //!
        //! Insert all necessary new-lines and spaces to move to a given column.
        //! @param [in] col The column position to move to. The first character of a line is at column 0.
        //! @return A reference to this object.
        //! @see I/O manipulator ts::column(size_t)
        //!
        TextFormatter& column(size_t col);

        //!
        //! Output spaces on the stream.
        //! @param [in] count Number of spaces to print.
        //! @return A reference to this object.
        //! @see I/O manipulator ts::spaces(size_t)
        //!
        TextFormatter& spaces(size_t count);

        //!
        //! Push one indentation level, typically when formatting child items.
        //! @return A reference to this object.
        //! @see I/O manipulator ts::ident()
        //!
        TextFormatter& indent()
        {
            _curMargin += _indent;
            return*this;
        }

        //!
        //! Pop one indentation level, typically when formatting back to parent.
        //! @return A reference to this object.
        //! @see I/O manipulator ts::unident()
        //!
        TextFormatter& unindent()
        {
            _curMargin -= std::min(_curMargin, _indent);
            return*this;
        }

    private:
        Report&            _report;      // Where to report errors.
        std::ofstream      _outFile;     // Own stream when output to a file we created.
        std::ostringstream _outString;   // Internal string buffer.
        std::ostream*      _out;         // Address of current output stream.
        size_t             _margin;      // Margin size for outer-most element.
        size_t             _indent;      // Indent size for inner elements.
        size_t             _curMargin;   // Current margin size.
        size_t             _tabSize;     // Tabulation size in characters.
        size_t             _column;      // Current column in line, starting at 0.
        bool               _afterSpace;  // After initial spaces in line.
        std::string        _buffer;      // Internal buffer for std::streambuf

        // Inherited from std::basic_streambuf<char>.
        // This is called when buffer becomes full.
        // If buffer is not used, then this is called every time when characters are put to stream.
        virtual int overflow(int c = traits_type::eof()) override;

        // Inherited from std::basic_streambuf<char>.
        // This function is called when stream is flushed, for example when std::endl is put to stream.
        virtual int sync() override;

        // Flush all data in buffer to underlying output.
        bool flushBuffer()
        {
            return flushData(pbase(), pptr());
        }

        // Flush data to underlying output.
        bool flushData(const char* firstAddr, const char* lastAddr);

        // Reset buffer, make it fully available to std::streambuf.
        void resetBuffer()
        {
            setp(&_buffer[0], &_buffer[0] + _buffer.size());
        }

        // Inaccessible operations.
        TextFormatter(const TextFormatter&) = delete;
        TextFormatter& operator=(const TextFormatter&) = delete;
    };

    //!
    //! I/O manipulator for TextFormatter: move to the current margin.
    //! @param [in,out] os Output stream.
    //! @return A reference to @a os.
    //! @see TextFormatter::margin()
    //!
    TSDUCKDLL inline std::ostream& margin(std::ostream& os)
    {
        return IOManipulator(os, &TextFormatter::margin);
    }

    //!
    //! I/O manipulator for TextFormatter: push one indentation level.
    //! @param [in,out] os Output stream.
    //! @return A reference to @a os.
    //! @see TextFormatter::indent()
    //!
    TSDUCKDLL inline std::ostream& indent(std::ostream& os)
    {
        return IOManipulator(os, &TextFormatter::indent);
    }

    //!
    //! I/O manipulator for TextFormatter: pop one indentation level.
    //! @param [in,out] os Output stream.
    //! @return A reference to @a os.
    //! @see TextFormatter::unindent()
    //!
    TSDUCKDLL inline std::ostream& unindent(std::ostream& os)
    {
        return IOManipulator(os, &TextFormatter::unindent);
    }

    //!
    //! I/O manipulator for TextFormatter: set the margin size for outer-most elements.
    //! @param [in] size The margin size for outer-most elements.
    //! @return An I/O manipulator proxy.
    //! @see TextFormatter::setMarginSize(size_t)
    //!
    TSDUCKDLL IOManipulatorProxy<TextFormatter, size_t> margin(size_t size);

    //!
    //! I/O manipulator for TextFormatter: output spaces on the stream.
    //! @param [in] count Number of spaces to print.
    //! @return An I/O manipulator proxy.
    //! @see TextFormatter::setMarginSize(size_t)
    //!
    TSDUCKDLL IOManipulatorProxy<TextFormatter, size_t> spaces(size_t count);

    //!
    //! I/O manipulator for TextFormatter: move to a given column.
    //! @param [in] col The column position to move to. The first character of a line is at column 0.
    //! @return An I/O manipulator proxy.
    //! @see TextFormatter::column(size_t)
    //!
    TSDUCKDLL IOManipulatorProxy<TextFormatter, size_t> column(size_t col);
}
