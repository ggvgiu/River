param([string]$MSBuildPath, [switch]$SkipDialogTests)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
if (!$MSBuildPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (!(Test-Path -LiteralPath $vswhere)) { throw 'Install Visual Studio with Desktop development with C++, or pass -MSBuildPath.' }
    $matches = @(& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'MSBuild\Current\Bin\MSBuild.exe')
    if ($LASTEXITCODE -ne 0 -or !$matches.Count) { throw 'Could not find MSBuild with the C++ tools installed.' }
    $MSBuildPath = $matches[0]
}
if (!(Test-Path -LiteralPath $MSBuildPath)) { throw "MSBuild not found: $MSBuildPath" }

function Build-Project([string]$project, [string]$configuration) {
    & $MSBuildPath (Join-Path $root $project) /m /nologo /verbosity:minimal "/p:Configuration=$configuration" /p:Platform=x64 /p:PreferredToolArchitecture=x64
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $project ($configuration)" }
}

Push-Location $root
try {
    Build-Project 'River.vcxproj' 'Release'
    foreach ($test in @('GameTests','RenderTests','SeedDialogTests')) {
        Build-Project "tests/$test.vcxproj" 'Debug'
    }
    Push-Location (Join-Path $root 'tests')
    try {
        foreach ($test in @('GameTests','RenderTests','SeedDialogTests')) {
            if ($SkipDialogTests -and $test -eq 'SeedDialogTests') {
                Write-Output 'Skipping interactive dialog execution; its resource and source build was checked.'
                continue
            }
            & "./bin/x64/Debug/$test.exe"
            if ($LASTEXITCODE -ne 0) { throw "Test failed: $test" }
        }
    } finally { Pop-Location }

    $package = Join-Path $root 'dist/windows-x64'
    New-Item -ItemType Directory -Path $package -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $root 'x64/Release/River.exe') -Destination $package
    Copy-Item -LiteralPath (Join-Path $root 'resources/PLAYING.txt') -Destination $package
    Write-Output "Playable build ready: $package"
} finally { Pop-Location }
