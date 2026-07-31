# Set up the current PowerShell process for the latest x64 MSVC toolchain.
# This intentionally imports vcvars64.bat's environment instead of invoking
# Launch-VsDevShell.ps1, which is sensitive to PowerShell execution policy and
# duplicate case-insensitive environment-variable names in some VS Code shells.

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"

$vswhereCandidates = @(
	(Get-Command vswhere.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source -First 1),
	"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
	"$env:ProgramFiles\Microsoft Visual Studio\Installer\vswhere.exe"
) | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -Unique

if(!$vswhereCandidates)
{
	throw "vswhere.exe was not found. Install the Visual Studio Installer or add vswhere.exe to PATH."
}

$vswhere = @($vswhereCandidates)[0]
$installationPath = & $vswhere -latest -products * `
	-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
	-property installationPath

if($LASTEXITCODE -ne 0 -or !$installationPath)
{
	throw "No Visual Studio installation containing the MSVC x86/x64 build tools was found."
}

$installationPath = ($installationPath | Select-Object -First 1).Trim()
$vcvars = Join-Path $installationPath "VC\Auxiliary\Build\vcvars64.bat"
if(!(Test-Path -LiteralPath $vcvars))
{
	throw "The discovered Visual Studio installation has no vcvars64.bat: $vcvars"
}

Write-Host "Initializing x64 MSVC from: $installationPath"

# Capture the environment from cmd.exe. Environment changes made by a batch
# file cannot otherwise propagate back into its parent PowerShell process.
$environmentLines = & $env:ComSpec /d /s /c "call `"$vcvars`" >nul && set"
if($LASTEXITCODE -ne 0)
{
	throw "vcvars64.bat failed with exit code $LASTEXITCODE."
}

$importedEnvironment = [System.Collections.Generic.Dictionary[string, string]]::new(
	[System.StringComparer]::OrdinalIgnoreCase)
foreach($line in $environmentLines)
{
	# Ignore cmd.exe's pseudo variables such as '=C:=C:\path'.
	if($line -match "^([^=]+)=(.*)$")
	{
		# A VS Code terminal may contain both PATH and Path (or ComSpec and
		# COMSPEC). Keep cmd.exe's first value; vcvars writes its updated PATH
		# using the uppercase spelling.
		if(!$importedEnvironment.ContainsKey($matches[1]))
		{
			$importedEnvironment.Add($matches[1], $matches[2])
		}
	}
}

foreach($variable in $importedEnvironment.GetEnumerator())
{
	Set-Item -LiteralPath "Env:$($variable.Key)" -Value $variable.Value
}

# Some VS Code installations prepend an MSYS environment whose cmake.exe and
# ninja.exe report Unix paths such as /usr/bin. Those tools cannot drive native
# MSVC correctly. Prefer the native copies installed with Visual Studio.
$nativeToolDirectories = @(
	(Join-Path $installationPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"),
	(Join-Path $installationPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja")
) | Where-Object { Test-Path -LiteralPath $_ }

$pathParts = @($nativeToolDirectories)
foreach($pathPart in ($env:Path -split ";"))
{
	if($pathPart -and !($nativeToolDirectories -contains $pathPart))
	{
		$pathParts += $pathPart
	}
}
$env:Path = $pathParts -join ";"

# CMake gives CC/CXX precedence over normal compiler discovery. They are not
# part of a standard VS developer prompt, so remove inherited overrides.
Remove-Item Env:CC -ErrorAction SilentlyContinue
Remove-Item Env:CXX -ErrorAction SilentlyContinue

$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if(!$cl)
{
	throw "The Visual Studio environment was imported, but cl.exe is not on PATH."
}

Write-Host "MSVC x64 environment ready: $($cl.Source)"
Write-Host "CMake: $((Get-Command cmake.exe -ErrorAction SilentlyContinue).Source)"
Write-Host "Ninja: $((Get-Command ninja.exe -ErrorAction SilentlyContinue).Source)"
Write-Host "Use a fresh CMake build directory if that directory previously selected another compiler."
