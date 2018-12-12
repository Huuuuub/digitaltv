#!/bin/bash
#-----------------------------------------------------------------------------
#
#  TSDuck - The MPEG Transport Stream Toolkit
#  Copyright (c) 2005-2018, Thierry Lelegard
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGE.
#
#-----------------------------------------------------------------------------
#
#  This script builds the TSDuck installers on a remote system. The remote
#  system can be a running physical machine or a local VM. In the case of
#  a local VM, it is booted and shutdown if it does not already runs.
#  Currently supports VMWare only as hypervisor for local VM's.
#  Remotely built installers are copied into the local installers directory.
#
#  Sample user's script to build TSDuck binaries on four platforms, one
#  physical Raspberry Pi and three VMWare virtual machines:
#
#    $HOME/tsduck/build/build-remote.sh --host raspberry
#    $HOME/tsduck/build/build-remote.sh --host vmwindows --vmware $HOME/VM/Windows.vmwarevm/Windows.vmx --windows
#    $HOME/tsduck/build/build-remote.sh --host vmfedora  --vmware $HOME/VM/Fedora.vmwarevm/Fedora.vmx
#    $HOME/tsduck/build/build-remote.sh --host vmubuntu  --vmware $HOME/VM/Ubuntu.vmwarevm/Ubuntu.vmx
#
#-----------------------------------------------------------------------------

SCRIPT=$(basename $BASH_SOURCE)
ROOTDIR=$(cd $(dirname $BASH_SOURCE)/..; pwd)

error() { echo >&2 "$SCRIPT: $*"; exit 1; }
usage() { echo >&2 "invalid command, try \"$SCRIPT --help\""; exit 1; }

# Default values for command line options.

REMOTE_DIR=tsduck
REMOTE_WIN=false
USER_NAME=$(id -un)
HOST_NAME=
VMX_FILE=
SSH_TIMEOUT=5
BOOT_TIMEOUT=120


#-----------------------------------------------------------------------------
# Display help text
#-----------------------------------------------------------------------------

showhelp()
{
    cat >&2 <<EOF

Build the TSDuck installers on a remote system and get them back.

Usage: $SCRIPT [options]

Options:

  -d path
  --directory path
      Relative path of the tsduck repository on the remote host, from the login
      directory of the remote user. Default: $REMOTE_DIR

  --help
      Display this help text.

  -h name
  --host name
      Build on this remote host. Mandatory for remote hosts. Optional for VM's.

  -t seconds
  --timeout seconds
      Connexion timeout for ssh and scp. Default: $SSH_TIMEOUT seconds.

  -u name
  --user name
      User name to use with ssh on the remote host. Default: $USER_NAME

  --vmware filename
      Use the VMWare virtual machine from the specified .vmx file. If the VM
      is not currently running, it is booted first and shut down after building
      the installers.

  -w
  --windows
      Specify that the remote host is a Windows host. Default: Unix

EOF
    exit 1
}


