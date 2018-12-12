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
//  UDP Socket
//
//----------------------------------------------------------------------------

#include "tsUDPSocket.h"
#include "tsNullReport.h"
TSDUCK_SOURCE;

// Furiously idiotic Windows feature, see comment in receiveOne()
#if defined(TS_WINDOWS)
volatile ::LPFN_WSARECVMSG ts::UDPSocket::_wsaRevcMsg = 0;
#endif


//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

ts::UDPSocket::UDPSocket(bool auto_open, Report& report) :
    Socket(),
    _local_address(),
    _default_destination(),
    _mcast(),
    _ssmcast()
{
    if (auto_open) {
        // Returned value ignored on purpose, the socket is marked as closed in the object on error.
        // coverity[CHECKED_RETURN]
        open(report);
    }
}


//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------

ts::UDPSocket::~UDPSocket()
{
    UDPSocket::close(NULLREP);
}


//----------------------------------------------------------------------------
// Open the socket
// Return true on success, false on error.
//----------------------------------------------------------------------------

bool ts::UDPSocket::open(Report& report)
{
    // Create a datagram socket.
    if (!createSocket(PF_INET, SOCK_DGRAM, IPPROTO_UDP, report)) {
        return false;
    }

    // Set the IP_PKTINFO option. This option is used to get the destination address of all
    // UDP packets arriving on this socket. Actual socket option is an int.
    int opt = 1;
    if (::setsockopt(getSocket(), IPPROTO_IP, IP_PKTINFO, TS_SOCKOPT_T(&opt), sizeof(opt)) != 0) {
        report.error(u"error setting socket IP_PKTINFO option: %s", {SocketErrorCodeMessage()});
        return false;
    }

    return true;
}


//----------------------------------------------------------------------------
// Close the socket
//----------------------------------------------------------------------------

bool ts::UDPSocket::close(Report& report)
{
    // Leave all multicast groups.
    if (isOpen()) {
        dropMembership(report);
    }

    // Close socket
    return Socket::close(report);
}


//----------------------------------------------------------------------------
// Bind to a local address and port.
// Return true on success, false on error.
//----------------------------------------------------------------------------

bool ts::UDPSocket::bind(const SocketAddress& addr, Report& report)
{
    ::sockaddr sock_addr;
    addr.copy(sock_addr);

    report.debug(u"binding socket to %s", {addr});
    if (::bind(getSocket(), &sock_addr, sizeof(sock_addr)) != 0) {
        report.error(u"error binding socket to local address: %s", {SocketErrorCodeMessage()});
        return false;
    }

    // Keep a cached value of the bound local address.
    return getLocalAddress(_local_address, report);
}


//----------------------------------------------------------------------------
// Set outgoing local address for multicast messages.
// Return true on success, false on error.
//----------------------------------------------------------------------------

bool ts::UDPSocket::setOutgoingMulticast(const UString& name, Report& report)
{
    IPAddress addr;
    return addr.resolve(name, report) && setOutgoingMulticast(addr, report);
}

bool ts::UDPSocket::setOutgoingMulticast(const IPAddress& addr, Report& report)
{
    ::in_addr iaddr;
    addr.copy(iaddr);

    if (::setsockopt(getSocket(), IPPROTO_IP, IP_MULTICAST_IF, TS_SOCKOPT_T(&iaddr), sizeof(iaddr)) != 0) {
        report.error(u"error setting outgoing local address: " + SocketErrorCodeMessage());
        return false;
    }
    return true;
}


//----------------------------------------------------------------------------
// Set a default destination address and port for outgoing messages.
// Both address and port are mandatory in socket address.
// Return true on success, false on error.
//----------------------------------------------------------------------------

bool ts::UDPSocket::setDefaultDestination(const UString& name, Report& report)
{
    SocketAddress addr;
    return addr.resolve(name, report) && setDefaultDestination(addr, report);
}

bool ts::UDPSocket::setDefaultDestination(const SocketAddress& addr, Report& report)
{
    if (!addr.hasAddress()) {
        report.error(u"missing IP address in UDP destination");
        return false;
    }
    else if (!addr.hasPort()) {
        report.error(u"missing port number in UDP destination");
        return false;
    }
    else {
        _default_destination = addr;
        return true;
    }
}


//----------------------------------------------------------------------------
// Set the Time To Live (TTL) option.
//----------------------------------------------------------------------------

