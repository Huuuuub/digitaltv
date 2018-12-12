﻿#-----------------------------------------------------------------------------
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

<#
 .SYNOPSIS

  Download, expand and install the Dektec DTAPI for Windows.

 .PARAMETER ForceDownload

  Force a download even if the DTAPI is already downloaded.

 .PARAMETER NoInstall

  Do not install the DTAPI package. By default, the API and the drivers are
  installed.

 .PARAMETER NoPause

  Do not wait for the user to press <enter> at end of execution. By default,
  execute a "pause" instruction at the end of execution, which is useful
  when the script was run from Windows Explorer.
#>
[CmdletBinding(SupportsShouldProcess=$true)]
param(
    [switch]$ForceDownload = $false,
    [switch]$NoInstall = $false,
    [switch]$NoPause = $false
)

$DektecUrl = "http://www.dektec.com/downloads/SDK/"
$DtapiInstaller = "DekTec SDK - Windows Setup.exe"

# Local file names.
$RootDir = $PSScriptRoot

# A function to exit this script.
function Exit-Script([string]$Message = "")
{
    $Code = 0
    if ($Message -ne "") {
        Write-Host "ERROR: $Message"
        $Code = 1
    }
    if (-not $NoPause) {
        pause
    }
    exit $Code
}

# Get the HTML page for Dektec SDK downloads.
$status = 0
$message = ""
try {
    $response = Invoke-WebRequest $DektecUrl
    $status = [int] [Math]::Floor($response.StatusCode / 100)
}
catch {
    $message = $_.Exception.Message
}
if ($status -ne 1 -and $status -ne 2) {
    if ($message -eq "" -and (Test-Path variable:response)) {
        Exit-Script "Status code $($response.StatusCode), $($response.StatusDescription)"
    }
    else {
        Exit-Script "#### Error accessing ${DektecUrl}: $message"
    }
}

# Parse HTML page to locate the WinSDK file.
$sdk = $response.ParsedHtml.getElementsByTagName("a") | Where-Object { $_.href -like "*/WinSDK*.zip" } | Select-Object -First 1
$sdkRef = $sdk.href -replace '^about:',''

# Build the absolute URL from base URL (the download page) and href link.
$DtapiUrl = (New-Object -TypeName 'System.Uri' -ArgumentList ([System.Uri]$DektecUrl),$sdkref)
$DtapiZipName = (Split-Path -Leaf $DtapiUrl.toString())
$DtapiZipFile = (Join-Path $RootDir $DtapiZipName)
$DtapiDir = (Join-Path $RootDir ([io.fileinfo] $DtapiZipName).BaseName)
$DtapiSetup = (Join-Path $DtapiDir $DtapiInstaller)

# Download WinSDK.zip
if (-not $ForceDownload -and (Test-Path $DtapiZipFile)) {
    Write-Output "$DtapiZipName already downloaded, use -ForceDownload to download again"
}
else {
    Write-Output "Downloading $DtapiUrl ..."
    Invoke-WebRequest -Uri $DtapiUrl -OutFile $DtapiZipFile
}
if (-not (Test-Path $DtapiZipFile)) {
    Exit-Script "$DtapiZipName download failed"
}

# Extract archive.
Write-Output $DtapiDir
if (Test-Path $DtapiDir) {
    Write-Output "Cleaning up previous $DtapiDir"
    Remove-Item $DtapiDir -Recurse -Force
}
Write-Output "Expanding DTAPI to $DtapiDir ..."
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory($DtapiZipFile, $DtapiDir)

# Install the DTAPI.
if (-not $NoInstall) {
    if (-not (Test-Path $DtapiSetup)) {
        Exit-Script "$DtapiSetup not found, cannot install DTAPI"
    }
    . $DtapiSetup
}

Exit-Script
