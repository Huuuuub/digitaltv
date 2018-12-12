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
//!  Transport stream processor: Execution context of an output plugin
//!
//----------------------------------------------------------------------------

#pragma once
#include "tspPluginExecutor.h"

namespace ts {
    namespace tsp {
        //!
        //! Execution context of a tsp output plugin.
        //! @ingroup plugin
        //!
        class OutputExecutor: public PluginExecutor
        {
        public:
            //!
            //! Constructor.
            //! @param [in,out] options Command line options for tsp.
            //! @param [in] pl_options Command line options for this plugin.
            //! @param [in] attributes Creation attributes for the thread executing this plugin.
            //! @param [in,out] global_mutex Global mutex to synchronize access to the packet buffer.
            //!
            OutputExecutor(Options* options,
                           const PluginOptions* pl_options,
                           const ThreadAttributes& attributes,
                           Mutex& global_mutex);

            //!
            //! Access the shared library API.
            //! Override ts::tsp::PluginExecutor::plugin() with a specialized returned class.
            //! @return Address of the plugin interface.
            //!
            OutputPlugin* plugin() {return _output;}

        private:
            OutputPlugin* _output;

            // Inherited from Thread
            virtual void main() override;

            // Inaccessible operations
            OutputExecutor() = delete;
            OutputExecutor(const OutputExecutor&) = delete;
            OutputExecutor& operator=(const OutputExecutor&) = delete;
        };
    }
}