bool ts::UDPSocket::setTTL(int ttl, bool multicast, Report& report)
{
    if (multicast) {
        TS_SOCKET_MC_TTL_T mttl = (TS_SOCKET_MC_TTL_T) (ttl);
        if (::setsockopt(getSocket(), IPPROTO_IP, IP_MULTICAST_TTL, TS_SOCKOPT_T (&mttl), sizeof(mttl)) != 0) {
            report.error(u"socket option multicast TTL: " + SocketErrorCodeMessage());
            return false;
        }
    }
    else {
        TS_SOCKET_TTL_T uttl = (TS_SOCKET_TTL_T) (ttl);
        if (::setsockopt(getSocket(), IPPROTO_IP, IP_TTL, TS_SOCKOPT_T (&uttl), sizeof(uttl)) != 0) {
            report.error(u"socket option unicast TTL: " + SocketErrorCodeMessage());
            return false;
        }
    }
    return true;
}


//----------------------------------------------------------------------------
// Set the Type Of Service (TOS) option.
//----------------------------------------------------------------------------

bool ts::UDPSocket::setTOS(int tos, Report& report)
{
    TS_SOCKET_TOS_T utos = (TS_SOCKET_TOS_T) (tos);
    if (::setsockopt(getSocket(), IPPROTO_IP, IP_TOS, TS_SOCKOPT_T(&utos), sizeof(utos)) != 0) {
        report.error(u"socket option TOS: " + SocketErrorCodeMessage());
        return false;
    }
    return true;
}


//----------------------------------------------------------------------------
// Enable or disable the broadcast option.
//----------------------------------------------------------------------------

bool ts::UDPSocket::setBroadcast(bool on, Report& report)
{
    int enable = int(on);
    if (::setsockopt(getSocket(), SOL_SOCKET, SO_BROADCAST, TS_SOCKOPT_T(&enable), sizeof(enable)) != 0) {
        report.error(u"socket option broadcast: " + SocketErrorCodeMessage());
        return false;
    }
    return true;
}


//----------------------------------------------------------------------------
// Enable or disable the broadcast option, based on an IP address.
//----------------------------------------------------------------------------

bool ts::UDPSocket::setBroadcastIfRequired(const IPAddress destination, Report& report)
{
    // Get all local interfaces.
    IPAddressMaskVector locals;
    if (!GetLocalIPAddresses(locals, report)) {
        return false;
    }

    // Loop on all local addresses and set broadcast when we match a local broadcast address.
    for (auto it = locals.begin(); it != locals.end(); ++it) {
        if (destination == it->broadcastAddress()) {
            return setBroadcast(true, report);
        }
    }

    // Not a broadcast address, nothing was done.
    return true;
}


//----------------------------------------------------------------------------
// Join one multicast group on one local interface.
//----------------------------------------------------------------------------

bool ts::UDPSocket::addMembership(const IPAddress& multicast, const IPAddress& local, const IPAddress& source, Report& report)
{
    // Verbose message about joining the group.
    UString groupString;
    if (source.hasAddress()) {
        groupString = source.toString() + u"@";
    }
    groupString += multicast.toString();
    if (local.hasAddress()) {
        report.verbose(u"joining multicast group %s from local address %s", {groupString, local});
    }
    else {
        report.verbose(u"joining multicast group %s from default interface", {groupString});
    }

    // Now join the group.
    if (source.hasAddress()) {
        // Source-specific multicast (SSM).
        SSMReq req(multicast, local, source);
        if (::setsockopt(getSocket(), IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, TS_SOCKOPT_T(&req.data), sizeof(req.data)) != 0) {
            report.error(u"error adding SSM membership to %s from local address %s: %s", {groupString, local, SocketErrorCodeMessage()});
            return false;
        }
        else {
            _ssmcast.insert(req);
            return true;
        }
    }
    else {
        // Standard multicast.
        MReq req(multicast, local);
        if (::setsockopt(getSocket(), IPPROTO_IP, IP_ADD_MEMBERSHIP, TS_SOCKOPT_T(&req.data), sizeof(req.data)) != 0) {
            report.error(u"error adding multicast membership to %s from local address %s: %s", {groupString, local, SocketErrorCodeMessage()});
            return false;
        }
        else {
            _mcast.insert(req);
            return true;
        }
    }
}


//----------------------------------------------------------------------------
// Join one multicast group, let the system select the local interface.
//----------------------------------------------------------------------------

bool ts::UDPSocket::addMembershipDefault(const IPAddress& multicast, const IPAddress& source, Report& report)
{
    return addMembership(multicast, IPAddress(), source, report);
}


//----------------------------------------------------------------------------
// Join one multicast group on all local interfaces.
//----------------------------------------------------------------------------

