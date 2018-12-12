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
//!  Input switch (tsswitch) input plugin executor thread.
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsPluginThread.h"
#include "tsMutex.h"
#include "tsCondition.h"

namespace ts {
    //!
    //! Input switch (tsswitch) namespace.
    //!
    namespace tsswitch {

        class Core;
        class Options;

        //!
        //! Execution context of a tsswitch input plugin.
        //! @ingroup plugin
        //!
        class InputExecutor : public PluginThread
        {
        public:
            //!
            //! Constructor.
            //! @param [in] index Input plugin index.
            //! @param [in,out] core Command core instance.
            //! @param [in,out] opt Command line options.
            //! @param [in,out] log Log report.
            //!
            InputExecutor(size_t index, Core& core, Options& opt, Report& log);

            //!
            //! Destructor.
            //!
            virtual ~InputExecutor();

            //!
            //! Access the shared library API.
            //! @return Address of the plugin interface.
            //!
            InputPlugin* plugin() const
            {
                return _input;
            }

            //!
            //! Tell the input executor thread to start an input session.
            //! @param [in] isCurrent True if the plugin immediately becomes the current one.
            //!
            void startInput(bool isCurrent);

            //!
            //! Tell the input executor thread to stop its input session.
            //! The thread is not terminated. It waits for another session.
            //!
            void stopInput();

            //!
            //! Notify the input executor thread that it becomes or is no longer the current input plugin.
            //! @param [in] isCurrent True if the plugin becomes the current one.
            //!
            void setCurrent(bool isCurrent);

            //!
            //! Terminate the input executor thread.
            //!
            void terminateInput();

            //!
            //! Get the area of packet to output.
            //! Indirectly called from the output plugin when it needs some packets.
            //! The input thread shall reserve this area since the output plugin
            //! will use it from another thread. When the output plugin completes
            //! its output and no longer need this area, it should call freeOutput().
            //!
            //! @param [out] first Returned address of first packet to output.
            //! @param [out] count Returned number of packets to output. Can be zero.
            //!
            void getOutputArea(TSPacket*& first, size_t& count);

            //!
            //! Free an output area which was previously returned by getOutputArea().
            //! Indirectly called from the output plugin after sending packets.
            //! @param [in] count Number of output packets to release.
            //!
            void freeOutput(size_t count);

            // Implementation of TSP. We do not use "joint termination" in tsswitch.
            virtual void useJointTermination(bool) override {}
            virtual void jointTerminate() override {}
            virtual bool useJointTermination() const override {return false;}
            virtual bool thisJointTerminated() const override {return false;}

        private:
            Core&          _core;         // Application core.
            Options&       _opt;          // Command line options.
            InputPlugin*   _input;        // Plugin API.
            const size_t   _pluginIndex;  // Index of this input plugin.
            TSPacketVector _buffer;       // Packet buffer.
            Mutex          _mutex;        // Mutex to protect all subsequent fields.
            Condition      _todo;         // Condition to signal something to do.
            bool           _isCurrent;    // This plugin is the current input one.
            bool           _outputInUse;  // The output part of the buffer is currently in use by the output plugin.
            bool           _startRequest; // Start input requested.
            bool           _stopRequest;  // Stop input requested.
            bool           _terminated;   // Terminate thread.
            size_t         _outFirst;     // Index of first packet to output in _buffer.
            size_t         _outCount;     // Number of packets to output, not always contiguous, may wrap up.

            // Implementation of Thread.
            virtual void main() override;

            // Inaccessible operations.
            InputExecutor() = delete;
            InputExecutor(const InputExecutor&) = delete;
            InputExecutor& operator=(const InputExecutor&) = delete;
        };

        //!
        //! Vector of pointers to InputExecutor.
        //!
        typedef std::vector<InputExecutor*> InputExecutorVector;
    }
}
