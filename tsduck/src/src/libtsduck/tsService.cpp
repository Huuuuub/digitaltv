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

#include "tsService.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------------

ts::Service::Service() :
    _id(),
    _tsid(),
    _onid(),
    _pmt_pid(),
    _lcn(),
    _type(),
    _name(),
    _provider(),
    _eits_present(),
    _eitpf_present(),
    _ca_controlled(),
    _running_status()
{
}

ts::Service::Service(uint16_t id) :
    Service()
{
    _id = id;
}

ts::Service::Service(const UString& desc) :
    Service()
{
    set(desc);
}


//----------------------------------------------------------------------------
// Reset using a string description.
// If the string evaluates to an integer (decimal or hexa),
// this is a service id, otherwise this is a service name.
//----------------------------------------------------------------------------

void ts::Service::set(const UString& desc)
{
    clear();

    uint16_t id = 0;
    if (desc.toInteger(id)) {
        _id = id;
    }
    else if (!desc.empty()) {
        _name = desc;
    }
}


//----------------------------------------------------------------------------
// Clear all fields
//----------------------------------------------------------------------------

void ts::Service::clear()
{
    _id.reset();
    _tsid.reset();
    _onid.reset();
    _pmt_pid.reset();
    _lcn.reset();
    _type.reset();
    _name.reset();
    _provider.reset();
    _eits_present.reset();
    _eitpf_present.reset();
    _ca_controlled.reset();
    _running_status.reset();
}


//----------------------------------------------------------------------------
// List of fields which are set in a Service
//----------------------------------------------------------------------------

uint32_t ts::Service::getFields() const
{
    uint32_t fields = 0;
    if (_id.set()) {
        fields |= ID;
    }
    if (_tsid.set()) {
        fields |= TSID;
    }
    if (_onid.set()) {
        fields |= ONID;
    }
    if (_pmt_pid.set()) {
        fields |= PMT_PID;
    }
    if (_lcn.set()) {
        fields |= LCN;
    }
    if (_type.set()) {
        fields |= TYPE;
    }
    if (_name.set()) {
        fields |= NAME;
    }
    if (_provider.set()) {
        fields |= PROVIDER;
    }
    if (_eits_present.set()) {
        fields |= EITS;
    }
    if (_eitpf_present.set()) {
        fields |= EITPF;
    }
    if (_ca_controlled.set()) {
        fields |= CA;
    }
    if (_running_status.set()) {
        fields |= RUNNING;
    }
    return fields;
}


//----------------------------------------------------------------------------
// Sorting criteria
//----------------------------------------------------------------------------

// Sort macro according to one field: If both objects have this field set,
// sort according to this field. If only one object has this field set,
// it comes first. If none of the two objects have this field set, move
// to next criterion.
#define _SORT_(field)                                                               \
    if (s1.field.set() && !s2.field.set()) {                                        \
        return true;                                                                \
    }                                                                               \
    if (!s1.field.set() && s2.field.set()) {                                        \
        return false;                                                               \
    }                                                                               \
    if (s1.field.set() && s2.field.set() && s1.field.value() != s2.field.value()) { \
        return s1.field.value() < s2.field.value();                                 \
    }

// Sort1: LCN, ONId, TSId, Id, name, provider, type, PMT PID
bool ts::Service::Sort1 (const Service& s1, const Service& s2)
{
    _SORT_(_lcn);
    _SORT_(_onid);
    _SORT_(_tsid);
    _SORT_(_id);
    _SORT_(_name);
    _SORT_(_provider);
    _SORT_(_type);
    _SORT_(_pmt_pid);
    return true; // Default: remain stable
}

// Sort2: name, provider, LCN, ONId, TSId, Id, type, PMT PID
bool ts::Service::Sort2 (const Service& s1, const Service& s2)
{
    _SORT_(_name);
    _SORT_(_provider);
    _SORT_(_lcn);
    _SORT_(_onid);
    _SORT_(_tsid);
    _SORT_(_id);
    _SORT_(_type);
    _SORT_(_pmt_pid);
    return true; // Default: remain stable
}

// Sort3: ONId, TSId, Id, type, name, provider, LCN, PMT PID
bool ts::Service::Sort3 (const Service& s1, const Service& s2)
{
    _SORT_(_onid);
    _SORT_(_tsid);
    _SORT_(_id);
    _SORT_(_type);
    _SORT_(_name);
    _SORT_(_provider);
    _SORT_(_lcn);
    _SORT_(_pmt_pid);
    return true; // Default: remain stable
}