bool ts::UDPSocket::addMembershipAll(const IPAddress& multicast, const IPAddress& source, Report& report)
{
    // There is no implicit way to listen on all interfaces.
    // If no local address is specified, we must get the list
    // of all local interfaces and send a multicast membership
    // request on each of them.

    // Get all local interfaces.
    IPAddressVector loc_if;
    if (!GetLocalIPAddresses(loc_if, report)) {
        return false;
    }

    // Add all memberships
    bool ok = true;
    for (size_t i = 0; i < loc_if.size(); ++i) {
        if (loc_if[i].hasAddress()) {
            ok = addMembership(multicast, loc_if[i], source, report) && ok;
        }
    }
    return ok;
}


//----------------------------------------------------------------------------
// Leave all multicast groups.
//----------------------------------------------------------------------------

bool ts::UDPSocket::dropMembership(Report& report)
{
    bool ok = true;

    // Drop all standard multicast groups.
    for (auto it = _mcast.begin(); it != _mcast.end(); ++it) {
        report.verbose(u"leaving multicast group %s from local address %s", {IPAddress(it->data.imr_multiaddr), IPAddress(it->data.imr_interface)});
        if (::setsockopt(getSocket(), IPPROTO_IP, IP_DROP_MEMBERSHIP, TS_SOCKOPT_T(&it->data), sizeof(it->data)) != 0) {
            report.error(u"error dropping multicast membership: %s", {SocketErrorCodeMessage()});
            ok = false;
        }
    }

    // Drop all source-specific multicast groups.
    for (auto it = _ssmcast.begin(); it != _ssmcast.end(); ++it) {
        report.verbose(u"leaving multicast group %s@%s from local address %s",
                       {IPAddress(it->data.imr_sourceaddr), IPAddress(it->data.imr_multiaddr), IPAddress(it->data.imr_interface)});
        if (::setsockopt(getSocket(), IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, TS_SOCKOPT_T(&it->data), sizeof(it->data)) != 0) {
            report.error(u"error dropping multicast membership: %s", {SocketErrorCodeMessage()});
            ok = false;
        }
    }

    _mcast.clear();
    _ssmcast.clear();

    return ok;
}


//----------------------------------------------------------------------------
// Send a message to a destination address and port.
//----------------------------------------------------------------------------

bool ts::UDPSocket::send(const void* data, size_t size, const SocketAddress& dest, Report& report)
{
    ::sockaddr addr;
    dest.copy(addr);

    if (::sendto(getSocket(), TS_SENDBUF_T(data), TS_SOCKET_SSIZE_T(size), 0, &addr, sizeof(addr)) < 0) {
        report.error(u"error sending UDP message: " + SocketErrorCodeMessage());
        return false;
    }
    return true;
}


//----------------------------------------------------------------------------
// Receive a message.
// If abort interface is non-zero, invoke it when I/O is interrupted
// (in case of user-interrupt, return, otherwise retry).
// Return true on success, false on error.
//----------------------------------------------------------------------------

bool ts::UDPSocket::receive(void* data,
                            size_t max_size,
                            size_t& ret_size,
                            SocketAddress& sender,
                            SocketAddress& destination,
                            const AbortInterface* abort,
                            Report& report)
{
    // Loop on unsollicited interrupts
    for (;;) {

        // Wait for a message.
        const SocketErrorCode err = receiveOne(data, max_size, ret_size, sender, destination, report);

        if (abort != nullptr && abort->aborting()) {
            // Aborting, no error message.
            return false;
        }
        else if (err == SYS_SUCCESS) {
            // Sometimes, we get "successful" empty message coming from nowhere. Ignore them.
            if (ret_size > 0 || sender.hasAddress()) {
                return true;
            }
        }
        else if (abort != nullptr && abort->aborting()) {
            // User-interrupt, end of processing but no error message
            return false;
        }
#if !defined(TS_WINDOWS)
        else if (err == EINTR) {
            // Got a signal, not a user interrupt, will ignore it
            report.debug(u"signal, not user interrupt");
        }
#endif
        else {
            // Abort on non-interrupt errors.
            report.error(u"error receiving from UDP socket: %s", {SocketErrorCodeMessage(err)});
            return false;
        }
    }
}


//----------------------------------------------------------------------------
// Perform one receive operation. Hide the system mud.
//----------------------------------------------------------------------------

