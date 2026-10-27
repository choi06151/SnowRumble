param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$BuildDirectory,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._-]*$')]
    [string]$Version,

    [string]$OutputDirectory = 'Saved/ReleasePackages'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$resolvedBuildDirectory = (Resolve-Path -LiteralPath $BuildDirectory).Path
$outputDirectoryPath = if ([System.IO.Path]::IsPathRooted($OutputDirectory)) {
    $OutputDirectory
} else {
    Join-Path $repositoryRoot $OutputDirectory
}
$resolvedOutputDirectory = [System.IO.Path]::GetFullPath($outputDirectoryPath)
$normalizedBuildDirectory = $resolvedBuildDirectory.TrimEnd('\', '/')
$buildDirectoryPrefix = $resolvedBuildDirectory.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar

if (-not (Test-Path -LiteralPath $resolvedBuildDirectory -PathType Container)) {
    throw "패키징 빌드 폴더를 찾을 수 없습니다: $resolvedBuildDirectory"
}

if ($resolvedOutputDirectory.TrimEnd('\', '/').Equals(
        $normalizedBuildDirectory,
        [System.StringComparison]::OrdinalIgnoreCase
    ) -or $resolvedOutputDirectory.StartsWith(
        $buildDirectoryPrefix,
        [System.StringComparison]::OrdinalIgnoreCase
    )) {
    throw '출력 폴더를 패키징 빌드 폴더 내부에 둘 수 없습니다.'
}

$buildItems = @(Get-ChildItem -LiteralPath $resolvedBuildDirectory -Force)
if ($buildItems.Count -eq 0) {
    throw "패키징 빌드 폴더가 비어 있습니다: $resolvedBuildDirectory"
}

if (-not (Test-Path -LiteralPath $resolvedOutputDirectory -PathType Container)) {
    New-Item -ItemType Directory -Path $resolvedOutputDirectory -Force | Out-Null
}

$archiveName = "SnowRumble-Windows-Development-$Version.zip"
$archivePath = Join-Path $resolvedOutputDirectory $archiveName
$checksumPath = "$archivePath.sha256"
$partsChecksumPath = "$archivePath.parts.sha256"
$partPaths = @()
$existingPartFiles = @(Get-ChildItem -LiteralPath $resolvedOutputDirectory -Filter "$archiveName.part*" -ErrorAction SilentlyContinue)
$hasExistingArtifacts = (Test-Path -LiteralPath $archivePath) `
    -or (Test-Path -LiteralPath $checksumPath) `
    -or (Test-Path -LiteralPath $partsChecksumPath) `
    -or ($existingPartFiles.Count -gt 0)

if ($hasExistingArtifacts) {
	throw "같은 버전의 배포 파일이 이미 있습니다. 새 버전명을 지정하세요: $Version"
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory(
    $resolvedBuildDirectory,
    $archivePath,
    [System.IO.Compression.CompressionLevel]::Optimal,
    $false
)

$maximumReleaseAssetBytes = 2GB
$hash = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath $checksumPath -Value "$hash  $archiveName" -Encoding ascii

$archiveInfo = Get-Item -LiteralPath $archivePath
if ($archiveInfo.Length -ge $maximumReleaseAssetBytes) {
	$partSizeBytes = 1900MB
	$buffer = New-Object byte[] (4MB)
	$sourceStream = [System.IO.File]::OpenRead($archivePath)
	$partIndex = 1
	$partHashes = New-Object 'System.Collections.Generic.List[string]'
	try {
		while ($sourceStream.Position -lt $sourceStream.Length) {
			$partPath = '{0}.part{1:D3}' -f $archivePath, $partIndex
			$partPaths += $partPath
			$partStream = [System.IO.File]::Create($partPath)
			try {
				$remainingBytes = [Math]::Min(
					[long]$partSizeBytes,
					$sourceStream.Length - $sourceStream.Position)
				while ($remainingBytes -gt 0) {
					$bytesToRead = [int][Math]::Min([long]$buffer.Length, $remainingBytes)
					$bytesRead = $sourceStream.Read($buffer, 0, $bytesToRead)
					if ($bytesRead -eq 0) {
						throw '압축 파일 조각을 만드는 중 원본 파일을 끝까지 읽지 못했습니다.'
					}
					$partStream.Write($buffer, 0, $bytesRead)
					$remainingBytes -= $bytesRead
				}
			} finally {
				$partStream.Dispose()
			}

			$partInfo = Get-Item -LiteralPath $partPath
			if ($partInfo.Length -ge $maximumReleaseAssetBytes) {
				throw "분할 파일이 GitHub Release의 파일 크기 제한을 넘었습니다: $partPath"
			}
			$partHash = (Get-FileHash -LiteralPath $partPath -Algorithm SHA256).Hash.ToLowerInvariant()
			$partHashes.Add("$partHash  $([System.IO.Path]::GetFileName($partPath))")
			$partIndex++
		}
		Set-Content -LiteralPath $partsChecksumPath -Value $partHashes -Encoding ascii
	} catch {
		foreach ($partPath in $partPaths) {
			if (Test-Path -LiteralPath $partPath) {
				Remove-Item -LiteralPath $partPath -Force
			}
		}
		if (Test-Path -LiteralPath $partsChecksumPath) {
			Remove-Item -LiteralPath $partsChecksumPath -Force
		}
		if (Test-Path -LiteralPath $archivePath) {
			Remove-Item -LiteralPath $archivePath -Force
		}
		if (Test-Path -LiteralPath $checksumPath) {
			Remove-Item -LiteralPath $checksumPath -Force
		}
		throw
	} finally {
		$sourceStream.Dispose()
	}

	Remove-Item -LiteralPath $archivePath -Force
	Write-Output "Archive parts: $($partPaths -join ', ')"
	Write-Output "Part SHA-256 list: $partsChecksumPath"
	Write-Output "Reassembled archive SHA-256: $checksumPath"
} else {
	Write-Output "Release archive: $archivePath"
}

Write-Output "SHA-256: $checksumPath"
Write-Output "Archive size: $($archiveInfo.Length) bytes"