#-----------------------------------------------------------------------------
# Decode command line arguments
#-----------------------------------------------------------------------------

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d|--directory)
            [[ $# -gt 1 ]] || usage; shift
            REMOTE_DIR=$1
            ;;
        --help)
            showhelp
            ;;
        -h|--host)
            [[ $# -gt 1 ]] || usage; shift
            HOST_NAME=$1
            ;;
        -t|--timeout)
            [[ $# -gt 1 ]] || usage; shift
            SSH_TIMEOUT=$1
            ;;
        -u|--user)
            [[ $# -gt 1 ]] || usage; shift
            USER_NAME=$1
            ;;
        --vmware)
            [[ $# -gt 1 ]] || usage; shift
            VMX_FILE=$1
            ;;
        -w|--windows)
            REMOTE_WIN=true
            ;;
        *)
            usage
            ;;
    esac
    shift
done


#-----------------------------------------------------------------------------
# Remote build.
#-----------------------------------------------------------------------------

SSH_OPTS="-o ConnectTimeout=$SSH_TIMEOUT"
NAME="$HOST_NAME"

curdate()  { date +%Y%m%d-%H%M; }

# Process VMWare startup.
VMX_SHUTDOWN=false
if [[ -n "$VMX_FILE" ]]; then

    # Locate vmrun, the VMWare command line.
    # Try in PATH and then VMWare Fusion installation path on macOS.
    VMRUN=$(which vmrun 2>/dev/null)
    [[ -z "$VMRUN" ]] && VMRUN=$(ls "/Applications/VMware Fusion.app/Contents/Public/vmrun" 2>/dev/null)
    [[ -z "$VMRUN" ]] && error "vmrun not found, cannot manage VMWare VM's"

    # Name for log file.
    [[ -z "$HOST_NAME" ]] && NAME=$(basename "$VMX_FILE" .vmx | tr A-Z a-z)

    # Try to get IP address of VM.
    IP=$("$VMRUN" getGuestIPAddress "$VMX_FILE")
    if [[ $? -ne 0 ]]; then

        # Cannot get VM IP address, try to boot the VM.
        echo Booting $(basename "$VMX_FILE")
        "$VMRUN" start "$VMX_FILE" nogui || error "cannot start VM in $VMX_FILE"

        # Wait until we can get the IP address of the VM.
        if [[ -z "$HOST_NAME" ]]; then
            maxdate=$(( $(date +%s) + $BOOT_TIMEOUT ))
            ok=1
            while [[ $(date +%s) -lt $maxdate ]]; do
                IP=$("$VMRUN" getGuestIPAddress "$VMX_FILE")
                ok=$?
                [[ $ok -eq 0 ]] && break
                sleep 5
            done
            [[ $ok -ne 0 ]] && error "Cannot get IP address of $HOST_NAME after $BOOT_TIMEOUT seconds"
            echo "IP address for $NAME is $IP, trying to ssh..."
            HOST_NAME="$IP"
        fi

        # Wait that the machine is accessible using ssh.
        # Don't wait once for boot timeout, sometimes it hangs.
        maxdate=$(( $(date +%s) + $BOOT_TIMEOUT ))
        ok=1
        while [[ $(date +%s) -lt $maxdate ]]; do
            ssh -o ConnectTimeout=5 "$HOST_NAME" cd &>/dev/null
            ok=$?
            [[ $ok -eq 0 ]] && break
            sleep 5
        done
        [[ $ok -ne 0 ]] && error "cannot contact VM $BOOT_TIMEOUT seconds after boot, aborting"
        echo "SSH ok for $HOST_NAME"

        # We need to shutdown the VM after building the installers.
        VMX_SHUTDOWN=true
    fi

    # Use the IP address if no host name is provided.
    [[ -z "$HOST_NAME" ]] && HOST_NAME="$IP"
fi

# Check accessibility of remote host.
[[ -z "$HOST_NAME" ]] && error "no remote host specified"
ssh $SSH_OPTS "$HOST_NAME" cd &>/dev/null || error "$HOST_NAME not responding"

# Build remote installers.
(
    if $REMOTE_WIN; then
        # Build on Windows.
        # Cleanup repository, rebuild from scratch.
        ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" PowerShell \
            ". '$REMOTE_DIR/build/Cleanup.ps1' -NoPause"

        # Create a remote timestamp in installers subdirectory.
        # Newer files will be the installers we build.
        ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" PowerShell \
            "[void](New-Item -Type File '$REMOTE_DIR/installers/timestamp.tmp' -Force)"

        # Build installers after updating the repository.
        ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" PowerShell \
            ". '$REMOTE_DIR/build/Build-Installer.ps1' -GitPull -NoSource -NoPause"

        # Get all files from installers directory which are newer than the timestamp.
        files=$(ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" PowerShell \
            "Get-ChildItem '$REMOTE_DIR/installers' |
             Where-Object { \$_.LastWriteTime -gt (Get-Item '$REMOTE_DIR/installers/timestamp.tmp').LastWriteTime } |
             ForEach-Object { \$_.Name }" | tr '\r' ' ')

        # Copy all installers files.
        for f in $files; do
            echo "Fetching $f"
            scp $SSH_OPTS "$USER_NAME@$HOST_NAME:$REMOTE_DIR/installers/$f" "$ROOTDIR/installers/"
        done

        # Delete the temporary timestamp.
        ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" PowerShell \
            "[void](Remove-Item -Force '$REMOTE_DIR/installers/timestamp.tmp' -ErrorAction Ignore)"
    else
        # Build on Unix.
        # Create a remote timestamp. Newer files will be the installers we build.
        # Build installers from scratch after updating the repository.
        ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" \
            cd "$REMOTE_DIR" '&&' \
            git pull '&&' \
            make distclean ';' \
            touch "installers/timestamp.tmp" '&&' \
            make installer

        # Get all files from installers directory which are newer than the time stamp.
        files=$(ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" \
            find "$REMOTE_DIR/installers" -maxdepth 1 -type f -newer "$REMOTE_DIR/installers/timestamp.tmp" -printf "'%f '")

        # Copy all files from installers directory which are newer than the time stamp.
        for f in $files; do
            echo "Fetching $f"
            scp $SSH_OPTS "$USER_NAME@$HOST_NAME:$REMOTE_DIR/installers/$f" "$ROOTDIR/installers/"
        done

        # Delete the temporary timestamp.
        ssh $SSH_OPTS "$USER_NAME@$HOST_NAME" rm -f "$REMOTE_DIR/installers/timestamp.tmp"
    fi
) &>"$ROOTDIR/installers/build-${HOST_NAME}-$(curdate).log"


# Shutdown the VM if we booted it.
if $VMX_SHUTDOWN; then
    echo "Shutting down" $(basename "$VMX_FILE")
    "$VMRUN" stop "$VMX_FILE" soft
fi

exit 0