ts::SocketErrorCode ts::UDPSocket::receiveOne(void* data, size_t max_size, size_t& ret_size, SocketAddress& sender, SocketAddress& destination, Report& report)
{
    // Clear returned values
    ret_size = 0;
    sender.clear();
    destination.clear();

    // Reserve a socket address to receive the sender address.
    ::sockaddr sender_sock;
    TS_ZERO(sender_sock);

    // Normally, this operation should be done quite easily using recvmsg.
    // On Windows, all socket operations are smoothly emulated, including
    // recvfrom, allowing a reasonable portability. However, in the specific
    // case of recvmsg, there is no equivalent but a similar - and carefully
    // incompatible - function named WSARecvMsg. Not only this function is
    // different from recvmsg, but it is also not exported from any DLL.
    // Its address must be queried dynamically. The stupid idiot who had
    // this pervert idea at Microsoft deserves to burn in hell (twice) !!

#if defined(TS_WINDOWS)

    // First, get the address of WSARecvMsg the first time we use it.
    if (_wsaRevcMsg == 0) {
        ::LPFN_WSARECVMSG funcAddress = 0;
        ::GUID guid = WSAID_WSARECVMSG;
        ::DWORD dwBytes = 0;
        const ::SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) {
            return LastSocketErrorCode();
        }
        if (::WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &funcAddress, sizeof(funcAddress), &dwBytes, 0, 0) != 0) {
            const SocketErrorCode err = LastSocketErrorCode();
            ::closesocket(sock);
            return err;
        }
        ::closesocket(sock);
        // Now update the volatile value.
        _wsaRevcMsg = funcAddress;
    }

    // Build an WSABUF pointing to the message.
    ::WSABUF vec;
    TS_ZERO(vec);
    vec.buf = reinterpret_cast<CHAR*>(data);
    vec.len = ::ULONG(max_size);

    // Reserve a buffer to receive packet ancillary data.
    ::CHAR ancil_data[1024];
    TS_ZERO(ancil_data);

    // Build a WSAMSG for WSARecvMsg.
    ::WSAMSG msg;
    TS_ZERO(msg);
    msg.name = &sender_sock;
    msg.namelen = sizeof(sender_sock);
    msg.lpBuffers = &vec;
    msg.dwBufferCount = 1; // number of WSAMSG
    msg.Control.buf = ancil_data;
    msg.Control.len = ::ULONG(sizeof(ancil_data));

    // Wait for a message.
    ::DWORD insize = 0;
    if (_wsaRevcMsg(getSocket(), &msg, &insize, 0, 0)  != 0) {
        return LastSocketErrorCode();
    }

    // Browse returned ancillary data.
    for (::WSACMSGHDR* cmsg = WSA_CMSG_FIRSTHDR(&msg); cmsg != 0; cmsg = WSA_CMSG_NXTHDR(&msg, cmsg)) {
        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
            const ::IN_PKTINFO* info = reinterpret_cast<const ::IN_PKTINFO*>(WSA_CMSG_DATA(cmsg));
            destination = SocketAddress(info->ipi_addr, _local_address.port());
        }
    }

#else
    // UNIX implementation, use a standard recvmsg sequence.

    // Build an iovec pointing to the message.
    ::iovec vec;
    TS_ZERO(vec);
    vec.iov_base = data;
    vec.iov_len = max_size;

    // Reserve a buffer to receive packet ancillary data.
    uint8_t ancil_data[1024];
    TS_ZERO(ancil_data);

    // Build a msghdr structure for recvmsg().
    ::msghdr hdr;
    TS_ZERO(hdr);
    hdr.msg_name = &sender_sock;
    hdr.msg_namelen = sizeof(sender_sock);
    hdr.msg_iov = &vec;
    hdr.msg_iovlen = 1; // number of iovec structures
    hdr.msg_control = ancil_data;
    hdr.msg_controllen = sizeof(ancil_data);

    // Wait for a message.
    TS_SOCKET_SSIZE_T insize = ::recvmsg(getSocket(), &hdr, 0);

    if (insize < 0) {
        return LastSocketErrorCode();
    }

    // Browse returned ancillary data.
    for (::cmsghdr* cmsg = CMSG_FIRSTHDR(&hdr); cmsg != nullptr; cmsg = CMSG_NXTHDR(&hdr, cmsg)) {
        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO && cmsg->cmsg_len >= sizeof(::in_pktinfo)) {
            const ::in_pktinfo* info = reinterpret_cast<const ::in_pktinfo*>(CMSG_DATA(cmsg));
            destination = SocketAddress(info->ipi_addr, _local_address.port());
        }
    }

#endif // Windows vs. UNIX

    // Successfully received a message
    ret_size = size_t(insize);
    sender = SocketAddress(sender_sock);

    return SYS_SUCCESS;
}
