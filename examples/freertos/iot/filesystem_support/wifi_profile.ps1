# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

$outFile = Join-Path -Path $PSScriptRoot -ChildPath "networks.dat"
$offset = 0
$maxPassLen = 32
$maxSsidLen = 32

# Offsets
$ssidOffset = 0
$ssidLenOffset = $ssidOffset + $maxSsidLen + 1
$bssidOffset = $ssidLenOffset + 1
$passOffset = $bssidOffset + 6
$passLenOffset = $passOffset + $maxPassLen + 1
$secOffset = $passLenOffset + 1
$secOffset = $secOffset + ($secOffset % 4) # Apply DWORD alignment

# Size of one WiFi Entry
$entrySize = $secOffset + 4

$enc = [System.Text.Encoding]::ASCII

[byte[]]$dat = New-Object byte[] $entrySize
[byte[]]$bssid = @(0,0,0,0,0,0)

if (Test-Path $outFile) {
    Write-Output "WiFi profile ($outFile) already exists."

    $reconfig = Read-Host -Prompt 'Reconfigure? y/[n]'

    if ($reconfig.ToLower() -ne 'y') {
        exit 0
    }
}

while ($true) {
    while ($true) {
        $ssid = Read-Host -Prompt 'Enter the WiFi network SSID'
        if (($ssid.Length -gt 0) -and ($ssid.Length -le $maxSsidLen)) {
            break
        }
    }

    while ($true) {
        $passSecure = Read-Host -AsSecureString -Prompt 'Enter the WiFi network password'
        if ($passSecure.Length -le $maxSsidLen) {
            break
        }
    }

    do {
        $security = [Int32](Read-Host -Prompt 'Enter the security (0=open, 1=WEP, 2=WPA)')
    } while ($security -lt 0 || $security -gt 2)

    # Copy data into byte array
    $enc.GetBytes($ssid).CopyTo($dat, $offset + $ssidOffset)
    [BitConverter]::GetBytes(([byte]$ssid.Length)).CopyTo($dat, $offset + $ssidLenOffset)
    $bssid.CopyTo($dat, $offset + $bssidOffset)
    $passPlainText = ConvertFrom-SecureString $passSecure -AsPlainText
    $enc.GetBytes($passPlainText).CopyTo($dat, $offset + $passOffset)
    [BitConverter]::GetBytes([byte]$passPlainText.Length).CopyTo($dat, $offset + $passLenOffset)
    [BitConverter]::GetBytes($security).CopyTo($dat, $offset + $secOffset)

    $more = Read-Host -Prompt 'Add another WiFi network? y/[n]'

    if ($more.ToLower() -ne 'y') {
        break
    }

    $newDat = New-Object byte[] ($dat.Length + $entrySize)
    $dat.CopyTo($newDat, 0)
    $dat = $newDat
    $offset += $entrySize
}

[IO.File]::WriteAllBytes($outFile, $dat)
